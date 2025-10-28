#pragma once

#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <memory>
#include <optional>
#include <string_view>
#include <variant>
#include <vector>

#include "api.hpp"
#include "interner.hpp"

namespace Serpent {
    enum struct IntegralLayout : uint8_t {
        Bool,
        UInt8,
        Int8,
        UInt16,
        Int16,
        UInt32,
        Int32,
        UInt64,
        Int64,
    };

    enum struct FloatingLayout : uint8_t {
        Float32,
        Float64,
    };

    enum struct PrimitiveLayout : uint8_t {
        /// Represents an interned string value
        String,
        /// Represents a unit type, can only have one value
        Unit,
    };

    struct ObjectLayout;
    struct VariantLayout;
    struct ArrayLayout;
    struct EnumLayout;

    struct NamedLayout;

    using ValueLayout = std::variant<
        IntegralLayout,
        FloatingLayout,
        PrimitiveLayout,
        EnumLayout,
        ObjectLayout,
        VariantLayout,
        ArrayLayout
    >;

    SERPENT_API void DefaultInitialize(ValueLayout const &layout, void *value);

    SERPENT_API size_t GetSize(ValueLayout const &layout);
    SERPENT_API size_t GetAlign(ValueLayout const &layout);

    struct SERPENT_API ObjectLayout final {
        private:
        struct Field;

        std::vector<Field> fields;
        std::unordered_map<InternedString, size_t> indices;
        size_t size;
        size_t align;

        ObjectLayout(
            std::vector<Field> fields,
            std::unordered_map<InternedString, size_t> indices,
            size_t size,
            size_t align
        );
        
        public:
        /// Returns nullopt if there are duplicated field names
        static std::optional<ObjectLayout> Of(std::initializer_list<NamedLayout> fields); 

        size_t Size() const;
        size_t Align() const;

        bool operator == (ObjectLayout const &rhs) const = default;

        void DefaultInitialize(void *data) const;
    };

    struct SERPENT_API VariantLayout final {
        private:
        std::vector<NamedLayout> variants;
        std::unordered_map<InternedString, size_t> indices;
        std::optional<InternedString> variantFieldName;
        size_t tagSize;
        size_t size;
        size_t align;

        VariantLayout(
            std::vector<NamedLayout> variants,
            std::unordered_map<InternedString, size_t> indices,
            std::optional<InternedString> variantFieldName,
            size_t tagSize,
            size_t size,
            size_t align
        );
        
        public:
        /// Returns nullopt if there are duplicated field names
        /// if variantFieldName is nullopt, it uses the name of the variant as a key
        /// `{"SomeVariant": 5}` vs `{"type": "SomeVariant", "value": 5}`
        static std::optional<VariantLayout> Of(std::initializer_list<NamedLayout> fields, std::optional<std::string_view> variantFieldName = std::nullopt);

        size_t Size() const;
        size_t Align() const;

        bool operator == (VariantLayout const &rhs) const = default;

        void DefaultInitialize(void *data) const;
    };

    struct SERPENT_API ArrayLayout final {
        struct Backing {
            void *data;
            size_t size;
            size_t capacity;
        };

        private:
        std::unique_ptr<ValueLayout> layout;

        ArrayLayout(ValueLayout &&layout);
        
        public:
        ArrayLayout(ArrayLayout &&move) = default;
        /// Deep clones the object
        ArrayLayout(ArrayLayout const &copy);

        static ArrayLayout Of(ValueLayout &&layout);

        ValueLayout const &Layout() const;

        /// Deep equality
        bool operator == (ArrayLayout const &rhs) const;

        void DefaultInitialize(void *data) const;
    };

    struct SERPENT_API EnumLayout final {
        private:
        IntegralLayout backing;
        std::vector<InternedString> names;

        EnumLayout(
            IntegralLayout backing,
            std::vector<InternedString> names
        );

        public:
        /// Returns nullopt if there are duplicated names
        static std::optional<EnumLayout> Of(std::initializer_list<std::string_view> names, IntegralLayout backing = IntegralLayout::UInt32);

        IntegralLayout Backing() const;

        bool operator == (EnumLayout const &rhs) const = default;
    };

    struct SERPENT_API NamedLayout final {
        private:
        InternedString name;
        ValueLayout layout;

        public:
        NamedLayout(std::string_view name, ValueLayout &&layout);

        std::string_view Name() const;
        ValueLayout const &Layout() const;

        bool operator == (NamedLayout const &rhs) const = default;
    };

    struct SERPENT_API ObjectLayout::Field final {
        NamedLayout layout;
        size_t offset;

        bool operator == (Field const &rhs) const = default;
    };
}
