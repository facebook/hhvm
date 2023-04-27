/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "watchman/bser.h"
#include <benchmark/benchmark.h>
#include <fmt/core.h>
#include <random>
#include <vector>

namespace {

std::vector<char> predictable_bser_data() {
  constexpr size_t kRootSize = 10000;

  std::unordered_map<w_string, json_ref> fields;
  fields.emplace(w_string{"name"}, typed_string_to_json("filename"));
  fields.emplace(w_string{"size"}, json_integer(10000));
  fields.emplace(
      w_string{"clock"},
      typed_string_to_json("c:1661943594:3604891:5106627930189791234:22998"));

  json_ref entry = json_object(std::move(fields));

  std::vector<json_ref> values;
  values.reserve(kRootSize);
  for (size_t i = 0; i < kRootSize; ++i) {
    values.push_back(entry);
  }

  json_ref root = json_array(std::move(values));

  bser_ctx_t ctx;
  ctx.bser_version = 2;
  ctx.bser_capabilities = 0;
  std::vector<char> output;
  ctx.dump = [](const char* buffer, size_t size, void* opaque) -> int {
    auto& output = *static_cast<std::vector<char>*>(opaque);
    // TODO: quadratic
    output.insert(output.end(), buffer, buffer + size);
    return 0;
  };
  if (w_bser_dump(&ctx, root, &output)) {
    throw std::runtime_error("w_bser_dump failed");
  }

  fmt::print("generated {} bytes of predictable BSER data\n", output.size());
  return output;
}

w_string random_string(std::mt19937& mt) {
  static const unsigned kMaximumLength = 60;
  static const char kCharTable[62 + 1] =
      "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

  std::uniform_int_distribution<unsigned> string_length(0, kMaximumLength);
  std::uniform_int_distribution<unsigned> random_char(0, 61);
  uint32_t length = string_length(mt);

  char buffer[kMaximumLength];
  std::generate(buffer, buffer + kMaximumLength, [&] {
    return kCharTable[random_char(mt)];
  });
  return w_string(buffer, length);
}

json_ref random_value(std::mt19937& mt, unsigned depth);

/**
 * Returns x - y unless y > x, in which case it returns 0.
 */
inline unsigned clamped_sub(unsigned x, unsigned y) {
  return x < y ? 0 : (x - y);
}

json_ref random_array(std::mt19937& mt, unsigned depth) {
  std::uniform_int_distribution<unsigned> array_length(
      0, clamped_sub(10, depth));
  unsigned length = array_length(mt);
  std::vector<json_ref> elements;
  elements.reserve(length);
  for (unsigned i = 0; i < length; ++i) {
    elements.push_back(random_value(mt, depth + 1));
  }
  return json_array(std::move(elements));
}

json_ref random_object(std::mt19937& mt, unsigned depth) {
  std::uniform_int_distribution<unsigned> object_length(
      0, clamped_sub(10, depth));
  unsigned length = object_length(mt);

  std::unordered_map<w_string, json_ref> elements;
  for (unsigned i = 0; i < length; ++i) {
    elements.emplace(random_string(mt), random_value(mt, depth + 1));
  }
  return json_object(std::move(elements));
}

json_ref random_value(std::mt19937& mt, unsigned depth = 0) {
  std::uniform_int_distribution<unsigned> type(0, 15);
  std::uniform_int_distribution<unsigned> integer;
  std::uniform_real_distribution<double> real(-10'000'000.0, 10'000'000.0);
  switch (type(mt)) {
    case 0:
      return json_null();
    case 1:
      return json_false();
    case 2:
      return json_true();
    case 3:
    case 4:
    case 5:
      return json_integer(integer(mt));
    case 6:
      return json_real(real(mt));
    case 7:
    case 8:
    case 9:
      return w_string_to_json(random_string(mt));
    case 10:
    case 11:
    case 12:
      return random_array(mt, depth);
    case 13:
    case 14:
    case 15:
      return random_object(mt, depth);
  }
  abort();
}

std::vector<char> unpredictable_bser_data() {
  // kRootSize 1000 with the default Mersenne-Twister seed produces a BSER
  // document about one megabyte.
  const size_t kRootSize = 1000;
  std::mt19937 mt;

  std::vector<json_ref> values;
  values.reserve(kRootSize);
  for (size_t i = 0; i < kRootSize; ++i) {
    values.push_back(random_value(mt));
  }
  json_ref root = json_array(std::move(values));

  bser_ctx_t ctx;
  ctx.bser_version = 2;
  ctx.bser_capabilities = 0;
  std::vector<char> output;
  ctx.dump = [](const char* buffer, size_t size, void* opaque) -> int {
    auto& output = *static_cast<std::vector<char>*>(opaque);
    // TODO: quadratic
    output.insert(output.end(), buffer, buffer + size);
    return 0;
  };
  if (w_bser_dump(&ctx, root, &output)) {
    throw std::runtime_error("w_bser_dump failed");
  }

  fmt::print("generated {} bytes of unpredictable BSER data\n", output.size());
  return output;
}

static std::vector<json_ref> leaks;

template <std::vector<char> (*SynthesizeFn)()>
struct ParseBenchmark {
  static std::vector<char> data;

  static void run(benchmark::State& state) {
    // Comment this line out when profiling with `perf` to avoid seeing
    // deallocation costs.
    leaks.clear();

    for (auto _ : state) {
      leaks.push_back(bunser(data.data(), data.data() + data.size()));
    }
  }
};

template <std::vector<char> (*SynthesizeFn)()>
std::vector<char> ParseBenchmark<SynthesizeFn>::data = SynthesizeFn();

void bser_parse_predictable(benchmark::State& state) {
  ParseBenchmark<predictable_bser_data>::run(state);
}
BENCHMARK(bser_parse_predictable);

void bser_parse_unpredictable(benchmark::State& state) {
  ParseBenchmark<unpredictable_bser_data>::run(state);
}
BENCHMARK(bser_parse_unpredictable);

} // namespace

int main(int argc, char** argv) {
  ::benchmark::Initialize(&argc, argv);
  if (::benchmark::ReportUnrecognizedArguments(argc, argv))
    return 1;
  ::benchmark::RunSpecifiedBenchmarks();
}
