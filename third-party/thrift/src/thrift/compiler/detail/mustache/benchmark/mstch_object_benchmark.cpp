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
#include <thrift/compiler/detail/mustache/mstch.h>

using namespace apache::thrift::mstch;

class test_mstch_object : public internal::object_t<node> {
 public:
  void register_method(
      const std::string& name, const std::function<node()>& method) {
    internal::object_t<node>::register_volatile_method(name, method);
  }

  node method1() { return true; }

  node method2() { return std::string("test"); }

  node method3() { return 1; }

  static const std::vector<std::string> kNames;
};

const std::vector<std::string> test_mstch_object::kNames = {
    "method1", "method2", "method3"};

BENCHMARK(callAt, n) {
  test_mstch_object object;
  object.register_method(
      "method1", std::bind(&test_mstch_object::method1, &object));
  object.register_method(
      "method2", std::bind(&test_mstch_object::method2, &object));
  object.register_method(
      "method3", std::bind(&test_mstch_object::method3, &object));

  for (size_t i = 0; i < n; ++i) {
    object.at(test_mstch_object::kNames[i % 3]);
  }
}

BENCHMARK(callHas, n) {
  test_mstch_object object;
  object.register_method(
      "method1", std::bind(&test_mstch_object::method1, &object));
  object.register_method(
      "method2", std::bind(&test_mstch_object::method2, &object));
  object.register_method(
      "method3", std::bind(&test_mstch_object::method3, &object));

  for (size_t i = 0; i < n; ++i) {
    object.has(test_mstch_object::kNames[i % 3]);
  }
}

int main(int argc, char** argv) {
  folly::Init init(&argc, &argv);
  folly::runBenchmarks();
}
