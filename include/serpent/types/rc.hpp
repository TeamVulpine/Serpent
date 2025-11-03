#pragma once

#include <atomic>
#include <concepts>

#include "serpent/api.hpp"

namespace Serpent {
    /// A Rust-style reference counted pointer.
    /// Unlike std::shared_ptr, Rc requires data to have a defined layout.
    /// Rc also stores the ref-counting control block in the same allocation as T, halving the size of the pointer on the stack.
    /// Reference counting is thread-safe, but access to the underlying data is not.
    /// Users are responsible for ensuring proper thread-safety of the underlying data.
    template <typename T>
    struct SERPENT_API Rc final {
        private:
        struct SERPENT_API Inner final {
            std::atomic_size_t refCount = 1;
            T const Data;

            Inner(T &&value) :
                Data(std::move(value))
            {}

            void AddRef() {
                refCount.fetch_add(1);
            }

            bool RemoveRef() {
                return refCount.fetch_sub(1) == 1;
            }
        };

        Inner *inner;

        Rc(Inner *inner) :
            inner(inner)
        {}

        public:
        Rc(Rc const &copy) :
            Rc(copy.inner)
        {
            inner->AddRef();
        }

        Rc(Rc &&move) :
            Rc(move.inner)
        {
            move.inner = nullptr;
        }

        ~Rc() {
            if (!inner)
                return;

            if (inner->RemoveRef())
                delete inner;

            inner = nullptr;
        }

        template<typename ...Args>
        static Rc Create(Args &&...args) {
            return Rc(new Inner(T(std::forward<Args>(args)...)));
        }

        Rc &operator = (Rc const &other) {
            if (this != &other) {
                if (inner && inner->RemoveRef())
                    delete inner;
                inner = other.inner;
                if (inner)
                    inner->AddRef();
            }

            return *this;
        }

        Rc &operator = (Rc &&other) noexcept {
            if (this != &other) {
                if (inner && inner->RemoveRef())
                    delete inner;
                inner = other.inner;
                other.inner = nullptr;
            }
            return *this;
        }

        bool operator == (Rc const &other) const requires std::equality_comparable<T> {
            if (inner == other.inner)
                return true;
            return inner->Data == other.inner->Data;
        }

        bool operator != (Rc const &other) const requires std::equality_comparable<T> {
            return !(*this == other);
        }

        bool PointerEq(Rc const &other) const {
            return inner == other.inner;
        }

        T const &operator * () const {
            return inner->Data;
        }

        T const *operator -> () const {
            return &inner->Data;
        }
    };
}
