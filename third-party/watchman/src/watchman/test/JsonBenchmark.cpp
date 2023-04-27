/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <benchmark/benchmark.h>
#include "watchman/thirdparty/jansson/jansson.h"

namespace {

void encode_doubles(benchmark::State& state) {
  // 3.7 ^ 500 still fits in a double.
  constexpr size_t N = 500;
  // Produce a variety of doubles.
  constexpr double B = 3.7;

  std::vector<json_ref> arr;
  arr.reserve(N);

  double value = 1;
  for (size_t i = 0; i < N; ++i) {
    arr.push_back(json_real(value));
    value *= B;
  }

  json_ref array = json_array(std::move(arr));

  for (auto _ : state) {
    benchmark::DoNotOptimize(json_dumps(array, JSON_COMPACT));
  }
}
BENCHMARK(encode_doubles);

void encode_zero_point_zero(benchmark::State& state) {
  constexpr size_t N = 500;

  std::vector<json_ref> arr;
  arr.reserve(N);
  for (size_t i = 0; i < N; ++i) {
    arr.push_back(json_real(0));
  }

  json_ref array = json_array(std::move(arr));

  for (auto _ : state) {
    benchmark::DoNotOptimize(json_dumps(array, JSON_COMPACT));
  }
}
BENCHMARK(encode_zero_point_zero);

void decode_doubles(benchmark::State& state) {
  // 3.7 ^ 500 still fits in a double.
  constexpr size_t N = 500;
  // Produce a variety of doubles.
  constexpr double B = 3.7;

  std::vector<json_ref> arr;
  arr.reserve(N);
  double value = 1;
  for (size_t i = 0; i < N; ++i) {
    arr.push_back(json_real(value));
    value *= B;
  }

  json_ref array = json_array(std::move(arr));
  auto encoded = json_dumps(array, JSON_COMPACT);

  for (auto _ : state) {
    json_error_t err;
    benchmark::DoNotOptimize(json_loads(encoded.c_str(), 0, &err));
  }
}

BENCHMARK(decode_doubles);

} // namespace

BENCHMARK_MAIN();
