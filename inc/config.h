#pragma once

#include "babayan.hpp"

namespace config {

/**
 * @brief Синглтон, 
 * хранящий параметры заданные пользователем
 * через командную строку
*/
struct Config {
private:
    Config() = default;
public:
    /** @brief Директории для сканирования */
    std::set<boost::filesystem::path> includes;
    /** @brief Директории для исключения из сканирования */
    std::set<boost::filesystem::path> excludes;
    /** @brief Уровень сканирования: 1 - рекурсивно, 0 - только указанные директории */
    bool level;
    /** @brief Минимальный размер файла, разрешенный для сканирования */
    std::size_t minfile;
    /** @brief Маски имен файлов разрешенных для сканирования */
    std::set<std::string> masks;
    /** @brief Размер блоков (в байтах) для чтения файлов */
    std::size_t block;
    /** @brief Алгоритм расчета хэша */
    std::string hash;
    /** @brief Кол-во потоков при поиске дубликатов */
    std::size_t threads;

    Config(const Config&) = delete;
    Config(const Config&&) = delete;

    static Config& instance(){
        static Config conf;
        return conf;
    }
};

inline void _parse_string(const std::string& val, std::set<boost::filesystem::path>& set){
    typedef boost::tokenizer<boost::char_separator<char> > tokenizer;

    boost::char_separator<char> sep(",");
    tokenizer tokens(val, sep);
    for (tokenizer::iterator tok_iter = tokens.begin(); tok_iter != tokens.end(); ++tok_iter){
        set.insert(boost::filesystem::path(*tok_iter));
    }
}

inline void _parse_mask(const std::string& val, std::set<std::string>& set){
    typedef boost::tokenizer<boost::char_separator<char> > tokenizer;

    boost::char_separator<char> sep(",");
    tokenizer tokens(val, sep);
    for (tokenizer::iterator tok_iter = tokens.begin(); tok_iter != tokens.end(); ++tok_iter){
        std::string mask = *tok_iter;
        boost::to_lower(mask);
        set.insert(mask);
    }
}

void set_include_dirs(const std::string& val){
    _parse_string(val, Config::instance().includes);
}
void set_exclude_dirs(const std::string& val){
    _parse_string(val, Config::instance().excludes);
}
void set_level(const bool& val){
    Config::instance().level = val;
}
void set_minfile(const std::size_t& val){
    Config::instance().minfile = val;
}
void set_masks(const std::string& val){
    _parse_mask(val, Config::instance().masks);
}
void set_block(const std::size_t& val){
    Config::instance().block = val;
}

void set_hash(const std::string& val){
    if ((val != "crc32") && (val != "md5")){
        std::cout << "Неверно задан алгоритм хэширования" << std::endl;
        throw std::exception();
    }

    Config::instance().hash = val;
}

void set_threads(const std::size_t& val){
    Config::instance().threads = val;
}

auto parse_app_arguments(int argc, char *argv[]){
        namespace po = boost::program_options;
        
        auto desc = std::make_shared<po::options_description>("Options");

        desc->add_options()
            ("help", "This screen")
            (
                "include, i", 
                po::value<std::string>()->default_value(".")->notifier(config::set_include_dirs), 
                "Directories for scaning"
            )
            (
                "exclude, e",
                po::value<std::string>()->notifier(config::set_exclude_dirs),
                "Directories for excluding from scaning"
            )
            (
                "recursive, r",
                po::value<bool>()->default_value(false)->notifier(config::set_level),
                "Recursive scaning"
            )
            (
                "minsize, s",
                po::value<std::size_t>()->default_value(1)->notifier(config::set_minfile),
                "Minimum size [bytes] of file for include to scaning"
            )
            (
                "masks, m",
                po::value<std::string>()->notifier(config::set_masks),
                "Masks of names of files that included to scan. Register undepended."
            )
            (
                "block, b",
                po::value<std::size_t>()->default_value(1024)->notifier(config::set_block),
                "Size [bytes] of block for reading files"
            )
            (
                "hash, a",
                po::value<std::string>()->default_value("crc32")->notifier(config::set_hash),
                "Algorithm for hash (crc32, md5)"
            )
            (
                "threads, t",
                po::value<std::size_t>()->default_value(16)->notifier(config::set_threads),
                "Amount of threads"
            );

        std::shared_ptr<po::variables_map> vm = std::make_shared<po::variables_map>();

        po::store(parse_command_line(argc, argv, *desc), *vm);
        po::notify(*vm);

        return std::make_pair(vm, desc);
    }
}
