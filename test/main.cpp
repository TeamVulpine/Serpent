#include <new>
#include <print>
#include "serpent/layout.hpp"

const Serpent::ValueLayout TestLayout = Serpent::ObjectLayout::Of({
    {"integral", Serpent::IntegralLayout::Int8},
    {"floating", Serpent::FloatingLayout::Float64},
    {"string", Serpent::PrimitiveLayout::String},
    {"unit", Serpent::PrimitiveLayout::Unit},
    {"array", Serpent::ArrayLayout::Of(Serpent::IntegralLayout::UInt64)},
    {"i1", Serpent::IntegralLayout::Int8},
    {"i2", Serpent::IntegralLayout::Int8},
    {"i3", Serpent::IntegralLayout::Int16},
    {"i4", Serpent::IntegralLayout::Int32},
}).value();

int main(int argc, char **argv) {
    void *data = ::operator new(Serpent::GetSize(TestLayout), std::align_val_t(Serpent::GetAlign(TestLayout)));
    std::println("{}", data);

    Serpent::DefaultInitialize(TestLayout, data);

    ::operator delete(data);

    return 0;
}
