/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

#include <glog/logging.h>
#include <folly/Benchmark.h>
#include <folly/Portability.h>
#include <folly/init/Init.h>
#include <thrift/test/gen-cpp2/UnionFieldRef_types.h>

using namespace std;
using namespace folly;
using namespace apache::thrift::test;

BENCHMARK(field_ref_direct, n) {
  Basic a;
  int64_t k = n * (1 + 'a');
  while (n--) {
    a.int64_ref() = 1;
    k -= a.int64_ref().value();
    a.str_ref() = "a";
    k -= a.str_ref().value()[0];
  }
  CHECK_EQ(k, 0);
  folly::doNotOptimizeAway(k);
}

template <class T, class U>
FOLLY_NOINLINE void run(T int64_ref, U str_ref, int n) {
  int64_t k = n * (1 + 'a');
  while (n--) {
    int64_ref = 1;
    k -= int64_ref.value();
    str_ref = "a";
    k -= str_ref.value()[0];
  }
  CHECK_EQ(k, 0);
  folly::doNotOptimizeAway(k);
}

BENCHMARK_RELATIVE(field_ref_indirect, n) {
  Basic a;
  run(a.int64_ref(), a.str_ref(), n);
}

BENCHMARK_RELATIVE(unsafe, n) {
  Basic a;
  int64_t k = n * (1 + 'a');
  while (n--) {
    a.set_int64(1);
    k -= a.get_int64();
    a.set_str("a");
    k -= a.get_str()[0];
  }
  CHECK_EQ(k, 0);
  folly::doNotOptimizeAway(k);
}

int main(int argc, char** argv) {
  folly::init(&argc, &argv);
  runBenchmarks();
  return 0;
}
