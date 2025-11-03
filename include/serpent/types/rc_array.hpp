#pragma once

#include <algorithm>
#include <atomic>
#include <cassert>
#include <initializer_list>
#include <new>
#include <span>
#include "serpent/api.hpp"

namespace Serpent {
    /// A Rust-style reference counted array.
    /// Unlike std::shared_ptr, RcArray requires data to have a defined layout.
    /// RcArray also stores the ref-counting control block in the same allocation as T[], halving the size of the pointer on the stack.
    /// Reference counting is thread-safe, but access to the underlying data is not.
    /// Users are responsible for ensuring proper thread-safety of the underlying data.
    template <typename T>
    struct SERPENT_API RcArray final {
        private:
        struct SERPENT_API Inner final {
            std::atomic_size_t refCount = 1;
            T data[];

            Inner() = default;

            void AddRef() {
                refCount.fetch_add(1);
            }

            bool RemoveRef() {
                return refCount.fetch_sub(1) == 1;
            }
        };

        Inner *inner;
        size_t len;

        RcArray(Inner *inner, size_t size) :
            inner(inner),
            len(size)
        {}

        public:
        RcArray(RcArray const &copy) :
            RcArray(copy.inner, copy.len)
        {
            inner->AddRef();
        }

        RcArray(RcArray &&move) :
            RcArray(move.inner, move.len)
        {
            move.inner = nullptr;
        }

        ~RcArray() {
            if (!inner)
                return;

            if (inner->RemoveRef()) {
                T *dataMut = const_cast<T *>(inner->data);
                for (size_t i = 0; i < len; i++)
                    dataMut[i].~T();

                inner->~Inner();

                ::operator delete (reinterpret_cast<void *>(inner));
            }

            inner = nullptr;
            len = 0;
        }

        static RcArray Create(std::span<T> init) {
            size_t count = init.size();
            size_t align = std::max(alignof(Inner), alignof(T));
            size_t bytes = sizeof(Inner) + sizeof(T) * count;
            bytes = (bytes + align - 1) & ~(align - 1);
            Inner *inner = new (::operator new(bytes, std::align_val_t(align))) Inner;

            T *data = const_cast<T *>(inner->data);
            size_t i = 0;
            for (T const &val : init) {
                new (&data[i]) T(val);
                i++;
            }

            return RcArray(inner, count);
        }

        static RcArray Create(std::initializer_list<T> init) {
            size_t count = init.size();
            size_t align = std::max(alignof(Inner), alignof(T));
            size_t bytes = sizeof(Inner) + sizeof(T) * count;
            bytes = (bytes + align - 1) & ~(align - 1);
            Inner *inner = new (::operator new(bytes, std::align_val_t(align))) Inner;

            T *data = const_cast<T *>(inner->data);
            size_t i = 0;
            for (T const &val : init) {
                new (&data[i]) T(val);
                i++;
            }

            return RcArray(inner, count);
        }

        T const &operator [] (size_t index) const {
            assert(index < len);

            return inner->data[index];
        }

        bool operator == (RcArray const &other) const requires std::equality_comparable<T> {
            if (inner == other.inner)
                return true;
            
            if (len != other.len)
                return false;

            for (size_t i = 0; i < len; i++) {
                if (inner->data[i] != other.inner->data[i])
                    return false;
            }

            return true;
        }

        bool operator != (RcArray const &other) const requires std::equality_comparable<T> {
            return !(*this == other);
        }

        bool PointerEq(RcArray const &other) const {
            return inner == other.inner;
        }

        size_t size() const {
            return len;
        }

        T const *begin() const {
            return inner->data;
        }

        T const *end() const {
            return inner->data + len;
        }
    };
}
