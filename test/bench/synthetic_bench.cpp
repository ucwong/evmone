// evmone: Fast Ethereum Virtual Machine implementation
// Copyright 2019-2020 The evmone Authors.
// SPDX-License-Identifier: Apache-2.0

#include "helpers.hpp"
#include "test/utils/bytecode.hpp"
#include <benchmark/benchmark.h>
#include <evmc/instructions.h>
#include <evmone/analysis.hpp>

namespace evmone::test
{
using namespace benchmark;

namespace
{
constexpr auto code_size_limit = 40 * 1024;
constexpr auto stack_limit = 1024;

enum class Mode
{
    interleaved,
    full_stack,
};

bytes generate_code(evmc_opcode opcode, Mode mode) noexcept
{
    bytes pattern;
    switch (mode)
    {
    case Mode::interleaved:
        pattern.push_back(opcode);
        if (opcode >= OP_PUSH1 && opcode <= OP_PUSH32)
            std::fill_n(std::back_inserter(pattern), opcode - OP_PUSH1 + 1, 0);
        pattern.push_back(OP_POP);
        break;
    case Mode::full_stack:
        for (int i = 0; i < stack_limit; ++i)
        {
            pattern.push_back(opcode);
            if (opcode >= OP_PUSH1 && opcode <= OP_PUSH32)
                std::fill_n(std::back_inserter(pattern), opcode - OP_PUSH1 + 1, 0);
        }
        std::fill_n(std::back_inserter(pattern), stack_limit, OP_POP);
        break;
    }

    auto num_patterns = code_size_limit / pattern.size();
    bytes code;
    code.reserve(num_patterns * pattern.size());
    while (num_patterns-- != 0)
        code += pattern;

    return code;
}

[[maybe_unused]] bytes generate_loop()
{
    bytecode code =
        push(100) + OP_JUMPDEST + push(1) + OP_SWAP1 + OP_SUB + OP_DUP1 + push(2) + OP_JUMPI;

    return std::move(code);
}
}  // namespace

bool register_synthetic_benchmarks() noexcept
{
    RegisterBenchmark("analyse/synth/push1_interleaved", [](State& state) {
        const auto code = generate_code(OP_PUSH1, Mode::interleaved);
        analyse(state, code);
    })->Unit(kMicrosecond);
    RegisterBenchmark("analyse/synth/push1_full_stack", [](State& state) {
        const auto code = generate_code(OP_PUSH1, Mode::full_stack);
        analyse(state, code);
    })->Unit(kMicrosecond);
    RegisterBenchmark("analyse/synth/push31_interleaved", [](State& state) {
        const auto code = generate_code(OP_PUSH31, Mode::interleaved);
        analyse(state, code);
    })->Unit(kMicrosecond);
    RegisterBenchmark("analyse/synth/push31_full_stack", [](State& state) {
        const auto code = generate_code(OP_PUSH31, Mode::full_stack);
        analyse(state, code);
    })->Unit(kMicrosecond);
    RegisterBenchmark("analyse/synth/push32_interleaved", [](State& state) {
        const auto code = generate_code(OP_PUSH32, Mode::interleaved);
        analyse(state, code);
    })->Unit(kMicrosecond);
    RegisterBenchmark("analyse/synth/push32_full_stack", [](State& state) {
        const auto code = generate_code(OP_PUSH32, Mode::full_stack);
        analyse(state, code);
    })->Unit(kMicrosecond);


    RegisterBenchmark("execute/synth/loop", [](State& state) {
        execute(state, generate_code(OP_PUSH1, Mode::interleaved), {});
    })->Unit(kMicrosecond);

    RegisterBenchmark("execute/synth/push1_interleaved", [](State& state) {
        execute(state, generate_code(OP_PUSH1, Mode::interleaved), {});
    })->Unit(kMicrosecond);
    RegisterBenchmark("execute/synth/push1_full_stack", [](State& state) {
        execute(state, generate_code(OP_PUSH1, Mode::full_stack), {});
    })->Unit(kMicrosecond);
    RegisterBenchmark("execute/synth/push31_interleaved", [](State& state) {
        execute(state, generate_code(OP_PUSH31, Mode::interleaved), {});
    })->Unit(kMicrosecond);
    RegisterBenchmark("execute/synth/push31_full_stack", [](State& state) {
        execute(state, generate_code(OP_PUSH31, Mode::full_stack), {});
    })->Unit(kMicrosecond);
    RegisterBenchmark("execute/synth/push32_interleaved", [](State& state) {
        execute(state, generate_code(OP_PUSH32, Mode::interleaved), {});
    })->Unit(kMicrosecond);
    RegisterBenchmark("execute/synth/push32_full_stack", [](State& state) {
        execute(state, generate_code(OP_PUSH32, Mode::full_stack), {});
    })->Unit(kMicrosecond);

    return true;
}

[[maybe_unsed]] static const auto synthetic_benchmarks_registered = register_synthetic_benchmarks();
}  // namespace evmone::test
