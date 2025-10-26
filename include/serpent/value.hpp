#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>

#include "serpent/interner.hpp"
#include "serpent/layout.hpp"

namespace Serpent {
    struct Value final {
        friend struct ValueHandle;
        private:
        ValueLayout const &Layout;
        std::atomic_size_t refCount = 1;
        uint8_t data[];

        /// Only here for placement new
        Value(ValueLayout const &layout);

        void IncrementRefCount();
        /// Returns true if the ref count reaches 0
        bool DecrementRefCount();

        public:
        /// Values can only be created or destroyed by Serpent.
        Value() = delete;
        Value(Value const &copy) = delete;
        Value(Value &&copy) = delete;
        ~Value() = delete;
    };

    struct ValueReference;

    /// Represents a handle for a Value.
    struct ValueHandle final {
        private:
        Value *value;

        public:
        /// Expects the underlying layout to live at least as long as any values referencing it
        ValueHandle(ValueLayout const &layout);

        ValueHandle(ValueHandle const &copy);
        ValueHandle &operator = (ValueHandle const &copy);

        ValueHandle(ValueHandle &&move);
        ValueHandle &operator = (ValueHandle &&copy);

        ~ValueHandle();

        ValueReference GetRoot();
    };

    /// Represents a reference to a value owned by a handle.
    struct ValueReference final {
        private:
        ValueHandle handle;
        void *ptr;

        ValueReference(ValueHandle handle, void *ptr);
        
        public:
        bool IsObject() const;
        bool IsArray() const;

        bool IsString() const;
        
        bool IsIntegral() const;
        bool IsSignedIntegral() const;
        bool IsUnsignedIntegral() const;
        bool IsBool() const;
        bool IsInt8() const;
        bool IsUInt8() const;
        bool IsInt16() const;
        bool IsUInt16() const;
        bool IsInt32() const;
        bool IsUInt32() const;
        bool IsInt64() const;
        bool IsUInt64() const;
        
        bool IsFloating() const;
        bool IsFloat32() const;
        bool IsFloat64() const;

        std::optional<ValueReference> operator [] (std::string_view field) const;
        std::optional<ValueReference> operator [] (size_t index) const;

        std::optional<InternedString> AsString() const;

        std::optional<int64_t> AsSignedIntegral() const;
        std::optional<uint64_t> AsUnsignedIntegral() const;
        std::optional<bool> AsBool() const;
        std::optional<int8_t> AsInt8() const;
        std::optional<uint8_t> AsUInt8() const;
        std::optional<int16_t> AsInt16() const;
        std::optional<uint16_t> AsUInt16() const;
        std::optional<int32_t> AsInt32() const;
        std::optional<uint32_t> AsUInt32() const;
        std::optional<int64_t> AsInt64() const;
        std::optional<uint64_t> AsUInt64() const;

        std::optional<double> AsFloating() const;
        std::optional<float> AsFloat32() const;
        std::optional<double> AsFloat64() const;

        bool Set(InternedString value);
        bool Set(bool value);
        bool Set(int8_t value);
        bool Set(uint8_t value);
        bool Set(int16_t value);
        bool Set(uint16_t value);
        bool Set(int32_t value);
        bool Set(uint32_t value);
        bool Set(int64_t value);
        bool Set(uint64_t value);
        bool Set(float value);
        bool Set(double value);
        
        /// Sets the value of an integral or floating point type by casting lossily
        bool SetUnexact(int64_t value);
        /// Sets the value of an integral or floating point type by casting lossily
        bool SetUnexact(uint64_t value);
        /// Sets the value of an integral or floating point type by casting lossily
        bool SetUnexact(double value);
    };
}
