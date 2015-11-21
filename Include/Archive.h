#pragma once


#include <cstdint>
#include <cstddef>
#include <cassert>
#include <type_traits>
#include <vector>
#include <string>
#include <limits>


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
    inline typename std::enable_if<std::is_unsigned<T>::value, Archive &>::type operator<<(T);

    template<class T>
    inline typename std::enable_if<std::is_unsigned<T>::value, Archive &>::type operator>>(T &);

    template<class T>
    inline typename std::enable_if<std::is_signed<T>::value, Archive &>::type operator<<(T);

    template<class T>
    inline typename std::enable_if<std::is_signed<T>::value, Archive &>::type operator>>(T &);

    template<class T>
    inline typename std::enable_if<std::is_enum<T>::value, Archive &>::type operator<<(T);

    template<class T>
    inline typename std::enable_if<std::is_enum<T>::value, Archive &>::type operator>>(T &);

    template<class T>
    inline typename std::enable_if<std::is_class<T>::value, Archive &>::type operator<<(const T &);

    template<class T>
    inline typename std::enable_if<std::is_class<T>::value, Archive &>::type operator>>(T &);

    template<class T, std::size_t N>
    inline Archive &operator<<(const T (&)[N]);

    template<class T, std::size_t N>
    inline Archive &operator>>(T (&)[N]);

    template<class T>
    inline Archive &operator<<(const std::vector<T> &);

    template<class T>
    inline Archive &operator>>(std::vector<T> &);

    void flush();

private:
    Stream *const stream_;
    std::size_t writtenByteCount_;
    std::size_t readByteCount_;

    void serializeInteger(std::uint8_t);
    void deserializeInteger(std::uint8_t *);
    void serializeInteger(std::uint16_t);
    void deserializeInteger(std::uint16_t *);
    void serializeInteger(std::uint32_t);
    void deserializeInteger(std::uint32_t *);
    void serializeInteger(std::uint64_t);
    void deserializeInteger(std::uint64_t *);
    void serializeVariableLengthInteger(std::uintmax_t);
    void deserializeVariableLengthInteger(std::uintmax_t *);
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
    serializeInteger(static_cast<std::uint8_t>(boolean));
    return *this;
}


Archive &
Archive::operator>>(bool &boolean)
{
    std::uint8_t temp;
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
typename std::enable_if<std::is_unsigned<T>::value, Archive &>::type
Archive::operator<<(T integer)
{
    serializeInteger(integer);
    return *this;
}


template<class T>
typename std::enable_if<std::is_unsigned<T>::value, Archive &>::type
Archive::operator>>(T &integer)
{
    deserializeInteger(&integer);
    return *this;
}


template<class T>
typename std::enable_if<std::is_signed<T>::value, Archive &>::type
Archive::operator<<(T integer)
{
    serializeInteger(static_cast<typename std::make_unsigned<T>::type>(integer));
    return *this;
}


template<class T>
typename std::enable_if<std::is_signed<T>::value, Archive &>::type
Archive::operator>>(T &integer)
{
    typename std::make_unsigned<T>::type temp;
    deserializeInteger(&temp);
    integer = temp <= std::numeric_limits<T>::max() ? static_cast<T>(temp)
                                                      : -static_cast<T>(-temp - 1) - 1;
    return *this;
}


template<class T>
typename std::enable_if<std::is_enum<T>::value, Archive &>::type
Archive::operator<<(T enumerator)
{
    operator<<(static_cast<typename std::underlying_type<T>::type>(enumerator));
    return *this;
}


template<class T>
typename std::enable_if<std::is_enum<T>::value, Archive &>::type
Archive::operator>>(T &enumerator)
{
    typename std::underlying_type<T>::type temp;
    operator>>(temp);
    enumerator = static_cast<T>(temp);
    return *this;
}


template<class T>
typename std::enable_if<std::is_class<T>::value, Archive &>::type
Archive::operator<<(const T &object)
{
    object.store(this);
    return *this;
}


template<class T>
typename std::enable_if<std::is_class<T>::value, Archive &>::type
Archive::operator>>(T &object)
{
    object.load(this);
    return *this;
}


template<class T, std::size_t N>
Archive &
Archive::operator<<(const T (&array)[N])
{
    std::ptrdiff_t i;

    for (i = 0; i < static_cast<std::ptrdiff_t>(N); ++i) {
        operator<<(array[i]);
    }

    return *this;
}


template<class T, std::size_t N>
Archive &
Archive::operator>>(T (&array)[N])
{
    std::ptrdiff_t i;

    for (i = 0; i < static_cast<std::ptrdiff_t>(N); ++i) {
        operator>>(array[i]);
    }

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
    std::uintmax_t temp;
    deserializeVariableLengthInteger(&temp);
    auto n = static_cast<typename std::vector<T>::size_type>(temp);

    while (n >= 1) {
        vector.emplace_back();
        operator>>(vector.back());
        --n;
    }

    return *this;
}

} // namespace Gink
