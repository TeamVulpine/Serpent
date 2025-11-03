#pragma once

#include <cstddef>
#include <optional>
#include <vector>

#include "serpent/api.hpp"

namespace Serpent {
    template <typename T>
    struct SERPENT_API Freelist final {
        private:
        std::vector<std::optional<T>> values {};
        std::vector<size_t> free {};

        public:
        T *Get(size_t index) {
            if (index >= values.size())
                return nullptr;
            auto &value = values[index];

            if (!value)
                return nullptr;

            return &*value;
        }

        T const *Get(size_t index) const {
            if (index >= values.size())
                return nullptr;

            auto &value = values[index];

            if (!value)
                return nullptr;

            return &*value;
        }

        void Remove(size_t index) {
            if (index >= values.size())
                return;

            auto &value = values[index];

            if (!value)
                return;

            value = std::nullopt;

            free.push_back(index);
        }

        size_t Push(T &&value) {
            if (free.empty()) {
                auto index = values.size();

                values.push_back(std::move(value));

                return index;
            }

            auto index = free.back();
            free.pop_back();

            values[index] = std::move(value);

            return index;
        }

        template <typename ...TArgs>
        size_t Emplace(TArgs &&...args) {
            if (free.empty()) {
                auto index = values.size();

                values.emplace_back(std::forward(args)...);

                return index;
            }

            auto index = free.back();
            free.pop_back();

            values[index] = T(std::forward(args)...);

            return index;
        }
    };
}
