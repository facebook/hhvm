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

#include <random>

#include <folly/Benchmark.h>
#include <folly/init/Init.h>
#include <thrift/lib/cpp2/op/Compare.h>
#include <thrift/lib/cpp2/op/Get.h>
#include <thrift/test/benchmarks/gen-cpp2/compare_types.h>

namespace apache::thrift::test {

std::mt19937 rng;
constexpr int N = 10;
MyStruct structs[N];
MyUnion unions[N];

void init() {
  for (int i = 0; i < N; ++i) {
    constexpr size_t size = op::size_v<MyStruct>;
    op::for_each_ordinal<MyStruct>([&]<class Ord>(Ord) {
      if (folly::to_underlying(Ord::value) + 1 == size) {
        // Make the last field different
        op::get<Ord>(structs[i]) = std::string(100, '0') + char(rng() % 10);
        op::get<Ord>(unions[i]) = std::string(100, '0') + char(rng() % 10);
      } else {
        op::get<Ord>(structs[i]) = std::string(100, '0');
      }
    });
  }

  // Sanity check
  for (int i = 1; i < N; i++) {
    CHECK_EQ(
        structs[i - 1] < structs[i],
        apache::thrift::op::detail::StructLessThan{}(
            structs[i - 1], structs[i]));
    CHECK_EQ(
        unions[i - 1] < unions[i],
        apache::thrift::op::detail::UnionLessThan{}(unions[i - 1], unions[i]));
  }
}

BENCHMARK(StructLess) {
  for (int i = 1; i < N; i++) {
    static_cast<void>(structs[i - 1] < structs[i]);
  }
}
BENCHMARK_RELATIVE(StructOpLess) {
  for (int i = 1; i < N; i++) {
    apache::thrift::op::detail::StructLessThan{}(structs[i - 1], structs[i]);
  }
}
BENCHMARK(UnionLess) {
  for (int i = 1; i < N; i++) {
    static_cast<void>(unions[i - 1] < unions[i]);
  }
}
BENCHMARK_RELATIVE(UnionOpLess) {
  for (int i = 1; i < N; i++) {
    apache::thrift::op::detail::UnionLessThan{}(unions[i - 1], unions[i]);
  }
}
} // namespace apache::thrift::test

int main(int argc, char** argv) {
  folly::Init init(&argc, &argv);
  apache::thrift::test::init();
  folly::runBenchmarks();
}
