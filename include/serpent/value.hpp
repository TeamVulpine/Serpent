#pragma once

#include <cstddef>
#include <cstdint>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <string_view>

#include "serpent/api.hpp"
#include "serpent/layout.hpp"

namespace Serpent {
    struct SERPENT_API Value final {
        friend struct ValueHandle;
        private:
        ValueLayout const &Layout;
        std::shared_mutex mutex;
        size_t refCount = 1;
        size_t generation = 0;
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
    struct SERPENT_API ValueHandle final {
        private:
        Value *value;

        public:
        /// Expects the underlying layout to live at least as long as any values referencing it
        ValueHandle(ValueLayout const &layout);

        ValueHandle(ValueHandle const &copy);
        ValueHandle &operator = (ValueHandle const &copy);

        ValueHandle(ValueHandle &&move);
        ValueHandle &operator = (ValueHandle &&move);

        ~ValueHandle();

        ValueReference GetRoot();
    };

    struct ValueView;
    struct ValueViewMut;

    /// Represents a reference to a value owned by a handle.
    struct SERPENT_API ValueReference final {
        private:
        ValueHandle handle;
        /// The layout of the referenced value, not the layout of the whole object
        ValueLayout const &Layout;
        void *ptr;
        size_t generation;

        ValueReference(ValueHandle handle, void *ptr);

        public:
        /// Returns true if the reference still points to valid data.
        /// A reference becomes invalid if the underlying value has changed generation,
        ///   i.e. when an array is overwritten or resized.
        bool IsValid() const;

        ValueView Read() const;
        ValueViewMut Write();

        bool IsObject() const;
        bool IsArray() const;
        bool IsVariant() const;
        bool IsVariant(std::string_view variant) const;
        bool IsEnum() const;
        bool IsEnum(std::string_view value) const;
        bool IsString() const;
        bool IsUnit() const;
        bool IsBool() const;
        bool IsInt8() const;
        bool IsUint8() const;
        bool IsInt16() const;
        bool IsUint16() const;
        bool IsInt32() const;
        bool IsUint32() const;
        bool IsInt64() const;
        bool IsUint64() const;
        bool IsFloat32() const;
        bool IsFloat64() const;
    };

    struct SERPENT_API ValueView final {
        private:
        ValueReference ref;
        std::shared_lock<std::shared_mutex> lock;

        public:
        ValueView(ValueReference ref);

        ValueReference &Ref();
        ValueReference const &Ref() const;
        ValueReference *operator * ();
        ValueReference const *operator * () const;
        ValueReference *operator -> ();
        ValueReference const *operator -> () const;

        /// Returns true if the reference still points to valid data.
        /// A reference becomes invalid if the underlying value has changed generation,
        ///   i.e. when an array is overwritten or resized.
        bool IsValid() const;

        /// If the value is an object, it returns the reference for that field, if it exists
        /// If the value is a variant, it returns the reference for that variant, if it is that variant
        std::optional<ValueReference> operator [] (std::string_view field) const;
        /// If the value is an array, it returns the reference to that index
        std::optional<ValueReference> operator [] (size_t index) const;

        std::optional<std::string_view> GetVariant() const;
        std::optional<std::string_view> GetEnum() const;
        std::optional<std::string_view> GetString() const;
        std::optional<bool> GetBool() const;
        std::optional<int8_t> GetInt8() const;
        std::optional<uint8_t> GetUint8() const;
        std::optional<int16_t> GetInt16() const;
        std::optional<uint16_t> GetUint16() const;
        std::optional<int32_t> GetInt32() const;
        std::optional<uint32_t> GetUint32() const;
        std::optional<int64_t> GetInt64() const;
        std::optional<uint64_t> GetUint64() const;
        std::optional<float> GetFloat32() const;
        std::optional<double> GetFloat64() const;
    };

    struct SERPENT_API ValueViewMut final {
        private:
        ValueReference ref;
        std::unique_lock<std::shared_mutex> lock;

        public:
        ValueViewMut(ValueReference ref);

        ValueReference &Ref();
        ValueReference const &Ref() const;
        ValueReference *operator * ();
        ValueReference const *operator * () const;
        ValueReference *operator -> ();
        ValueReference const *operator -> () const;

        /// Returns true if the reference still points to valid data.
        /// A reference becomes invalid if the underlying value has changed generation,
        ///   i.e. when an array is overwritten or resized.
        bool IsValid() const;

        /// If the value is an object, it returns the reference for that field, if it exists
        /// If the value is a variant, it returns the reference for that variant, if it is that variant
        std::optional<ValueReference> operator [] (std::string_view field) const;
        /// If the value is an array, it returns the reference to that index
        std::optional<ValueReference> operator [] (size_t index) const;

        std::optional<std::string_view> GetVariant() const;
        std::optional<std::string_view> GetEnum() const;
        std::optional<std::string_view> GetString() const;
        std::optional<bool> GetBool() const;
        std::optional<int8_t> GetInt8() const;
        std::optional<uint8_t> GetUint8() const;
        std::optional<int16_t> GetInt16() const;
        std::optional<uint16_t> GetUint16() const;
        std::optional<int32_t> GetInt32() const;
        std::optional<uint32_t> GetUint32() const;
        std::optional<int64_t> GetInt64() const;
        std::optional<uint64_t> GetUint64() const;
        std::optional<float> GetFloat32() const;
        std::optional<double> GetFloat64() const;

        bool SetVariant(std::string_view variant);
        bool SetEnum(std::string_view value);
        bool SetString(std::string_view value);
        bool SetBool(bool value);
        bool SetInt8(int8_t value);
        bool SetUint8(uint8_t value);
        bool SetInt16(int16_t value);
        bool SetUint16(uint16_t value);
        bool SetInt32(int32_t value);
        bool SetUint32(uint32_t value);
        bool SetInt64(int64_t value);
        bool SetUint64(uint64_t value);
        bool SetFloat32(float value);
        bool SetFloat64(double value);

        bool SetFromHandle(ValueHandle handle);
        bool SetFromReference(ValueReference reference);

        /// Pushes a default initialized value.
        /// Updates the generation of this, since we know how the data changed
        bool Push();

        /// Updates the generation of this, since we know how the data changed
        bool PushVariant(std::string_view variant);
        /// Updates the generation of this, since we know how the data changed
        bool PushEnum(std::string_view value);
        /// Updates the generation of this, since we know how the data changed
        bool PushString(std::string_view value);
        /// Updates the generation of this, since we know how the data changed
        bool PushBool(bool value);
        /// Updates the generation of this, since we know how the data changed
        bool PushInt8(int8_t value);
        /// Updates the generation of this, since we know how the data changed
        bool PushUint8(uint8_t value);
        /// Updates the generation of this, since we know how the data changed
        bool PushInt16(int16_t value);
        /// Updates the generation of this, since we know how the data changed
        bool PushUint16(uint16_t value);
        /// Updates the generation of this, since we know how the data changed
        bool PushInt32(int32_t value);
        /// Updates the generation of this, since we know how the data changed
        bool PushUint32(uint32_t value);
        /// Updates the generation of this, since we know how the data changed
        bool PushInt64(int64_t value);
        /// Updates the generation of this, since we know how the data changed
        bool PushUint64(uint64_t value);
        /// Updates the generation of this, since we know how the data changed
        bool PushFloat32(float value);
        /// Updates the generation of this, since we know how the data changed
        bool PushFloat64(double value);

        /// Updates the generation of this, since we know how the data changed
        bool PushFromHandle(ValueHandle handle);
        /// Updates the generation of this, since we know how the data changed
        bool PushFromReference(ValueReference reference);

        /// SAFETY:
        ///   This function performs a reinterpret_cast on the data,
        ///     the caller must ensure
        ///   1. The layout of the type exactly matches the layout of the referenced value,
        ///   2. The type is a trivially copyable POD struct (no virtual functions or dynamic members), and
        ///   3. The value pointed to by this ValueReference remains alive at least as long as the returned pointer.
        ///   Failure to follow these rules will result in undefined behavior.
        template <typename T>
        T *As();
    };
}
