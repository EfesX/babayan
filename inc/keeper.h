#pragma once

#include "babayan.hpp"

using namespace boost::multi_index;
using namespace boost::multi_index::detail;

/**
 * @brief Структура представляющая файл в файловой системе
*/
struct file_entry {
    /* Путь к файлу */
    boost::filesystem::path path;
    /* Размер файла */
    unsigned size;

    file_entry(boost::filesystem::path path_, unsigned size_) :
        path(path_), size(size_)
    {}
};

/* Тэг для multi_index_container */
struct path {};
/* Тэг для multi_index_container */
struct size {};

/* Контейнер содержащий файлы отобранные для сканирования */
typedef multi_index_container<
    file_entry,
    indexed_by<
        hashed_unique<tag<path>,  BOOST_MULTI_INDEX_MEMBER(file_entry, boost::filesystem::path, path)>,
        ordered_non_unique<tag<size>,  BOOST_MULTI_INDEX_MEMBER(file_entry, unsigned, size)>
    >
> file_set;

using iter_t = boost::multi_index::detail::bidir_node_iterator<
        boost::multi_index::detail::ordered_index_node<
            boost::multi_index::detail::null_augment_policy, 
                boost::multi_index::detail::index_node_base<
                    file_entry, 
                    std::allocator<file_entry> 
                > 
            > 
        >;
using coro_pull_t = boost::coroutines2::coroutine<std::pair<iter_t, iter_t>>::pull_type;
using coro_push_t = boost::coroutines2::coroutine<std::pair<iter_t, iter_t>>::push_type;

/**
 * @brief Интерфейс хранилища файлов отобранных для сканирования
*/
class IKeeper {
public:
    virtual ~IKeeper() = default;
    virtual void add_file(boost::filesystem::path file, unsigned size) = 0;
    /**
     * @brief Группирует список файлов по размеру
     * @return Генератор-корутина, которая возвращает пары итераторов
     * на первый файл из группы с одинаковым размером файлов и 
     * последний файл из этой группы
    */
    virtual coro_pull_t group_by_size() = 0;
};

class Keeper : public IKeeper {
private:
    /* Отобранные файлы */
    file_set _files;
public:
    Keeper() {}
    void add_file(boost::filesystem::path file, unsigned size) override {
        _files.insert(file_entry(file, size));
    }
    
    coro_pull_t group_by_size() override {
        coro_pull_t coro([&](coro_push_t& yield){
            const typename boost::multi_index::index<file_set, size>::type& filesBySize= get<size>(_files);
            auto start = filesBySize.begin();
            auto end   = filesBySize.end();

            while(start != end){
                auto iters = filesBySize.equal_range(start->size);
                yield(iters);
                start = iters.second;
            }
        });
        return coro;
    }
};
