#include <array>
#include <cassert>
#include <print>
#include <span>
#include <unordered_map>
#include "serpent/layout.hpp"
#include "serpent/types/interned_map.hpp"
#include "serpent/types/interner.hpp"
#include "serpent/types/rc_array.hpp"

const auto ColorLayout = Serpent::EnumLayout::Of({
    "Red",
    "Green",
    "Blue"
}).value();

const auto Vec3fLayout = Serpent::ObjectLayout::Of({
    {"x", Serpent::FloatingLayout::Float64},
    {"y", Serpent::FloatingLayout::Float64},
    {"z", Serpent::FloatingLayout::Float64},
}).value();

const auto U64Array = Serpent::ArrayLayout::Of(Serpent::IntegralLayout::UInt64);

const auto TestLayout = Serpent::ObjectLayout::Of({
    {"integral", Serpent::IntegralLayout::Int8}, // offset 0 size 1
    {"floating", Serpent::FloatingLayout::Float64}, // offset 8 size 8
    {"string", Serpent::PrimitiveLayout::String}, // offset 16 size 8
    {"unit", Serpent::PrimitiveLayout::Unit}, // offset 24 size 0
    {"array", U64Array}, // offsett 24 size 8
    {"i1", Serpent::IntegralLayout::Int8}, // offset 32 size 1
    {"i2", Serpent::IntegralLayout::Int8}, // offset 33 size 1
    {"i3", Serpent::IntegralLayout::Int16}, // offset 34 size 2
    {"i4", Serpent::IntegralLayout::Int32},  // offset 36 size 4
    {"position", Vec3fLayout} // offset 40 size 8
}).value(); // Size 48 align 8

void test(std::span<int const> span) {
    for (auto const &value : span) {
        std::println("{}", value);
    }
}

int main(int argc, char **argv) {
    std::unordered_map<Serpent::InternedString, int> stoi {};
    stoi.insert({Serpent::InternedString("key"), 1});
    stoi.insert({Serpent::InternedString("key2"), 2});
    stoi.insert({Serpent::InternedString("key3"), 3});

    Serpent::InternedMap<int> stoi2 = Serpent::InternedMap<int>::Create(stoi);

    if (auto p = stoi2.Get("key"))
        std::println("{}", *p);

    {
        std::array<int, 6> ca = {
            0, 1, 2, 3, 4, 5
        };
        Serpent::RcArray<int> ia = Serpent::RcArray<int>::Create(ca);

        test(ia);
    }

    return 0;
}
