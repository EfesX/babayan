#pragma once

#include "babayan.hpp"

/* Мьютекс на доступ к std::cout */
boost::mutex io_mutex;

/**
 * @brief Интерфейс класса ищущего дубликаты в IKeeper
*/
class IReader {
public:
    IReader() {}
    virtual ~IReader() = default;

    virtual void process(std::shared_ptr<IKeeper> keeper) = 0;
};

/**
 * @brief Вспомогательная структура
*/
struct m_file {
private:
    /* Алгоритм хэширования */
    std::unique_ptr<ihasher> hasher;
public:
    const boost::filesystem::path& file;
    unsigned size;

    m_file(const boost::filesystem::path& file_, unsigned size_) : 
        file(file_), size(size_), blocks_ready(0)
    {
        block_size = config::Config::instance().block;
        unsigned blocks = (size / block_size);
        total_blocks = (size % block_size) ? (blocks + 1) : blocks;

        if(config::Config::instance().hash == "crc32"){
            hasher = std::make_unique<crc32_hasher>();
        } else if (config::Config::instance().hash == "md5") {
            hasher = std::make_unique<md5_hasher>();
        } else {
            std::cout << "Unknown hasher " + config::Config::instance().hash << std::endl;
            throw std::exception();
        }

        hasher->reset();
    }

    m_file(const m_file&)  = delete;
    m_file(const m_file&&) = delete;

    ~m_file() = default;

    unsigned block_size;
    unsigned total_blocks;
    unsigned blocks_ready;
    unsigned checksum;

    /**
     * @brief Структуры равны, если файлы на которые указывают поля file равны по хэшу от их данных.
     * Вычисление хэша идет поблочно. До первого несовпадения хэшей.
     * */
    bool operator==(m_file& other){
        if(size != other.size) return false;

        if(blocks_ready != other.blocks_ready){
            if(blocks_ready < other.blocks_ready){
                while(blocks_ready != other.blocks_ready){
                    next_hash();
                }
            } else {
                while(blocks_ready != other.blocks_ready){
                    other.next_hash();
                }
            }
        }

        while (blocks_ready != total_blocks){
            if(hasher->checksum() != other.hasher->checksum()){
                return false;
            }

            next_hash();
            other.next_hash();
        }

        if(hasher->checksum() == other.hasher->checksum()){
            return true;
        }

        return false;
    }

    /**
     * @brief Вычислить следующую порцию хэша
    */
    void next_hash(){
        if (blocks_ready == total_blocks){
            std::cout << "Уже вычислен весь хэш. Вычислять больше нечего" << std::endl;
            throw std::exception();
        }

        boost::interprocess::file_mapping m_file(file.string().c_str(), boost::interprocess::read_only);
        boost::interprocess::mapped_region region(
            m_file, boost::interprocess::read_only, 
            blocks_ready * block_size, block_size
        );

        void * raddr       = region.get_address();
        std::size_t rsize  = region.get_size();

        hasher->next_hash(raddr, rsize);
        checksum = hasher->checksum();
        blocks_ready++;
    }
};

/* Тэг для files_container */
struct checksum{};

/* Контейнер хранящий указатели на структуры m_file */
typedef multi_index_container<
    m_file*,
    indexed_by<
        ordered_non_unique<
            tag<checksum>,  BOOST_MULTI_INDEX_MEMBER(m_file, unsigned, checksum)
        >
    >
> files_container;

/**
 * @brief Имплементация IReader
*/
class Reader : public IReader {
private:
    /**
     * @brief Выполняет основную работу по поиску дубликатов.
    */
    static void _process(std::pair<iter_t, iter_t> iters){
        auto distance = boost::distance(iters.first, iters.second);
        if(distance <= 1) return;

        files_container files;

        // Сравнить каждый файл с каждым, что приведет к
        // вычислению достаточного кол-ва блоков хэшей. 
        // И после этого закинуть в контейнер для дальнейшей 
        // группировки по значению хэша
        m_file* elem;
        while(iters.first != iters.second){
            elem = new m_file(iters.first->path, iters.first->size);
            for(auto it = files.begin(); it != files.end(); it++){
                (**it == *elem);
            }

            files.insert(elem);
            iters.first++;
        }

        // Группировать по значению хэша
        const typename boost::multi_index::index<
            files_container, checksum
        >::type& filesByHash = get<checksum>(files);

        auto start = filesByHash.begin();
        auto end   = filesByHash.end();

        boost::unique_lock<boost::mutex> scoped_lock(io_mutex);

        while(start != end){
            auto _iters = filesByHash.equal_range((*start)->checksum);

            if(std::distance(_iters.first, _iters.second) > 1){
                while(_iters.first != _iters.second){
                    std::cout << (*_iters.first)->file << std::endl;        
                    _iters.first++;
                }
                std::cout << std::endl;
            }
            start = _iters.second;
        }

        for (auto v : files){
            delete v;
        }
    }

public:
    Reader() = default;
    ~Reader() = default;

    /**
     * @brief Итерируется по группам файлов с одинаковым размером 
     * из IKeeper и для каждой группы запускает поток для поиска дубликатов
     * @arg keeper Хранилище подготовленных файлов
     */
    void process(std::shared_ptr<IKeeper> keeper) override {
        boost::asio::thread_pool pool(config::Config::instance().threads);
        
        for(auto iters : keeper->group_by_size()){
            boost::asio::post(pool, boost::bind(Reader::_process, iters));
        }

        pool.join();
    }
};
