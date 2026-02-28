/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <folly/Benchmark.h>
#include <folly/init/Init.h>
#include <thrift/lib/cpp2/gen/module_types_h.h>

using apache::thrift::detail::isset_bitset;
using apache::thrift::detail::IssetBitsetOption;

template <IssetBitsetOption kOption, class N>
void set(N& n) {
  isset_bitset<16, kOption> b;
  while (n--) {
    b.set(n % 16, true);
    b.set(15 - n % 16, false);
    folly::doNotOptimizeAway(b.at(0));
    folly::doNotOptimizeAway(b.at(1));
  }
}

template <IssetBitsetOption kOption, class N>
void get(N& n) {
  isset_bitset<16, kOption> b;
  for (int i = 0; i < 16; i++) {
    b.set(i, i % 3 == 0);
  }
  while (n--) {
    folly::doNotOptimizeAway(b.get(n % 16));
  }
}

BENCHMARK(unpacked_set, n) {
  set<IssetBitsetOption::Unpacked>(n);
}

BENCHMARK(unpacked_get, n) {
  get<IssetBitsetOption::Unpacked>(n);
}

BENCHMARK(non_atomic_set, n) {
  set<IssetBitsetOption::Packed>(n);
}

BENCHMARK(non_atomic_get, n) {
  get<IssetBitsetOption::Packed>(n);
}

BENCHMARK(atomic_set, n) {
  set<IssetBitsetOption::PackedWithAtomic>(n);
}

BENCHMARK(atomic_get, n) {
  get<IssetBitsetOption::PackedWithAtomic>(n);
}

int main(int argc, char** argv) {
  const folly::Init init(&argc, &argv);
  folly::runBenchmarks();
}
