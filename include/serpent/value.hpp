#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <string_view>
#include <sys/types.h>
#include <variant>

#include "serpent/api.hpp"
#include "serpent/types/interner.hpp"
#include "serpent/layout.hpp"

namespace Serpent {
    struct GcHandle;
    struct ArrayHandle;

    using Handle = std::variant<
        std::monostate,
        std::nullopt_t,
        int8_t,
        uint8_t,
        int16_t,
        uint16_t,
        int32_t,
        uint32_t,
        int64_t,
        uint64_t,
        float,
        double,
        InternedString,
        GcHandle,
        ArrayHandle
    >;

    struct GcValue;

    struct SERPENT_API GcHandle final {
        private:
        GcValue *value;

        GcHandle(GcValue *value);

        public:
        ~GcHandle();

        static GcHandle Create(std::shared_ptr<GcLayout const> &layout);
        static GcHandle FromRaw(GcValue *SERPENT_NONNULL raw);

        Handle Get(std::string_view key);
        Handle Get(size_t index);

        bool Set(std::string_view key, Handle value);
        bool Set(size_t index, Handle value);

        /// Leaks the value into a raw GcValue pointer. To reacquire the value, call FromRaw
        GcValue *SERPENT_NONNULL IntoRaw();
    };

    struct ArrayValue;

    struct SERPENT_API ArrayHandle final {
        private:
        ArrayValue *value;

        ArrayHandle(ArrayValue *value);

        public:
        ~ArrayHandle();

        static ArrayHandle Create(ArrayLayout &layout);
        static GcHandle FromRaw(ArrayValue *SERPENT_NONNULL raw);

        size_t Length();

        Handle Get(size_t index);

        bool Set(size_t index, Handle value);
        /// Leaks the value into a raw ArrayValue pointer. To reacquire the value, call FromRaw
        ArrayValue *SERPENT_NONNULL IntoRaw();
    };
}
