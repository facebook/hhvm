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

#include <string>

#include <folly/Benchmark.h>
#include <folly/Traits.h>
#include <folly/init/Init.h>
#include <folly/lang/Pretty.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/test/benchmarks/gen-cpp2/terse_write_benchmark_types.h>

namespace apache::thrift::test {
namespace {

constexpr int kIterationCount = 1'000'000;

template <typename T>
void fill(T& obj) {
  op::for_each_field_id<T>([&obj](auto id) { op::get<>(id, obj) = 1; });
}
template <typename T>
void fill_half(T& obj) {
  bool pass = false;
  op::for_each_field_id<T>([&obj, &pass](auto id) {
    if (pass = !pass, pass) {
      op::get<>(id, obj) = 1;
    }
  });
}
template <typename T>
void fill_first(T& obj) {
  op::get<>(field_id<1>{}, obj) = 1;
}
template <typename T>
void fill_middle(T& obj) {
  op::get<>(field_id<op::num_fields<T> / 2 + 1>{}, obj) = 1;
}
template <typename T>
void fill_last(T& obj) {
  op::get<>(field_id<op::num_fields<T>>{}, obj) = 1;
}

template <class Struct>
void add_benchmark(bool empty = false) {
  std::string name = folly::pretty_name<Struct>();
  name = name.substr(name.rfind("::") + 2) + (empty ? "_empty" : "");

  folly::addBenchmark(__FILE__, name, [&] {
    static Struct obj;
    for (int i = 0; i < kIterationCount; i++) {
      auto s = CompactSerializer::serialize<std::string>(obj);
      folly::doNotOptimizeAway(s);
    }
    return 1;
  });
}

template <class Struct>
void add_filled_benchmark() {
  std::string name = folly::pretty_name<Struct>();
  name = name.substr(name.rfind("::") + 2) + "_filled";

  folly::addBenchmark(__FILE__, name, [&] {
    static Struct obj;
    fill(*obj.interal());
    for (int i = 0; i < kIterationCount; i++) {
      auto s = CompactSerializer::serialize<std::string>(obj);
      folly::doNotOptimizeAway(s);
    }
    return 1;
  });
}

template <class Struct>
void add_first_element_nonempty_benchmark() {
  std::string name = folly::pretty_name<Struct>();
  name = name.substr(name.rfind("::") + 2) + "_first_element_nonempty";

  folly::addBenchmark(__FILE__, name, [&] {
    static Struct obj;
    fill_first(*obj.interal());
    for (int i = 0; i < kIterationCount; i++) {
      auto s = CompactSerializer::serialize<std::string>(obj);
      folly::doNotOptimizeAway(s);
    }
    return 1;
  });
}

template <class Struct>
void add_filled_half_benchmark() {
  std::string name = folly::pretty_name<Struct>();
  name = name.substr(name.rfind("::") + 2) + "_half_filled";

  folly::addBenchmark(__FILE__, name, [&] {
    static Struct obj;
    fill_half(*obj.interal());
    for (int i = 0; i < kIterationCount; i++) {
      auto s = CompactSerializer::serialize<std::string>(obj);
      folly::doNotOptimizeAway(s);
    }
    return 1;
  });
}

template <class Struct>
void add_middle_element_nonempty_benchmark() {
  std::string name = folly::pretty_name<Struct>();
  name = name.substr(name.rfind("::") + 2) + "_middle_element_nonempty";
  folly::addBenchmark(__FILE__, name, [&] {
    static Struct obj;
    fill_middle(*obj.interal());
    for (int i = 0; i < kIterationCount; i++) {
      auto s = CompactSerializer::serialize<std::string>(obj);
      folly::doNotOptimizeAway(s);
    }
    return 1;
  });
}

template <class Struct>
void add_last_element_nonempty_benchmark() {
  std::string name = folly::pretty_name<Struct>();
  name = name.substr(name.rfind("::") + 2) + "_last_element_nonempty";
  folly::addBenchmark(__FILE__, name, [&] {
    static Struct obj;
    fill_last(*obj.interal());
    for (int i = 0; i < kIterationCount; i++) {
      auto s = CompactSerializer::serialize<std::string>(obj);
      folly::doNotOptimizeAway(s);
    }
    return 1;
  });
}
} // namespace

template <typename T>
void addTerseWriteBenchmarks() {
  add_benchmark<T>(true);
  add_filled_half_benchmark<T>();
  add_filled_benchmark<T>();
  add_first_element_nonempty_benchmark<T>();
  add_middle_element_nonempty_benchmark<T>();
  add_last_element_nonempty_benchmark<T>();
}

void addBenchmarks() {
  add_benchmark<struct_8>();
  addTerseWriteBenchmarks<terse_struct_8>();
  add_benchmark<struct_16>();
  addTerseWriteBenchmarks<terse_struct_16>();
  add_benchmark<struct_32>();
  addTerseWriteBenchmarks<terse_struct_32>();
  add_benchmark<struct_64>();
  addTerseWriteBenchmarks<terse_struct_64>();
  add_benchmark<struct_128>();
  addTerseWriteBenchmarks<terse_struct_128>();
  add_benchmark<struct_256>();
  addTerseWriteBenchmarks<terse_struct_256>();
  add_benchmark<struct_512>();
  addTerseWriteBenchmarks<terse_struct_512>();
}

} // namespace apache::thrift::test

int main(int argc, char** argv) {
  const folly::Init init(&argc, &argv);
  apache::thrift::test::addBenchmarks();
  folly::runBenchmarks();
  return 0;
}
