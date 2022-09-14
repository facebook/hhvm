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

} // namespace

int main(int argc, char** argv) {
  ::benchmark::Initialize(&argc, argv);
  if (::benchmark::ReportUnrecognizedArguments(argc, argv))
    return 1;
  ::benchmark::RunSpecifiedBenchmarks();
}
