#pragma once


#include <cstdint>
#include <cstddef>
#include <cassert>
#include <type_traits>
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
    inline Archive &operator<<(const std::string &);
    inline Archive &operator>>(std::string &);

    template<class T>
    inline typename std::enable_if<std::is_signed<T>::value, Archive &>::type operator<<(T);

    template<class T>
    inline typename std::enable_if<std::is_signed<T>::value, Archive &>::type operator>>(T &);

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

    void flush();

private:
    Stream *const stream_;
    std::size_t writtenByteCount_;
    std::size_t readByteCount_;

    void serializeInteger(std::int8_t);
    void deserializeInteger(std::int8_t *);
    void serializeInteger(std::int16_t);
    void deserializeInteger(std::int16_t *);
    void serializeInteger(std::int32_t);
    void deserializeInteger(std::int32_t *);
    void serializeInteger(std::int64_t);
    void deserializeInteger(std::int64_t *);
    void serializeVariableLengthInteger(std::intmax_t);
    void deserializeVariableLengthInteger(std::intmax_t *);
    void serializeBytes(const char *, std::size_t);
    void deserializeBytes(const char **, std::size_t *);
};


Archive::Archive(Stream *stream)
    : stream_(stream), writtenByteCount_(0), readByteCount_(0)
{
    assert(stream != nullptr);
}


Archive &
Archive::operator<<(bool boolean)
{
    serializeInteger(static_cast<std::int8_t>(boolean));
    return *this;
}


Archive &
Archive::operator>>(bool &boolean)
{
    std::int8_t temp;
    deserializeInteger(&temp);
    boolean = temp;
    return *this;
}


Archive &
Archive::operator<<(const std::string &string)
{
    serializeBytes(string.data(), string.size());
    return *this;
}


Archive &
Archive::operator>>(std::string &string)
{
    const char *bytes;
    size_t numberOfBytes;
    deserializeBytes(&bytes, &numberOfBytes);
    string.append(bytes, numberOfBytes);
    return *this;
}


template<class T>
typename std::enable_if<std::is_signed<T>::value, Archive &>::type
Archive::operator<<(T integer)
{
    serializeInteger(integer);
    return *this;
}


template<class T>
typename std::enable_if<std::is_signed<T>::value, Archive &>::type
Archive::operator>>(T &integer)
{
    deserializeInteger(&integer);
    return *this;
}


template<class T>
typename std::enable_if<std::is_unsigned<T>::value, Archive &>::type
Archive::operator<<(T uInteger)
{
    serializeInteger(static_cast<typename std::make_signed<T>::type>(uInteger));
    return *this;
}


template<class T>
typename std::enable_if<std::is_unsigned<T>::value, Archive &>::type
Archive::operator>>(T &uInteger)
{
    typename std::make_signed<T>::type temp;
    deserializeInteger(&temp);
    uInteger = temp;
    return *this;
}


template<class T>
typename std::enable_if<std::is_enum<T>::value, Archive &>::type
Archive::operator<<(T enumerator)
{
    serializeVariableLengthInteger(static_cast<std::intmax_t>(enumerator));
    return *this;
}


template<class T>
typename std::enable_if<std::is_enum<T>::value, Archive &>::type
Archive::operator>>(T &enumerator)
{
    std::intmax_t temp;
    deserializeVariableLengthInteger(&temp);
    enumerator = static_cast<T>(temp);
    return *this;
}


template<class T>
typename std::enable_if<std::is_class<T>::value, Archive &>::type
Archive::operator<<(const T &object)
{
    object.store();
    return *this;
}


template<class T>
typename std::enable_if<std::is_class<T>::value, Archive &>::type
Archive::operator>>(T &object)
{
    object.load();
    return *this;
}


template<class T>
Archive &
Archive::operator<<(const std::vector<T> &vector)
{
    serializeVariableLengthInteger(vector.size());

    for (const T &x: vector) {
        operator<<(x);
    }

    return *this;
}


template<class T>
Archive &
Archive::operator>>(std::vector<T> &vector)
{
    std::intmax_t temp;
    deserializeVariableLengthInteger(&temp);
    auto n = static_cast<typename std::vector<T>::size_type>(temp);

    while (n != 0) {
        vector.emplace_back();
        operator>>(vector.back());
        --n;
    }

    return *this;
}

} // namespace Gink
