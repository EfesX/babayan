#pragma once

#include "babayan.hpp"

/**
 * @brief Интерфейс отбирателя файлов
*/
class IScaner {
protected:
    /* Хранилище файлов */
    std::shared_ptr<IKeeper> _keeper;
public:
    IScaner(std::shared_ptr<IKeeper> keeper) : _keeper(keeper) {}
    virtual ~IScaner() = default;
    /* Просканировать файлы в соответсвии с конфигом и закинуть подходящие в _keeper */
    virtual void collect() = 0;
};

/**
 * @brief Реализация IScaner
*/
class Scaner : public IScaner {
private:
    const bool& _r; 
    const std::set<boost::filesystem::path>& _inc;
    const std::set<boost::filesystem::path>& _exc;
    const unsigned _filesize;

    /**
     * @brief Вызывается рекурсивно (если позволено в конфиге) для 
     * каждой директории, проверяет файлы в директориях на соответствие
     * заданным фильтрам, складывает файлы, прошедшие фильтры в _keeper
     * базового класса
     * @arg it Указатель на начало директории
     * @arg end Указатель на конец директории
    */
    void _scan(auto& it, auto& end){
        namespace fs = boost::filesystem;

        for(; it != end; ++it){
            switch (fs::status(*it).type())
            {
            case fs::regular_file:
                /* Фильтр по размеру файла */
                if(fs::file_size(it->path()) < _filesize) continue;;

                if(config::Config::instance().masks.size() > 0){
                    /* Фильтр по маскам разрешенных имен */
                    for(auto mask : config::Config::instance().masks){
                        std::string s = it->path().filename().string();
                        boost::to_lower(s);
                        if(boost::contains(s, mask)){
                            _keeper->add_file(it->path(), fs::file_size(it->path()));
                        };
                    }
                } else {
                    _keeper->add_file(it->path(), fs::file_size(it->path()));
                }
                break;

            case fs::directory_file:
                if (_r){
                    if(_exc.find(it->path()) == _exc.end()){
                        boost::filesystem::directory_iterator it_r(it->path()); 
                        _scan(it_r, end);
                    }
                }
                break;

            default: break;
            }
        }
    }

public:

    Scaner(std::shared_ptr<IKeeper> keeper) : 
        _r(config::Config::instance().level),
        _inc(config::Config::instance().includes),
        _exc(config::Config::instance().excludes),
        _filesize(config::Config::instance().minfile),
        IScaner(keeper)
    {}

    void collect() override {
        namespace fs = boost::filesystem;

        for (auto path: _inc){
            try{
                boost::filesystem::directory_iterator begin(path);
                boost::filesystem::directory_iterator end;
                _scan(begin, end);
            } catch(const boost::filesystem::filesystem_error& ex) {
                std::cout << "filesystem's error:\n"
                    << "    code: " << ex.code().value() << '\n'
                    << "    category: " << ex.code().category().name() << '\n'
                    << "    what happens: " << ex.what() << '\n';
            }
        }
    }
};
