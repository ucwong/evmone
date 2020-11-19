#pragma once

#include <evmc/evmc.hpp>

namespace evmone::test
{
constexpr auto gas_limit = std::numeric_limits<int64_t>::max();

extern evmc::VM vm;
}  // namespace evmone::test
