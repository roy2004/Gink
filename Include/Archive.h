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
Archive::operator<<(bool rvalue)
{
    operator<<(static_cast<::int8_t>(rvalue));
    return *this;
}


Archive &
Archive::operator>>(bool &lvalue)
{
    ::int8_t temp;
    operator>>(temp);
    lvalue = temp;
    return *this;
}


template<class T>
typename std::enable_if<std::is_unsigned<T>::value, Archive &>::type
Archive::operator<<(T rvalue)
{
    operator<<(static_cast<typename std::make_signed<T>::type>(rvalue));
    return *this;
}


template<class T>
typename std::enable_if<std::is_unsigned<T>::value, Archive &>::type
Archive::operator>>(T &lvalue)
{
    operator>>(reinterpret_cast<typename std::make_signed<T>::type &>(lvalue));
    return *this;
}


template<class T>
typename std::enable_if<std::is_enum<T>::value, Archive &>::type
Archive::operator<<(T rvalue)
{
    operator<<(static_cast<varint_t>(rvalue));
    return *this;
}


template<class T>
typename std::enable_if<std::is_enum<T>::value, Archive &>::type
Archive::operator>>(T &lvalue)
{
    varint_t temp;
    operator>>(temp);
    lvalue = static_cast<T>(temp);
    return *this;
}


template<class T>
typename std::enable_if<std::is_class<T>::value, Archive &>::type
Archive::operator<<(const T &rvalue)
{
    rvalue.save(this);
    return *this;
}


template<class T>
typename std::enable_if<std::is_class<T>::value, Archive &>::type
Archive::operator>>(T &lvalue)
{
    lvalue.load(this);
    return *this;
}


template<class T>
Archive &
Archive::operator<<(const std::vector<T> &rvalue)
{
    operator<<(static_cast<varint_t>(rvalue.size()));

    for (const T &x: rvalue) {
        operator<<(x);
    }

    return *this;
}


template<class T>
Archive &
Archive::operator>>(std::vector<T> &lvalue)
{
    varint_t temp;
    operator>>(temp);
    auto n = static_cast<typename std::vector<T>::size_type>(temp);

    while (n != 0) {
        lvalue.emplace_back();
        operator>>(lvalue.back());
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
