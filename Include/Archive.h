#pragma once


#include <cstddef>
#include <cstdint>
#include <cassert>
#include <vector>
#include <string>


namespace Gink {

class Stream;


class Archive final
{
    Archive(Archive &) = delete;
    Archive &operator=(Archive &) = delete;

public:
    inline explicit Archive(Stream *);

    inline Archive &operator<<(bool);
    inline Archive &operator>>(bool &);

    template<class T>
    inline typename std::enable_if<std::is_unsigned<T>::value, Archive &>::type operator<<(T);

    template<class T>
    inline typename std::enable_if<std::is_unsigned<T>::value, Archive &>::type operator>>(T &);

    template<class T>
    inline typename std::enable_if<std::is_enum<T>::value, Archive &>::type operator<<(T);

    template<class T>
    inline typename std::enable_if<std::is_enum<T>::value, Archive &>::type operator>>(T &);

    template<class T>
    inline typename std::enable_if<std::is_class<T>::value, Archive &>::type operator<<(const T &);

    template<class T>
    inline typename std::enable_if<std::is_class<T>::value, Archive &>::type operator>>(T &);

    template<class T>
    inline Archive &operator<<(const std::vector<T> &);

    template<class T>
    inline Archive &operator>>(std::vector<T> &);

    inline std::size_t getWrittenByteCount();
    inline std::size_t getReadByteCount();

    Archive &operator<<(std::int8_t);
    Archive &operator>>(std::int8_t &);
    Archive &operator<<(std::int16_t);
    Archive &operator>>(std::int16_t &);
    Archive &operator<<(std::int32_t);
    Archive &operator>>(std::int32_t &);
    Archive &operator<<(std::int64_t);
    Archive &operator>>(std::int64_t &);
    Archive &operator<<(const std::string &);
    Archive &operator>>(std::string &);

    void flush();

private:
    typedef enum: std::intmax_t {} varint_t;

    Stream *const stream_;
    std::size_t writtenByteCount_;
    std::size_t readByteCount_;

    Archive &operator<<(varint_t);
    Archive &operator>>(varint_t &);
};


Archive::Archive(Stream *stream)
    : stream_(stream), writtenByteCount_(0), readByteCount_(0)
{
    assert(stream != nullptr);
}


Archive &
Archive::operator<<(bool boolean)
{
    operator<<(static_cast<::int8_t>(boolean));
    return *this;
}


Archive &
Archive::operator>>(bool &boolean)
{
    ::int8_t temp;
    operator>>(temp);
    boolean = temp;
    return *this;
}


template<class T>
typename std::enable_if<std::is_unsigned<T>::value, Archive &>::type
Archive::operator<<(T uInteger)
{
    operator<<(static_cast<typename std::make_signed<T>::type>(uInteger));
    return *this;
}


template<class T>
typename std::enable_if<std::is_unsigned<T>::value, Archive &>::type
Archive::operator>>(T &uInteger)
{
    operator>>(reinterpret_cast<typename std::make_signed<T>::type &>(uInteger));
    return *this;
}


template<class T>
typename std::enable_if<std::is_enum<T>::value, Archive &>::type
Archive::operator<<(T enumerator)
{
    operator<<(static_cast<varint_t>(enumerator));
    return *this;
}


template<class T>
typename std::enable_if<std::is_enum<T>::value, Archive &>::type
Archive::operator>>(T &enumerator)
{
    varint_t temp;
    operator>>(temp);
    enumerator = static_cast<T>(temp);
    return *this;
}


template<class T>
typename std::enable_if<std::is_class<T>::value, Archive &>::type
Archive::operator<<(const T &structure)
{
    structure.save(this);
    return *this;
}


template<class T>
typename std::enable_if<std::is_class<T>::value, Archive &>::type
Archive::operator>>(T &structure)
{
    structure.load(this);
    return *this;
}


template<class T>
Archive &
Archive::operator<<(const std::vector<T> &vector)
{
    operator<<(static_cast<varint_t>(vector.size()));

    for (const T &x: vector) {
        operator<<(x);
    }

    return *this;
}


template<class T>
Archive &
Archive::operator>>(std::vector<T> &vector)
{
    varint_t temp;
    operator>>(temp);
    auto n = static_cast<typename std::vector<T>::size_type>(temp);

    while (n != 0) {
        vector.emplace_back();
        operator>>(vector.back());
        --n;
    }

    return *this;
}


std::size_t
Archive::getWrittenByteCount()
{
    return writtenByteCount_;
}


std::size_t
Archive::getReadByteCount()
{
    return readByteCount_;
}

} // namespace Gink
