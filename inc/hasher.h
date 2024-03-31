#pragma once

#include "babayan.hpp"

/**
 * @brief Интерфейс алгоритмов хэширования
*/
class ihasher {
public:
    virtual ~ihasher() = default;
    /**
     * @brief Считает хэш от блока данных и комбинирует с предыдущим хэшем
     * @argdo addr Начальный адрес блока
     * @arg size Размер блока
     * @return Хэш после расчета блока
    */
    virtual unsigned next_hash(void* addr, const std::size_t& size) = 0;
    /**
     * @brief Возвращает текущий хэш
     * @return Текущий хэш
    */
    virtual unsigned checksum() = 0;
    /**
     * @brief Сбрасывает текущий хэш в начальное состояние
    */
    virtual void reset() = 0;
};

/**
 * @brief Класс расчета хэша по алгоритму CRC32
*/
class crc32_hasher : public ihasher {
private:
    boost::crc_32_type hash;
public:
    crc32_hasher() {
        hash.reset();
    }
    ~crc32_hasher() = default;

    unsigned next_hash(void* addr, const std::size_t& size) override {
        hash.process_bytes(addr, size);
        return hash.checksum();
    };

    unsigned checksum() override {
        return hash.checksum();
    }

    void reset() override {
        hash.reset();
    }
};

/**
 * @brief Класс расчета хэша по алгоритму MD5
*/
class md5_hasher : public ihasher {
private:
    boost::uuids::detail::md5 hash;
    boost::uuids::detail::md5::digest_type result;
public:
    md5_hasher() {
    }
    ~md5_hasher() = default;

    unsigned next_hash(void* addr, const std::size_t& size) override {
        hash.process_bytes(addr, size);
        hash.get_digest(result);
        return *result;
    };

    unsigned checksum() override {
        return *result;
    }

    void reset() override {
    }
};

