#include <algorithm>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <memory>
#include <optional>
#include <variant>

#include "serpent/layout.hpp"
#include "serpent/interner.hpp"

size_t Serpent::GetSize(ValueLayout const &layout) {
    return std::visit(
        [](auto const &value) -> size_t {
            using T = std::decay_t<decltype(value)>;
            if constexpr (std::same_as<T, IntegralLayout> || std::same_as<T, EnumLayout>) {
                IntegralLayout integral;
                if constexpr (std::same_as<T, IntegralLayout>) {
                    integral = value;
                } else {
                    integral = value.Backing();
                }

                switch (integral) {
                    case IntegralLayout::Bool:
                    case IntegralLayout::Int8:
                    case IntegralLayout::UInt8:
                        return 1;
                    case IntegralLayout::Int16:
                    case IntegralLayout::UInt16:
                        return 2;
                    case IntegralLayout::Int32:
                    case IntegralLayout::UInt32:
                        return 4;
                    case IntegralLayout::Int64:
                    case IntegralLayout::UInt64:
                        return 8;
                }
            } else if constexpr (std::same_as<T, FloatingLayout>) {
                switch (value) {
                    case FloatingLayout::Float32:
                        return 4;
                    case FloatingLayout::Float64:
                        return 8;
                }
            } else if constexpr (std::same_as<T, ObjectLayout> || std::same_as<T, VariantLayout>) {
                return value.Size();
            } else if constexpr (std::same_as<T, ArrayLayout>) {
                return sizeof(ArrayLayout::Backing);
            } else if constexpr (std::same_as<T, PrimitiveLayout>) {
                switch (value) {
                    case PrimitiveLayout::String:
                        return sizeof(InternedString);
                    case PrimitiveLayout::Unit:
                        return 0;
                }
            }
        },
        layout
    );
}

size_t Serpent::GetAlign(ValueLayout const &layout) {
    return std::visit(
        [](auto &value) -> size_t {
            using T = std::decay_t<decltype(value)>;
            if constexpr (std::same_as<T, IntegralLayout> || std::same_as<T, EnumLayout>) {
                IntegralLayout integral;
                if constexpr (std::same_as<T, IntegralLayout>) {
                    integral = value;
                } else {
                    integral = value.Backing();
                }

                switch (integral) {
                    case IntegralLayout::Bool:
                    case IntegralLayout::Int8:
                    case IntegralLayout::UInt8:
                        return 1;
                    case IntegralLayout::Int16:
                    case IntegralLayout::UInt16:
                        return 2;
                    case IntegralLayout::Int32:
                    case IntegralLayout::UInt32:
                        return 4;
                    case IntegralLayout::Int64:
                    case IntegralLayout::UInt64:
                        return 8;
                }
            } else if constexpr (std::same_as<T, FloatingLayout>) {
                switch (value) {
                    case FloatingLayout::Float32:
                        return 4;
                    case FloatingLayout::Float64:
                        return 8;
                }
            } else if constexpr (std::same_as<T, ObjectLayout> || std::same_as<T, VariantLayout>) {
                return value.Align();
            } else if constexpr (std::same_as<T, ArrayLayout>) {
                return alignof(ArrayLayout::Backing);
            } else if constexpr (std::same_as<T, PrimitiveLayout>) {
                switch (value) {
                    case PrimitiveLayout::String:
                        return alignof(InternedString);
                    case PrimitiveLayout::Unit:
                        return 1;
                }
            }
        },
        layout
    );
}

Serpent::ObjectLayout::ObjectLayout(
    std::vector<Field> fields,
    std::unordered_map<InternedString, size_t> indices,
    size_t size,
    size_t align
) :
    fields(fields),
    indices(indices),
    size(size),
    align(align)
{}

std::optional<Serpent::ObjectLayout> Serpent::ObjectLayout::Of(std::initializer_list<NamedLayout> init) {
    size_t offset = 0;
    std::vector<ObjectLayout::Field> fields {};
    std::unordered_map<InternedString, size_t> indices;
    size_t size = 0;
    size_t align = 0;

    for (auto const &field : init) {
        auto const &layout = field.Layout();
        size_t const fieldAlign = GetAlign(layout);
        size_t const fieldSize  = GetSize(layout);

        offset = (offset + fieldAlign - 1) & ~(fieldAlign - 1);

        InternedString name = field.Name();

        if (indices.contains(name))
            return std::nullopt;

        indices.emplace(name, fields.size());

        fields.push_back(ObjectLayout::Field {
            .layout = field,
            .offset = offset
        });

        align = std::max(align, fieldAlign);

        offset += fieldSize;
    }

    size = (offset + align - 1) & ~(align - 1);

    return ObjectLayout(fields, indices, size, align);
}

size_t Serpent::ObjectLayout::Size() const {
    return size;
};

size_t Serpent::ObjectLayout::Align() const {
    return align;
};

Serpent::VariantLayout::VariantLayout(
    std::vector<NamedLayout> variants,
    std::unordered_map<InternedString, size_t> indices,
    std::optional<InternedString> variantFieldName,
    size_t size,
    size_t align
) :
    variants(variants),
    indices(indices),
    variantFieldName(variantFieldName),
    size(size),
    align(align)
{}

std::optional<Serpent::VariantLayout> Serpent::VariantLayout::Of(std::initializer_list<NamedLayout> init, std::optional<std::string_view> variantFieldName) {
    std::vector<NamedLayout> variants {init};
    std::unordered_map<InternedString, size_t> indices;
    size_t size = 0;
    size_t align = 0;

    for (auto const &variant : variants) {
        auto const &layout = variant.Layout();
        size_t const variantAlign = GetAlign(layout);
        size_t const variantSize  = GetSize(layout);

        InternedString name = variant.Name();

        if (indices.contains(name))
            return std::nullopt;

        size = std::max(size, variantSize);

        align = std::max(align, variantAlign);
    }

    size = (size + align - 1) & ~(align - 1);

    return VariantLayout(variants, indices, variantFieldName, size, align);
}

size_t Serpent::VariantLayout::Size() const {
    return size;
};

size_t Serpent::VariantLayout::Align() const {
    return align;
};

Serpent::ArrayLayout::ArrayLayout(Serpent::ValueLayout &&layout) :
    layout(std::make_unique<ValueLayout>(layout))
{}

Serpent::ArrayLayout::ArrayLayout(Serpent::ArrayLayout const &copy) :
    layout(std::make_unique<ValueLayout>(*copy.layout))
{}

Serpent::ValueLayout const &Serpent::ArrayLayout::Layout() const {
    return *layout;
}

bool Serpent::ArrayLayout::operator == (ArrayLayout const &rhs) const {
    if (this == &rhs)
        return true;

    return Layout() == rhs.Layout();
}

Serpent::InternedString Serpent::NamedLayout::Name() const {
    return name;
}

Serpent::ValueLayout const &Serpent::NamedLayout::Layout() const {
    return layout;
}
