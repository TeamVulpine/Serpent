#pragma once

#include <cstddef>
#include <functional>
#include <shared_mutex>
#include <string_view>
#include <unordered_map>

#include "api.hpp"
#include "freelist.hpp"

namespace Serpent {
    struct SERPENT_API Interner final {
        private:
        struct Value {
            char const *data;
            size_t size;
            size_t refCount = 1;

            Value(std::string_view view);
            ~Value();

            std::string_view View() const;
        };

        Freelist<Value> strings {};
        std::unordered_map<std::string_view, size_t> indices {};
        std::shared_mutex mutex {};

        Interner();

        public:
        Interner(Interner const &copy) = delete;
        Interner(Interner &&move) = delete;

        Interner &operator = (Interner const &copy) = delete;
        Interner &operator = (Interner &&move) = delete;

        static Interner &Instance();

        size_t Acquire(std::string_view view);
        size_t AddRef(size_t index);
        void RemoveRef(size_t index);
        std::string_view Get(size_t index);
    };

    struct InternedString final {
        private:
        size_t index;

        public:
        static InternedString const Empty;

        InternedString(std::string_view view);
        InternedString();

        InternedString(InternedString const &copy);
        InternedString(InternedString &&move) = default;

        ~InternedString();

        InternedString &operator = (InternedString const &copy);
        InternedString &operator = (InternedString &&move) = default;

        operator std::string_view () const;
        std::string_view Value() const;

        size_t Index() const;

        bool operator == (InternedString const &rhs) const;
        bool operator != (InternedString const &rhs) const;
        bool operator == (std::string_view const &rhs) const;
        bool operator != (std::string_view const &rhs) const;
    };
    bool operator == (std::string_view const &lhs, InternedString const &rhs);
    bool operator != (std::string_view const &lhs, InternedString const &rhs);
}

template <>
struct std::hash<Serpent::InternedString> final {
    size_t operator () (Serpent::InternedString const &s) const noexcept;
};
