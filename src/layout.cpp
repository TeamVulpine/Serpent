#include <algorithm>
#include <cassert>
#include <concepts>
#include <cstddef>
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
            } else if constexpr (std::same_as<T, StringLayout>) {
                return sizeof(InternedString);
            } else if constexpr (std::same_as<T, ObjectLayout> || std::same_as<T, VariantLayout>) {
                return value.Size();
            } else if constexpr (std::same_as<T, ArrayLayout>) {
                return sizeof(ArrayLayout::Backing);
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
            } else if constexpr (std::same_as<T, StringLayout>) {
                return alignof(InternedString);
            } else if constexpr (std::same_as<T, ObjectLayout> || std::same_as<T, VariantLayout>) {
                return value.Align();
            } else if constexpr (std::same_as<T, ArrayLayout>) {
                return alignof(ArrayLayout::Backing);
            }
        },
        layout
    );
}

std::optional<Serpent::ObjectLayout> Serpent::ObjectLayout::Of(std::initializer_list<FieldLayout> init) {
    size_t offset = 0;
    std::vector<ObjectLayout::Field> fields {};
    std::unordered_map<InternedString, size_t> indices;
    size_t size = 0;
    size_t align = 1;

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
