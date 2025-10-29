#pragma once

#include <cstddef>
#include <cstdint>
#include <expected>
#include <string_view>

namespace Serpent {
    enum struct StructuredStreamError : uint8_t {
        ArrayLengthMismatch,
        KeyNotExists,
        InvalidType,
    };

    template <typename T>
    using StructuredStreamResult = std::expected<T, StructuredStreamError>;

    class StructuredInputStream {
        public:
        virtual ~StructuredInputStream() = default;

        virtual StructuredStreamResult<StructuredInputStream *> BeginObject() = 0;
        virtual StructuredStreamResult<StructuredInputStream *> WriteKey(std::string_view key) = 0;
        virtual StructuredStreamResult<StructuredInputStream *> EndObject() = 0;

        virtual StructuredStreamResult<StructuredInputStream *> BeginArray(size_t length) = 0;
        virtual StructuredStreamResult<StructuredInputStream *> EndArray() = 0;

        virtual StructuredStreamResult<StructuredInputStream *> WriteString(std::string_view value) = 0;
        virtual StructuredStreamResult<StructuredInputStream *> WriteBool(bool value) = 0;
        virtual StructuredStreamResult<StructuredInputStream *> WriteInt8(int8_t value) = 0;
        virtual StructuredStreamResult<StructuredInputStream *> WriteUint8(uint8_t value) = 0;
        virtual StructuredStreamResult<StructuredInputStream *> WriteInt16(int16_t value) = 0;
        virtual StructuredStreamResult<StructuredInputStream *> WriteUint16(uint16_t value) = 0;
        virtual StructuredStreamResult<StructuredInputStream *> WriteInt32(int32_t value) = 0;
        virtual StructuredStreamResult<StructuredInputStream *> WriteUint32(uint32_t value) = 0;
        virtual StructuredStreamResult<StructuredInputStream *> WriteInt64(int64_t value) = 0;
        virtual StructuredStreamResult<StructuredInputStream *> WriteUint64(uint64_t value) = 0;
        virtual StructuredStreamResult<StructuredInputStream *> WriteFloat32(float value) = 0;
        virtual StructuredStreamResult<StructuredInputStream *> WriteFloat64(double value) = 0;
        virtual StructuredStreamResult<StructuredInputStream *> WriteEnum(std::string_view value) = 0;
        virtual StructuredStreamResult<StructuredInputStream *> WriteVariant(std::string_view variant) = 0;
    };

    class StructuredOutputStream {
        public:
        virtual ~StructuredOutputStream() = default;

        virtual StructuredStreamResult<StructuredOutputStream *> BeginObject() = 0;
        virtual StructuredStreamResult<StructuredOutputStream *> ReadKey(std::string_view key) = 0;
        virtual StructuredStreamResult<StructuredOutputStream *> EndObject() = 0;

        virtual StructuredStreamResult<StructuredInputStream *> BeginArray() = 0;
        virtual StructuredStreamResult<size_t> ArrayLength() = 0;
        virtual StructuredStreamResult<StructuredInputStream *> EndArray() = 0;

        virtual StructuredStreamResult<std::string> ReadString() = 0;
        virtual StructuredStreamResult<bool> ReadBool() = 0;
        virtual StructuredStreamResult<int8_t> ReadInt8() = 0;
        virtual StructuredStreamResult<uint8_t> ReadUint8() = 0;
        virtual StructuredStreamResult<int16_t> ReadInt16() = 0;
        virtual StructuredStreamResult<uint16_t> ReadUint16() = 0;
        virtual StructuredStreamResult<int32_t> ReadInt32() = 0;
        virtual StructuredStreamResult<uint32_t> ReadUint32() = 0;
        virtual StructuredStreamResult<int64_t> ReadInt64() = 0;
        virtual StructuredStreamResult<uint64_t> ReadUint64() = 0;
        virtual StructuredStreamResult<float> ReadFloat32() = 0;
        virtual StructuredStreamResult<double> ReadFloat64() = 0;
        virtual StructuredStreamResult<std::string> ReadEnum() = 0;
        virtual StructuredStreamResult<std::string> ReadVariant() = 0;
    };
}
