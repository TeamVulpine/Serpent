#include "serpent/value.hpp"
#include <atomic>
#include <cstddef>
#include "serpent/layout.hpp"
#include "serpent/types/rc.hpp"

namespace Serpent {
    struct GcHeader final {
        std::atomic_size_t refCount;

        void AddRef() {
            refCount.fetch_add(1);
        }
        /// Returns true if refCount reaches 0
        bool RemoveRef() {
            return refCount.fetch_sub(1) == 1;
        }
    };

    struct GcValue final {
        GcHeader header;
        Rc<GcLayout const> layout;

        void AddRef() {
            header.AddRef();
        }

        bool RemoveRef() {
            return header.RemoveRef();
        }
    };

    struct ArrayValue final {
        GcHeader header;
        ValueLayout layout;
        size_t size;

        void AddRef() {
            header.AddRef();
        }

        bool RemoveRef() {
            return header.RemoveRef();
        }
    };
}
