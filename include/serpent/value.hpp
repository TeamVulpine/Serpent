#pragma once

#include <atomic>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include "serpent/layout.hpp"

namespace Serpent {
    struct SERPENT_API GcValue final {
        private:
        std::shared_ptr<GcLayout const> layout;
        std::atomic_size_t refCount = 1;
        /// MR1W lock used for accessing underlying data
        std::shared_mutex dataMutex {};
        
        void *operator new(std::size_t) = delete;
        void  operator delete(void *) = delete;

        void *operator new[](std::size_t) = delete;
        void  operator delete[](void *) = delete;
        
        public:
        GcValue(std::shared_ptr<GcLayout const> layout);

        static inline GcValue *New(size_t size, std::shared_ptr<GcLayout const> layout) {
            GcValue *value = reinterpret_cast<GcValue *>(malloc(sizeof(GcValue) + size));

            new (&value->layout) std::shared_ptr(layout);
            new (&value->refCount) std::atomic_size_t(0);
            new (&value->dataMutex) std::shared_mutex;

            return value;
        }

        inline std::shared_lock<std::shared_mutex> Read() {
            return std::shared_lock(dataMutex);
        }

        inline std::unique_lock<std::shared_mutex> Write() {
            return std::unique_lock(dataMutex);
        }

        inline void *Data() const {
            return reinterpret_cast<void *>(reinterpret_cast<size_t>(this) + sizeof(GcValue));
        }
    }; 
}
