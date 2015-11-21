#include "Archive.h"

#include <cerrno>
#include <cstring>

#include "Stream.h"
#include "SystemError.h"


namespace Gink {

#define INTEGER_SERIALIZER(n)                                                                 \
    void                                                                                      \
    Archive::serializeInteger(std::uint##n##_t integer)                                       \
    {                                                                                         \
        std::size_t bufferSize = stream_->getBufferSize() - writtenByteCount_;                \
                                                                                              \
        if (bufferSize < sizeof integer) {                                                    \
            stream_->growBuffer(sizeof integer - bufferSize);                                 \
        }                                                                                     \
                                                                                              \
        auto buffer = static_cast<unsigned char *>(stream_->getBuffer()) + writtenByteCount_; \
        buffer[sizeof integer - 1] = integer;                                                 \
        std::ptrdiff_t i;                                                                     \
                                                                                              \
        for (i = sizeof integer - 2; i >= 0; --i) {                                           \
            integer >>= std::numeric_limits<unsigned char>::digits;                           \
            buffer[i] = integer;                                                              \
        }                                                                                     \
                                                                                              \
        writtenByteCount_ += sizeof integer;                                                  \
    }

#define INTEGER_DESERIALIZER(n)                                                              \
    void                                                                                     \
    Archive::deserializeInteger(std::uint##n##_t *integer)                                   \
    {                                                                                        \
        std::size_t dataSize = stream_->getDataSize() + writtenByteCount_ - readByteCount_;  \
                                                                                             \
        if (dataSize < sizeof *integer) {                                                    \
            throw GINK_SYSTEM_ERROR(ENODATA, "deserialize failed");                          \
        }                                                                                    \
                                                                                             \
        auto data = static_cast<const unsigned char *>(stream_->getData()) + readByteCount_; \
        *integer = data[0];                                                                  \
        std::ptrdiff_t i;                                                                    \
                                                                                             \
        for (i = 1; i < static_cast<std::ptrdiff_t>(sizeof *integer); ++i) {                 \
            *integer = *integer << std::numeric_limits<unsigned char>::digits | data[i];     \
        }                                                                                    \
                                                                                             \
        readByteCount_ += sizeof *integer;                                                   \
    }


INTEGER_SERIALIZER(8)


INTEGER_DESERIALIZER(8)


INTEGER_SERIALIZER(16)


INTEGER_DESERIALIZER(16)


INTEGER_SERIALIZER(32)


INTEGER_DESERIALIZER(32)


INTEGER_SERIALIZER(64)


INTEGER_DESERIALIZER(64)


#undef INTEGER_SERIALIZER
#undef INTEGER_DESERIALIZER


void
Archive::serializeVariableLengthInteger(std::uintmax_t integer)
{
    std::uintmax_t temp = (integer ^ integer << 1) >> 1;

    if (temp >> 6 == 0) {
        serializeInteger(static_cast<std::uint8_t>(integer | 0x80));
        return;
    }

    if (temp >> 13 == 0) {
        serializeInteger(static_cast<std::uint16_t>((integer & 0x7FFF) | 0x4000));
        return;
    }

    if (temp >> 28 == 0) {
        serializeInteger(static_cast<std::uint32_t>((integer & 0x3FFFFFFF) | 0x20000000));
        return;
    }

    if (temp >> 59 == 0) {
        serializeInteger(static_cast<std::uint64_t>((integer & 0x1FFFFFFFFFFFFFFF)
                                                    | 0x1000000000000000));
        return;
    }

    serializeInteger(static_cast<std::uint8_t>(-1));
    serializeInteger(integer);
}


void
Archive::deserializeVariableLengthInteger(std::uintmax_t *integer)
{
    std::uint8_t integerHead;
    deserializeInteger(&integerHead);

    if ((integerHead & 0x80) != 0) {
        std::uint8_t temp = integerHead;
        *integer = (temp & ~0x80) | -(temp & 0x40);
        return;
    }

    if ((integerHead & 0x40) != 0) {
        readByteCount_ -= sizeof integerHead;
        std::uint16_t temp;
        deserializeInteger(&temp);
        *integer = (temp & ~0x4000) | -(temp & 0x2000);
        return;
    }

    if ((integerHead & 0x20) != 0) {
        readByteCount_ -= sizeof integerHead;
        std::uint32_t temp;
        deserializeInteger(&temp);
        *integer = (temp & ~0x20000000) | -(temp & 0x10000000);
        return;
    }

    if ((integerHead & 0x10) != 0) {
        readByteCount_ -= sizeof integerHead;
        std::uint64_t temp;
        deserializeInteger(&temp);
        *integer = (temp & ~0x1000000000000000) | -(temp & 0x0800000000000000);
        return;
    }

    deserializeInteger(integer);
}


void
Archive::serializeBytes(const char *bytes, std::size_t numberOfBytes)
{
    serializeVariableLengthInteger(numberOfBytes);
    std::size_t bufferSize = stream_->getBufferSize() - writtenByteCount_;

    if (bufferSize < numberOfBytes) {
        stream_->growBuffer(numberOfBytes - bufferSize);
    }

    auto buffer = static_cast<char *>(stream_->getBuffer()) + writtenByteCount_;
    std::memcpy(buffer, bytes, numberOfBytes);
    writtenByteCount_ += numberOfBytes;
}


void
Archive::deserializeBytes(const char **bytes, std::size_t *numberOfBytes)
{
    std::uintmax_t temp;
    deserializeVariableLengthInteger(&temp);
    *numberOfBytes = temp;
    std::size_t dataSize = stream_->getDataSize() + writtenByteCount_ - readByteCount_;

    if (dataSize < *numberOfBytes) {
        throw GINK_SYSTEM_ERROR(ENODATA, "deserialize failed");
    }

    auto data = static_cast<const char *>(stream_->getData()) + readByteCount_;
    *bytes = data;
    readByteCount_ += *numberOfBytes;
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
