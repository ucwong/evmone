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
constexpr auto stack_limit = 1023;

enum class Mode
{
    min_stack,
    full_stack,
};

const bytes loop_prefix = push(255) + OP_JUMPDEST;
const bytes loop_suffix = push("ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff") +
                          OP_ADD + OP_DUP1 + push(2) + OP_JUMPI;

bytes generate_code(evmc_opcode opcode, Mode mode) noexcept
{
    if (!(opcode >= OP_PUSH1 && opcode <= OP_PUSH32))
        return {};

    bytes inner_code;
    switch (mode)
    {
    case Mode::min_stack:
    {
        bytes instr_pair;
        instr_pair.push_back(opcode);
        std::fill_n(std::back_inserter(instr_pair), opcode - OP_PUSH1 + 1, 0);
        instr_pair.push_back(OP_POP);
        for (int i = 0; i < stack_limit; ++i)
            inner_code += instr_pair;
        break;
    }
    case Mode::full_stack:
        for (int i = 0; i < stack_limit; ++i)
        {
            inner_code.push_back(opcode);
            if (opcode >= OP_PUSH1 && opcode <= OP_PUSH32)
                std::fill_n(std::back_inserter(inner_code), opcode - OP_PUSH1 + 1, 0);
        }
        std::fill_n(std::back_inserter(inner_code), stack_limit, OP_POP);
        break;
    }

    return loop_prefix + inner_code + loop_suffix;
}

bytes generate_loop()
{
    bytecode code =
        push(255) + OP_JUMPDEST + push(1) + OP_SWAP1 + OP_SUB + OP_DUP1 + push(2) + OP_JUMPI;

    return std::move(code);
}

bytes generate_loop2()
{
    bytecode code = push(255) + OP_JUMPDEST +
                    push("ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff") +
                    OP_ADD + OP_DUP1 + push(2) + OP_JUMPI;

    return std::move(code);
}
}  // namespace

bool register_synthetic_benchmarks() noexcept
{
    RegisterBenchmark("analyse/synth/push1_interleaved", [](State& state) {
        const auto code = generate_code(OP_PUSH1, Mode::min_stack);
        analyse(state, code);
    })->Unit(kMicrosecond);
    RegisterBenchmark("analyse/synth/push1_full_stack", [](State& state) {
        const auto code = generate_code(OP_PUSH1, Mode::full_stack);
        analyse(state, code);
    })->Unit(kMicrosecond);
    RegisterBenchmark("analyse/synth/push31_interleaved", [](State& state) {
        const auto code = generate_code(OP_PUSH31, Mode::min_stack);
        analyse(state, code);
    })->Unit(kMicrosecond);
    RegisterBenchmark("analyse/synth/push31_full_stack", [](State& state) {
        const auto code = generate_code(OP_PUSH31, Mode::full_stack);
        analyse(state, code);
    })->Unit(kMicrosecond);
    RegisterBenchmark("analyse/synth/push32_interleaved", [](State& state) {
        const auto code = generate_code(OP_PUSH32, Mode::min_stack);
        analyse(state, code);
    })->Unit(kMicrosecond);
    RegisterBenchmark("analyse/synth/push32_full_stack", [](State& state) {
        const auto code = generate_code(OP_PUSH32, Mode::full_stack);
        analyse(state, code);
    })->Unit(kMicrosecond);


    RegisterBenchmark("analyse/synth/loop", [](State& state) {
        analyse(state, generate_loop());
    })->Unit(kMicrosecond);
    RegisterBenchmark("execute/synth/loop", [](State& state) {
        execute(state, generate_loop());
    })->Unit(kMicrosecond);


    RegisterBenchmark("analyse/synth/loop2", [](State& state) {
        analyse(state, generate_loop2());
    })->Unit(kMicrosecond);
    RegisterBenchmark("execute/synth/loop2", [](State& state) {
        execute(state, generate_loop2());
    })->Unit(kMicrosecond);

    RegisterBenchmark("execute/synth/push1_interleaved", [](State& state) {
        execute(state, generate_code(OP_PUSH1, Mode::min_stack), {});
    })->Unit(kMicrosecond);
    RegisterBenchmark("execute/synth/push1_full_stack", [](State& state) {
        execute(state, generate_code(OP_PUSH1, Mode::full_stack), {});
    })->Unit(kMicrosecond);
    RegisterBenchmark("execute/synth/push31_interleaved", [](State& state) {
        execute(state, generate_code(OP_PUSH31, Mode::min_stack), {});
    })->Unit(kMicrosecond);
    RegisterBenchmark("execute/synth/push31_full_stack", [](State& state) {
        execute(state, generate_code(OP_PUSH31, Mode::full_stack), {});
    })->Unit(kMicrosecond);
    RegisterBenchmark("execute/synth/push32_interleaved", [](State& state) {
        execute(state, generate_code(OP_PUSH32, Mode::min_stack), {});
    })->Unit(kMicrosecond);
    RegisterBenchmark("execute/synth/push32_full_stack", [](State& state) {
        execute(state, generate_code(OP_PUSH32, Mode::full_stack), {});
    })->Unit(kMicrosecond);

    return true;
}

[[maybe_unsed]] static const auto synthetic_benchmarks_registered = register_synthetic_benchmarks();
}  // namespace evmone::test
