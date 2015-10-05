#pragma once


#include <cassert>
#include <cerrno>
#include <string>
#include <vector>

#include <Pixy/Binary.h>

#include "Stream.h"
#include "SystemError.h"


namespace Gink {


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
    inline typename std::enable_if<std::is_integral<T>::value, Archive &>::type operator<<(T);

    template<class T>
    inline typename std::enable_if<std::is_integral<T>::value, Archive &>::type operator>>(T &);

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

    inline void flush();

private:
    typedef enum: ::intmax_t {} varint_t;

    Stream *const stream_;
    ::size_t writtenByteCount_;
    ::size_t readByteCount_;
};


inline ::ptrdiff_t __PackInteger(char *, ::size_t, ::int8_t);
inline ::ptrdiff_t __UnpackInteger(const char *, ::size_t, ::int8_t *);
inline ::ptrdiff_t __PackInteger(char *, ::size_t, ::int16_t);
inline ::ptrdiff_t __UnpackInteger(const char *, ::size_t, ::int16_t *);
inline ::ptrdiff_t __PackInteger(char *, ::size_t, ::int32_t);
inline ::ptrdiff_t __UnpackInteger(const char *, ::size_t, ::int32_t *);
inline ::ptrdiff_t __PackInteger(char *, ::size_t, ::int64_t);
inline ::ptrdiff_t __UnpackInteger(const char *, ::size_t, ::int64_t *);


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


Archive &
Archive::operator<<(const std::string &rvalue)
{
    for (;;) {
        ::ptrdiff_t result = ::PackBytes(
            static_cast<char *>(stream_->getBuffer()) + writtenByteCount_,
            stream_->getBufferSize() - writtenByteCount_,
            rvalue.data(),
            rvalue.size()
        );

        if (result >= 0) {
            writtenByteCount_ += result;
            break;
        }

        stream_->growBuffer(-result);
    }

    return *this;
}


Archive &
Archive::operator>>(std::string &lvalue)
{
    const char *bytes;
    size_t numberOfBytes;

    ::ptrdiff_t result = ::UnpackBytes(
        static_cast<const char *>(stream_->getData()) + readByteCount_,
        stream_->getDataSize() + writtenByteCount_ - readByteCount_,
        &bytes,
        &numberOfBytes
    );

    if (result < 0) {
        throw GINK_SYSTEM_ERROR(errno, "`::UnpackBytes()` failed");
    }

    readByteCount_ += result;
    lvalue.append(bytes, numberOfBytes);
    return *this;
}


template<class T>
typename std::enable_if<std::is_integral<T>::value, Archive &>::type
Archive::operator<<(T rvalue)
{
    for (;;) {
        ::ptrdiff_t result = __PackInteger(
            static_cast<char *>(stream_->getBuffer()) + writtenByteCount_,
            stream_->getBufferSize() - writtenByteCount_,
            static_cast<typename std::make_signed<T>::type>(rvalue)
        );

        if (result >= 0) {
            writtenByteCount_ += result;
            break;
        }

        stream_->growBuffer(-result);
    }

    return *this;
}


template<class T>
typename std::enable_if<std::is_integral<T>::value, Archive &>::type
Archive::operator>>(T &lvalue)
{
    ::ptrdiff_t result = __UnpackInteger(
        static_cast<const char *>(stream_->getData()) + readByteCount_,
        stream_->getDataSize() + writtenByteCount_ - readByteCount_,
        reinterpret_cast<typename std::make_signed<T>::type *>(&lvalue)
    );

    if (result < 0) {
        throw GINK_SYSTEM_ERROR(errno, "`__UnpackInteger()` failed");
    }

    readByteCount_ += result;
    return *this;
}


template<class T>
typename std::enable_if<std::is_enum<T>::value, Archive &>::type
Archive::operator<<(T rvalue)
{
    for (;;) {
        ::ptrdiff_t result = ::PackVariableLengthInteger(
            static_cast<char *>(stream_->getBuffer()) + writtenByteCount_,
            stream_->getBufferSize() - writtenByteCount_,
            static_cast<::intmax_t>(rvalue)
        );

        if (result >= 0) {
            writtenByteCount_ += result;
            break;
        }

        stream_->growBuffer(-result);
    }

    return *this;
}


template<class T>
typename std::enable_if<std::is_enum<T>::value, Archive &>::type
Archive::operator>>(T &lvalue)
{
    ::intmax_t temp;

    ::ptrdiff_t result = ::UnpackVariableLengthInteger(
        static_cast<const char *>(stream_->getData()) + readByteCount_,
        stream_->getDataSize() + writtenByteCount_ - readByteCount_,
        &temp
    );

    if (result < 0) {
        throw GINK_SYSTEM_ERROR(errno, "`::UnpackVariableLengthInteger()` failed");
    }

    readByteCount_ += result;
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


void
Archive::flush()
{
    stream_->write(nullptr, writtenByteCount_);
    writtenByteCount_ = 0;
    stream_->read(nullptr, readByteCount_);
    readByteCount_ = 0;
}


::ptrdiff_t
__PackInteger(char *buffer, ::size_t bufferSize, ::int8_t integer)
{
    return ::PackInteger8(buffer, bufferSize, integer);
}


::ptrdiff_t
__UnpackInteger(const char *data, ::size_t dataSize, ::int8_t *integer)
{
    return ::UnpackInteger8(data, dataSize, integer);
}


::ptrdiff_t
__PackInteger(char *buffer, ::size_t bufferSize, ::int16_t integer)
{
    return ::PackInteger16(buffer, bufferSize, integer);
}


::ptrdiff_t
__UnpackInteger(const char *data, ::size_t dataSize, ::int16_t *integer)
{
    return ::UnpackInteger16(data, dataSize, integer);
}


::ptrdiff_t
__PackInteger(char *buffer, ::size_t bufferSize, ::int32_t integer)
{
    return ::PackInteger32(buffer, bufferSize, integer);
}


::ptrdiff_t
__UnpackInteger(const char *data, ::size_t dataSize, ::int32_t *integer)
{
    return ::UnpackInteger32(data, dataSize, integer);
}


::ptrdiff_t
__PackInteger(char *buffer, ::size_t bufferSize, ::int64_t integer)
{
    return ::PackInteger64(buffer, bufferSize, integer);
}


::ptrdiff_t
__UnpackInteger(const char *data, ::size_t dataSize, ::int64_t *integer)
{
    return ::UnpackInteger64(data, dataSize, integer);
}

}
