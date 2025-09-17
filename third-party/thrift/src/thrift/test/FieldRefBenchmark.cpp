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

#include <glog/logging.h>
#include <folly/Benchmark.h>
#include <folly/Portability.h>
#include <folly/init/Init.h>
#include <thrift/test/gen-cpp2/optionals_types.h>

using namespace std;
using namespace cpp2;
using namespace folly;

BENCHMARK_COUNTERS(field_ref_direct, counters, n) {
  HasOptionals a;
  counters["stack_mem"] = sizeof(a);
  int64_t k = n * (1 + 'a');
  while (n--) {
    a.int64Opt() = 1;
    k -= a.int64Opt().value();
    a.stringOpt() = "a";
    k -= a.stringOpt().value()[0];
  }
  CHECK_EQ(k, 0);
  folly::doNotOptimizeAway(k);
}

template <class T, class U>
FOLLY_NOINLINE void run(T int64Opt_ref, U stringOpt_ref, int n) {
  int64_t k = n * (1 + 'a');
  while (n--) {
    int64Opt_ref = 1;
    k -= int64Opt_ref.value();
    stringOpt_ref = "a";
    k -= stringOpt_ref.value()[0];
  }
  CHECK_EQ(k, 0);
  folly::doNotOptimizeAway(k);
}

BENCHMARK_RELATIVE(field_ref_indirect, n) {
  HasOptionals a;
  run(a.int64Opt(), a.stringOpt(), n);
}

BENCHMARK_RELATIVE(unsafe, n) {
  HasOptionals a;
  apache::thrift::ensure_isset_unsafe(a.int64Opt());
  apache::thrift::ensure_isset_unsafe(a.stringOpt());
  int64_t k = n * (1 + 'a');
  while (n--) {
    a.int64Opt().value_unchecked() = 1;
    k -= a.int64Opt().value_unchecked();
    a.stringOpt().value_unchecked() = "a";
    k -= a.stringOpt().value_unchecked()[0];
  }
  CHECK_EQ(k, 0);
  folly::doNotOptimizeAway(k);
}

int main(int argc, char** argv) {
  const folly::Init init(&argc, &argv);
  runBenchmarks();
  return 0;
}
