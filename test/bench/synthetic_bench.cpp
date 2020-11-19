#include "vm.hpp"
#include <benchmark/benchmark.h>
#include <evmc/instructions.h>
#include <evmone/analysis.hpp>

namespace evmone::test
{
using namespace benchmark;

namespace
{
constexpr auto code_size_limit = 25 * 1024;
// constexpr auto stack_limit = 1024;

enum class Mode
{
    interleaved,
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
    }

    auto num_patterns = code_size_limit / pattern.size();
    bytes code;
    code.reserve(num_patterns * pattern.size());
    while (num_patterns-- != 0)
        code += pattern;

    return code;
}

void analyse(State& state, bytes_view code) noexcept
{
    auto bytes_analysed = uint64_t{0};
    for (auto _ : state)
    {
        auto r = evmone::analyze(EVMC_ISTANBUL, code.data(), code.size());
        DoNotOptimize(r);
        bytes_analysed += code.size();
    }
    state.counters["size"] = Counter(static_cast<double>(code.size()));
    state.counters["rate"] = Counter(static_cast<double>(bytes_analysed), Counter::kIsRate);
}

void execute(State& state, evmc_opcode opcode, Mode mode) noexcept
{
    const auto code = generate_code(opcode, mode);

    evmc_message msg{};
    msg.gas = gas_limit;

    auto total_gas_used = int64_t{0};
    auto iteration_gas_used = int64_t{0};
    for (auto _ : state)
    {
        const auto r = vm.execute(EVMC_ISTANBUL, msg, code.data(), code.size());
        iteration_gas_used = gas_limit - r.gas_left;
        total_gas_used += iteration_gas_used;
    }
    state.counters["gas_used"] = Counter(static_cast<double>(iteration_gas_used));
    state.counters["gas_rate"] = Counter(static_cast<double>(total_gas_used), Counter::kIsRate);
}
}  // namespace

void register_synthetic_benchmarks()
{
    RegisterBenchmark("execute/synth/push1_interleaved", [](State& state) {
      execute(state, OP_PUSH1, Mode::interleaved);
    })->Unit(kMicrosecond);
    RegisterBenchmark("execute/synth/push31_interleaved", [](State& state) {
      execute(state, OP_PUSH31, Mode::interleaved);
    })->Unit(kMicrosecond);
    RegisterBenchmark("execute/synth/push32_interleaved", [](State& state) {
      execute(state, OP_PUSH32, Mode::interleaved);
    })->Unit(kMicrosecond);

    RegisterBenchmark("analyse/synth/push1_interleaved", [](State& state) {
        const auto code = generate_code(OP_PUSH1, Mode::interleaved);
        analyse(state, code);
    })->Unit(kMicrosecond);

    RegisterBenchmark("analyse/synth/push31_interleaved", [](State& state) {
      const auto code = generate_code(OP_PUSH31, Mode::interleaved);
      analyse(state, code);
    })->Unit(kMicrosecond);

    RegisterBenchmark("analyse/synth/push32_interleaved", [](State& state) {
      const auto code = generate_code(OP_PUSH32, Mode::interleaved);
      analyse(state, code);
    })->Unit(kMicrosecond);
}
}  // namespace evmone::test
