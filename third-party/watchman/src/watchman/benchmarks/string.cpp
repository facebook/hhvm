/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <benchmark/benchmark.h>
#include "watchman/watchman_string.h"

namespace {

void string_allocate_and_deallocate(benchmark::State& state) {
  char c[] = "hello world, how are you?";
  for (auto _ : state) {
    benchmark::DoNotOptimize(w_string{c, sizeof(c) - 1});
  }
}

BENCHMARK(string_allocate_and_deallocate);

void string_hash(benchmark::State& state) {
  char c[] = "if there are no branches in the hash function, constant is fine";
  w_string str = c;
  for (auto _ : state) {
    benchmark::DoNotOptimize(str.hashValue());
  }
}

BENCHMARK(string_hash);

void string_piece_hash(benchmark::State& state) {
  char c[] = "if there are no branches in the hash function, constant is fine";
  w_string_piece piece = c;
  for (auto _ : state) {
    benchmark::DoNotOptimize(piece.hashValue());
  }
}

BENCHMARK(string_piece_hash);

} // namespace

int main(int argc, char** argv) {
  ::benchmark::Initialize(&argc, argv);
  if (::benchmark::ReportUnrecognizedArguments(argc, argv))
    return 1;
  ::benchmark::RunSpecifiedBenchmarks();
}
