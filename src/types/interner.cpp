#include <algorithm>
#include <functional>
#include <mutex>
#include <shared_mutex>
#include <string_view>

#include "serpent/types/interner.hpp"

Serpent::Interner::Interner() {}

Serpent::Interner &Serpent::Interner::Instance() {
    static Interner value {};

    return value;
}

Serpent::Interner::Value::Value(std::string_view view) {
    if (view.empty()) {
        data = nullptr;
        size = 0;
        return;
    }

    auto tmpData = new char[view.length()];
    std::copy(view.begin(), view.end(), tmpData);

    data = tmpData;
    size = view.size();
}

void Serpent::Interner::Value::Cleanup() {
    if (data)
        delete[] data;
    data = 0;
}

std::string_view Serpent::Interner::Value::View() const {
    return {data, size};
}

size_t Serpent::Interner::Acquire(std::string_view view) {
    if (view.empty())
        return 0;

    std::unique_lock<std::shared_mutex> lock {mutex};

    if (indices.contains(view)) {
        lock.unlock();
        return AddRef(indices[view]);
    }

    size_t idx = strings.Push(view);
    auto &value = *strings.Get(idx);
    indices[value.View()] = idx;

    return idx + 1;
}

size_t Serpent::Interner::AddRef(size_t index) {
    if (index == 0)
        return 0;

    std::unique_lock<std::shared_mutex> lock {mutex};

    auto maybeValue = strings.Get(index - 1);

    if (maybeValue)
        maybeValue->refCount += 1;

    return index;
}

void Serpent::Interner::RemoveRef(size_t index) {
    if (index == 0)
        return;

    std::unique_lock<std::shared_mutex> lock {mutex};

    auto maybeValue = strings.Get(index - 1);

    if (!maybeValue)
        return;

    maybeValue->refCount -= 1;

    if (maybeValue->refCount == 0) {
        indices.erase(maybeValue->View());
        maybeValue->Cleanup();
        strings.Remove(index - 1);
    }
}

std::string_view Serpent::Interner::Get(size_t index) {
    if (index == 0)
        return "";

    std::shared_lock<std::shared_mutex> lock {mutex};

    auto maybeValue = strings.Get(index - 1);

    if (!maybeValue)
        return {};
    
    return maybeValue->View();
}

Serpent::InternedString const Serpent::InternedString::Empty {};

Serpent::InternedString::InternedString(std::string_view view) :
    index(Interner::Instance().Acquire(view))
{}

Serpent::InternedString::InternedString() :
    index(Interner::Instance().Acquire(""))
{}

Serpent::InternedString::InternedString(Serpent::InternedString const &copy) :
    index(Interner::Instance().AddRef(copy.index))
{}

Serpent::InternedString::InternedString(Serpent::InternedString &&move) :
    index(move.index)
{
    move.index = 0;
}

Serpent::InternedString::~InternedString() {
    Interner::Instance().RemoveRef(index);
    index = 0;
}

Serpent::InternedString &Serpent::InternedString::operator = (Serpent::InternedString const &copy) {
    if (index != copy.index) {
        auto &interner = Interner::Instance();
        interner.RemoveRef(this->index);

        this->index = interner.AddRef(copy.index);
    }

    return *this;
}

Serpent::InternedString &Serpent::InternedString::operator = (Serpent::InternedString &&move) {
    if (index != move.index) {
        auto &interner = Interner::Instance();
        interner.RemoveRef(this->index);

        this->index = move.index;
        move.index = 0;
    }

    return *this;
}

Serpent::InternedString::operator std::string_view () const {
    return Interner::Instance().Get(index);
}

std::string_view Serpent::InternedString::Value() const {
    return Interner::Instance().Get(index);
}

size_t Serpent::InternedString::Index() const {
    return index;
}

bool Serpent::InternedString::operator == (InternedString const &rhs) const {
    return index == rhs.index;
}

bool Serpent::InternedString::operator != (InternedString const &rhs) const {
    return index != rhs.index;
}

bool Serpent::InternedString::operator == (std::string_view const &rhs) const {
    return Value() == rhs;
}

bool Serpent::InternedString::operator != (std::string_view const &rhs) const {
    return Value() != rhs;
}

bool operator == (std::string_view const &lhs, Serpent::InternedString const &rhs) {
    return rhs == lhs;
}

bool operator != (std::string_view const &lhs, Serpent::InternedString const &rhs) {
    return rhs != lhs;
}

size_t std::hash<Serpent::InternedString>::operator () (Serpent::InternedString const &s) const noexcept {
    return std::hash<size_t>()(s.Index());
}
