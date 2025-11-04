#pragma once

#include <functional>
#include <string_view>
#include <unordered_map>
#include <vector>
#include "serpent/api.hpp"
#include "serpent/types/interner.hpp"
#include "serpent/types/rc_array.hpp"

namespace Serpent {
    /// An immutable map with an InternedString key
    template <typename TValue>
    struct SERPENT_API InternedMap {
        private:
        struct Node {
            InternedString key;
            TValue value;

            bool operator == (Node const &other) const = default;
            bool operator != (Node const &other) const = default;
        };

        RcArray<RcArray<Node>> buckets;

        InternedMap(
            RcArray<RcArray<Node>> buckets
        ) :
            buckets(buckets)
        {}

        static size_t Index(InternedString const &value, size_t bucketCount) {
            return std::hash<InternedString>()(value) & (bucketCount - 1);
        }

        public:
        bool operator == (InternedMap const &other) const = default;
        bool operator != (InternedMap const &other) const = default;

        static InternedMap Create(std::unordered_map<InternedString, TValue> const &from) {
            constexpr float Factor = 0.75f;
            size_t size = from.size();
            size_t bucketCount = 1;
            while (bucketCount < size / Factor)
                bucketCount <<= 1;

            std::vector<std::vector<Node>> buckets;
            buckets.reserve(bucketCount);
            for (size_t i = 0; i < bucketCount; i++)
                buckets.emplace_back();

            for (auto const &[key, value] : from)
                buckets[Index(key, bucketCount)].push_back(Node {key, value});

            std::vector<RcArray<Node>> first;
            first.reserve(bucketCount);
            for (size_t i = 0; i < bucketCount; i++)
                first.push_back(RcArray<Node>::Create(buckets[i]));

            RcArray<RcArray<Node>> nodes = RcArray<RcArray<Node>>::Create(first);

            return InternedMap(nodes);
        }

        TValue const *Get(InternedString const &key) const {
            for (auto const &node : buckets[Index(key, buckets.size())]) {
                if (node.key == key)
                    return &node.value;
            }
            return nullptr;
        }

        TValue const *Get(InternedString &&key) const {
            return Get(key);
        }

        TValue const *Get(std::string_view key) const {
            return Get(InternedString(key));
        }
    };
}
