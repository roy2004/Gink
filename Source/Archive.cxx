#include "Archive.h"

#include <cerrno>

#include <Pixy/Binary.h>

#include "Stream.h"
#include "SystemError.h"


namespace Gink {

#define ARCHIVE_INTEGER_SERIALIZER(n)                                          \
    Archive &                                                                  \
    Archive::operator<<(std::int##n##_t integer)                               \
    {                                                                          \
        for (;;) {                                                             \
            ::ptrdiff_t result = ::PackInteger##n(                             \
                static_cast<char *>(stream_->getBuffer()) + writtenByteCount_, \
                stream_->getBufferSize() - writtenByteCount_,                  \
                integer                                                        \
            );                                                                 \
                                                                               \
            if (result >= 0) {                                                 \
                writtenByteCount_ += result;                                   \
                break;                                                         \
            }                                                                  \
                                                                               \
            stream_->growBuffer(-result);                                      \
        }                                                                      \
                                                                               \
        return *this;                                                          \
    }

#define ARCHIVE_INTEGER_DESERIALIZER(n)                                         \
    Archive &                                                                   \
    Archive::operator>>(std::int##n##_t &integer)                               \
    {                                                                           \
        ::ptrdiff_t result = ::UnpackInteger##n(                                \
            static_cast<const char *>(stream_->getData()) + readByteCount_,     \
            stream_->getDataSize() + writtenByteCount_ - readByteCount_,        \
            &integer                                                            \
        );                                                                      \
                                                                                \
        if (result < 0) {                                                       \
            throw GINK_SYSTEM_ERROR(errno, "`::UnpackInteger" #n "()` failed"); \
        }                                                                       \
                                                                                \
        readByteCount_ += result;                                               \
        return *this;                                                           \
    }


ARCHIVE_INTEGER_SERIALIZER(8)


ARCHIVE_INTEGER_DESERIALIZER(8)


ARCHIVE_INTEGER_SERIALIZER(16)


ARCHIVE_INTEGER_DESERIALIZER(16)


ARCHIVE_INTEGER_SERIALIZER(32)


ARCHIVE_INTEGER_DESERIALIZER(32)


ARCHIVE_INTEGER_SERIALIZER(64)


ARCHIVE_INTEGER_DESERIALIZER(64)


#undef ARCHIVE_INTEGER_SERIALIZER
#undef ARCHIVE_INTEGER_DESERIALIZER


Archive &
Archive::operator<<(const std::string &string)
{
    for (;;) {
        ::ptrdiff_t result = ::PackBytes(
            static_cast<char *>(stream_->getBuffer()) + writtenByteCount_,
            stream_->getBufferSize() - writtenByteCount_,
            string.data(),
            string.size()
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
Archive::operator>>(std::string &string)
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
    string.append(bytes, numberOfBytes);
    return *this;
}


Archive &
Archive::operator<<(varint_t varint)
{
    for (;;) {
        ::ptrdiff_t result = ::PackVariableLengthInteger(
            static_cast<char *>(stream_->getBuffer()) + writtenByteCount_,
            stream_->getBufferSize() - writtenByteCount_,
            static_cast<::intmax_t>(varint)
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
Archive::operator>>(varint_t &varint)
{
    ::ptrdiff_t result = ::UnpackVariableLengthInteger(
        static_cast<const char *>(stream_->getData()) + readByteCount_,
        stream_->getDataSize() + writtenByteCount_ - readByteCount_,
        reinterpret_cast<::intmax_t *>(&varint)
    );

    if (result < 0) {
        throw GINK_SYSTEM_ERROR(errno, "`::UnpackVariableLengthInteger()` failed");
    }

    readByteCount_ += result;
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

} // namespace Gink
