#include <algorithm>
#include <bit>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <print>
#include <string_view>
#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>

#include "serpent/layout.hpp"
#include "serpent/interner.hpp"

void Serpent::DefaultInitialize(Serpent::ValueLayout const &layout, void *ptr) {
    std::visit(
        [ptr](auto &value) {
            using T = std::decay_t<decltype(value)>;
            std::println("{}: {}", ptr, typeid(T).name());
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
                        *reinterpret_cast<uint8_t *>(ptr) = 0;
                        break;
                    case IntegralLayout::Int16:
                    case IntegralLayout::UInt16:
                        *reinterpret_cast<uint16_t *>(ptr) = 0;
                        break;
                    case IntegralLayout::Int32:
                    case IntegralLayout::UInt32:
                        *reinterpret_cast<uint32_t *>(ptr) = 0;
                        break;
                    case IntegralLayout::Int64:
                    case IntegralLayout::UInt64:
                        *reinterpret_cast<uint64_t *>(ptr) = 0;
                        break;
                }
            } else if constexpr (std::same_as<T, FloatingLayout>) {
                switch (value) {
                    case FloatingLayout::Float32:
                        *reinterpret_cast<float *>(ptr) = 0;
                        break;
                    case FloatingLayout::Float64:
                        *reinterpret_cast<double *>(ptr) = 0;
                        break;
                }
            } else if constexpr (std::same_as<T, ObjectLayout> || std::same_as<T, VariantLayout> || std::same_as<T, ArrayLayout>) {
                value.DefaultInitialize(ptr);
            } else if constexpr (std::same_as<T, PrimitiveLayout>) {
                switch (value) {
                    case PrimitiveLayout::String:
                        *reinterpret_cast<size_t *>(ptr) = Interner::Instance().Acquire("");
                        break;
                    case PrimitiveLayout::Unit:
                        break;
                }
            }
        },
        layout
    );
}

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

            std::unreachable();
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

            std::unreachable();
        },
        layout
    );
}

Serpent::ObjectLayout::ObjectLayout(
    std::vector<Field> fields,
    std::unordered_map<Serpent::InternedString, size_t> indices,
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
    std::unordered_map<Serpent::InternedString, size_t> indices;
    size_t size = 0;
    size_t align = 1;

    for (auto const &field : init) {
        auto const &layout = field.Layout();
        size_t const fieldAlign = GetAlign(layout);
        size_t const fieldSize  = GetSize(layout);

        offset = (offset + fieldAlign - 1) & ~(fieldAlign - 1);

        auto name = field.Name();

        if (indices.contains(name))
            return std::nullopt;

        indices.insert({name, fields.size()});

        fields.emplace_back(ObjectLayout::Field {
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

void Serpent::ObjectLayout::DefaultInitialize(void *data) const {
    for (auto const &field : fields) {
        std::println("Field {}", field.layout.Name());
        Serpent::DefaultInitialize(field.layout.Layout(), reinterpret_cast<void *>(size_t(data) + field.offset));
    }
}

Serpent::VariantLayout::VariantLayout(
    std::vector<NamedLayout> variants,
    std::unordered_map<InternedString, size_t> indices,
    std::optional<InternedString> variantFieldName,
    size_t tagSize,
    size_t size,
    size_t align
) :
    variants(variants),
    indices(indices),
    variantFieldName(variantFieldName),
    tagSize(tagSize),
    size(size),
    align(align)
{}

std::optional<Serpent::VariantLayout> Serpent::VariantLayout::Of(std::initializer_list<NamedLayout> init, std::optional<std::string_view> variantFieldName) {
    std::vector<NamedLayout> variants {init};
    std::unordered_map<InternedString, size_t> indices;
    size_t size = 0;
    size_t align = 1;

    size_t tmp = variants.size();
    while (tmp > 0) {
        tmp >>= 8;
        size += 1;
    }
    size = std::bit_ceil(size);
    size_t tagSize = size;
    align = std::max(tagSize, size_t(1));

    for (auto const &variant : variants) {
        auto const &layout = variant.Layout();
        size_t const variantAlign = GetAlign(layout);
        align = std::max(align, variantAlign);
    }

    for (size_t i = 0; i < variants.size(); i++) {
        auto const &variant = variants[i];
        auto const &layout = variant.Layout();
        size_t const variantAlign = GetAlign(layout);
        size_t const variantSize  = GetSize(layout);

        InternedString name = variant.Name();

        if (indices.contains(name))
            return std::nullopt;

        size = std::max(size, variantSize);

        indices.insert({name, i});
    }

    size = (size + align - 1) & ~(align - 1);

    return VariantLayout(variants, indices, variantFieldName, tagSize, size, align);
}

size_t Serpent::VariantLayout::Size() const {
    return size;
}

size_t Serpent::VariantLayout::Align() const {
    return align;
}

void Serpent::VariantLayout::DefaultInitialize(void *data) const {
    switch (tagSize) {
        case 1:
            *reinterpret_cast<uint8_t *>(data) = 0;
            break;
        case 2:
            *reinterpret_cast<uint16_t *>(data) = 0;
            break;
        case 4:
            *reinterpret_cast<uint32_t *>(data) = 0;
            break;
        case 8:
            *reinterpret_cast<uint64_t *>(data) = 0;
            break;
    }

    if (variants.empty())
        return;

    Serpent::DefaultInitialize(variants[0].Layout(), reinterpret_cast<void *>(size_t(data) + align));
}

Serpent::ArrayLayout::ArrayLayout(Serpent::ValueLayout &&layout) :
    layout(std::make_unique<ValueLayout>(layout))
{}

Serpent::ArrayLayout::ArrayLayout(Serpent::ArrayLayout const &copy) :
    layout(std::make_unique<ValueLayout>(*copy.layout))
{}

Serpent::ArrayLayout Serpent::ArrayLayout::Of(ValueLayout &&layout) {
    return ArrayLayout(std::move(layout));
}

Serpent::ValueLayout const &Serpent::ArrayLayout::Layout() const {
    return *layout;
}

bool Serpent::ArrayLayout::operator == (ArrayLayout const &rhs) const {
    if (this == &rhs)
        return true;

    return Layout() == rhs.Layout();
}

void Serpent::ArrayLayout::DefaultInitialize(void *data) const {
    Backing *backing = reinterpret_cast<Backing *>(data);

    backing->data = 0;
    backing->capacity = 0;
    backing->size = 0;
}

Serpent::EnumLayout::EnumLayout(
    IntegralLayout backing,
    std::vector<InternedString> names
) :
    backing(backing),
    names(names)
{}

std::optional<Serpent::EnumLayout> Serpent::EnumLayout::Of(std::initializer_list<std::string_view> names, IntegralLayout backing) {
    std::vector<InternedString> values;
    std::unordered_set<InternedString> set;

    for (std::string_view name : names) {
        if (set.contains(name))
            return std::nullopt;
        
        values.emplace_back(name);
        set.emplace(name);
    }

    return EnumLayout(backing, values);
}

Serpent::IntegralLayout Serpent::EnumLayout::Backing() const {
    return backing;
}

Serpent::NamedLayout::NamedLayout(
    std::string_view name,
    ValueLayout &&layout
) :
    name(name),
    layout(layout)
{}

std::string_view Serpent::NamedLayout::Name() const {
    return name.Value();
}

Serpent::ValueLayout const &Serpent::NamedLayout::Layout() const {
    return layout;
}
