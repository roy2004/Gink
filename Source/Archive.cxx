#include "Archive.h"

#include <climits>
#include <cerrno>
#include <cstring>

#include "Stream.h"
#include "SystemError.h"


namespace Gink {


#define INTEGER_SERIALIZER(n)                                                        \
    void                                                                             \
    Archive::serializeInteger(std::int##n##_t integer)                               \
    {                                                                                \
        std::size_t bufferSize = stream_->getBufferSize() - writtenByteCount_;       \
                                                                                     \
        if (bufferSize < sizeof integer) {                                           \
            stream_->growBuffer(sizeof integer - bufferSize);                        \
        }                                                                            \
                                                                                     \
        auto buffer = static_cast<char *>(stream_->getBuffer()) + writtenByteCount_; \
        buffer[sizeof integer - 1] = integer;                                        \
        std::ptrdiff_t i;                                                            \
                                                                                     \
        for (i = sizeof integer - 2; i >= 0; --i) {                                  \
            integer = static_cast<std::uint##n##_t>(integer) >> CHAR_BIT;            \
            buffer[i] = integer;                                                     \
        }                                                                            \
                                                                                     \
        writtenByteCount_ += sizeof integer;                                         \
    }

#define INTEGER_DESERIALIZER(n)                                                     \
    void                                                                            \
    Archive::deserializeInteger(std::int##n##_t *integer)                           \
    {                                                                               \
        std::size_t dataSize = stream_->getDataSize() - readByteCount_;             \
                                                                                    \
        if (dataSize < sizeof *integer) {                                           \
            throw GINK_SYSTEM_ERROR(ENODATA, "deserialize failed");                 \
        }                                                                           \
                                                                                    \
        auto data = static_cast<const char *>(stream_->getData()) + readByteCount_; \
        *integer = static_cast<unsigned char>(data[0]);                             \
        std::ptrdiff_t i;                                                           \
                                                                                    \
        for (i = 1; i < static_cast<std::ptrdiff_t>(sizeof *integer); ++i) {        \
            *integer = *integer << CHAR_BIT | static_cast<unsigned char>(data[i]);  \
        }                                                                           \
                                                                                    \
        readByteCount_ += sizeof *integer;                                          \
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
Archive::serializeVariableLengthInteger(std::intmax_t integer)
{
    std::intmax_t integerSign = integer >> (sizeof integer * CHAR_BIT - 1);

    if (integer >> 6 == integerSign) {
        serializeInteger(static_cast<std::int8_t>(integer & UINTMAX_C(0x7F)));
        return;
    }

    if (integer >> 13 == integerSign) {
        serializeInteger(static_cast<std::int16_t>((integer | UINTMAX_C(0x8000))
                                                   & UINTMAX_C(0xBFFF)));
        return;
    }

    if (integer >> 28 == integerSign) {
        serializeInteger(static_cast<std::int32_t>((integer | UINTMAX_C(0xC0000000))
                                                   & UINTMAX_C(0xDFFFFFFF)));
        return;
    }

    if (integer >> 59 == integerSign) {
        serializeInteger(static_cast<std::int64_t>((integer | UINTMAX_C(0xE000000000000000))
                                                   & UINTMAX_C(0xEFFFFFFFFFFFFFFF)));
        return;
    }

    serializeInteger(static_cast<std::int8_t>(UINT8_MAX));
    serializeInteger(integer);
}


void
Archive::deserializeVariableLengthInteger(std::intmax_t *integer)
{
    std::int8_t integerHead;
    deserializeInteger(&integerHead);

    if ((integerHead & 1 << 7) == 0) {
        *integer = static_cast<std::int8_t>(static_cast<std::uint8_t>(integerHead) << 1) >> 1;
        return;
    }

    if ((integerHead & 1 << 6) == 0) {
        readByteCount_ -= sizeof integerHead;
        std::int16_t temp;
        deserializeInteger(&temp);
        *integer = static_cast<std::int16_t>(static_cast<std::uint16_t>(temp) << 2) >> 2;
        return;
    }

    if ((integerHead & 1 << 5) == 0) {
        readByteCount_ -= sizeof integerHead;
        std::int32_t temp;
        deserializeInteger(&temp);
        *integer = static_cast<std::int32_t>(static_cast<std::uint32_t>(temp) << 3) >> 3;
        return;
    }

    if ((integerHead & 1 << 4) == 0) {
        readByteCount_ -= sizeof integerHead;
        std::int64_t temp;
        deserializeInteger(&temp);
        *integer = static_cast<std::int64_t>(static_cast<std::uint64_t>(temp) << 4) >> 4;
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
    std::intmax_t temp;
    deserializeVariableLengthInteger(&temp);
    *numberOfBytes = temp;
    std::size_t dataSize = stream_->getDataSize() - readByteCount_;

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
