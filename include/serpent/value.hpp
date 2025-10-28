#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>

#include "serpent/interner.hpp"
#include "serpent/layout.hpp"

namespace Serpent {
    struct SERPENT_API Value final {
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

    /// Represents a reference to a value owned by a handle.
    struct SERPENT_API ValueReference final {
        private:
        ValueHandle handle;
        void *ptr;

        ValueReference(ValueHandle handle, void *ptr);
        
        public:
        std::optional<ValueReference> operator [] (std::string_view field) const;
        std::optional<ValueReference> operator [] (size_t index) const;
    };
}
