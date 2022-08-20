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
#include <folly/Demangle.h>
#include <folly/Traits.h>
#include <folly/init/Init.h>
#include <thrift/test/testset/gen-cpp2/testset_for_each_field.h>

namespace apache::thrift::test {
namespace {

constexpr int kIterationCount = 1'000'000;

template <class T>
using DetectValueUnchecked = decltype(T{}.field_1_ref().value_unchecked());

template <class T>
FOLLY_NOINLINE void indirect(T ref) {
  ref = "a";
  folly::doNotOptimizeAway(ref);
  folly::doNotOptimizeAway(ref.value());
}

template <class Struct>
void add_benchmark() {
  std::string name = folly::demangle(typeid(Struct).name()).toStdString();
  name = name.substr(name.rfind("::") + 2);

  folly::addBenchmark(__FILE__, name, [] {
    static Struct s;
    for (int i = 0; i < kIterationCount; i++) {
      s.field_1_ref() = "a";
      folly::doNotOptimizeAway(s.field_1_ref());
      folly::doNotOptimizeAway(s.field_1_ref().value());
    }
    return 1;
  });

  folly::addBenchmark(__FILE__, "%" + name + "_indirect", [] {
    static Struct s;
    for (int i = 0; i < kIterationCount; i++) {
      indirect(std::move(s.field_1_ref()));
    }
    return 1;
  });

  folly::addBenchmark(__FILE__, "%" + name + "_unsafe", [] {
    static Struct s;
    for (int i = 0; i < kIterationCount; i++) {
      if constexpr (folly::is_detected_v<DetectValueUnchecked, Struct>) {
        s.field_1_ref().value_unchecked() = "a";
        folly::doNotOptimizeAway(s.field_1_ref());
        folly::doNotOptimizeAway(s.field_1_ref().value_unchecked());
      } else if constexpr (apache::thrift::is_thrift_union_v<Struct>) {
        s.set_field_1("a");
        folly::doNotOptimizeAway(s.field_1_ref());
        folly::doNotOptimizeAway(s.get_field_1());
      } else {
        s.field_1_ref().value() = "a";
        folly::doNotOptimizeAway(s.field_1_ref());
        folly::doNotOptimizeAway(s.field_1_ref().value());
      }
    }
    return 1;
  });

  folly::addBenchmark(__FILE__, "%" + name + "_baseline", [] {
    static std::decay_t<decltype(*Struct{}.field_1_ref())> v;
    for (int i = 0; i < kIterationCount; i++) {
      v = "a";
      folly::doNotOptimizeAway(v);
    }
    return 1;
  });
}
} // namespace

void addFieldRefBenchmarks() {
  add_benchmark<testset::struct_string>();
  add_benchmark<testset::struct_optional_string>();
  add_benchmark<testset::struct_required_string>();
  add_benchmark<testset::union_string>();
}

} // namespace apache::thrift::test

int main(int argc, char** argv) {
  folly::init(&argc, &argv);
  apache::thrift::test::addFieldRefBenchmarks();
  folly::runBenchmarks();
  return 0;
}
