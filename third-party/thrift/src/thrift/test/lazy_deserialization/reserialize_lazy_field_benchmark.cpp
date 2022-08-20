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

#include <folly/Benchmark.h>
#include <folly/init/Init.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/test/lazy_deserialization/gen-cpp2/simple_types.h>

namespace apache::thrift::test {

LazyFoo gen() {
  LazyFoo foo;
  foo.field4_ref().emplace(10'000, 1'000);
  return foo;
}

BENCHMARK(reserialize_with_undeserialized_field) {
  LazyFoo foo;
  BENCHMARK_SUSPEND {
    CompactSerializer::deserialize(
        CompactSerializer::serialize<std::string>(gen()), foo);
  }

  CompactSerializer::serialize<std::string>(foo);
}

BENCHMARK_RELATIVE(reserialize_without_undeserialized_field) {
  CompactSerializer::serialize<std::string>(gen());
}

} // namespace apache::thrift::test

int main(int argc, char** argv) {
  folly::init(&argc, &argv);
  folly::runBenchmarks();
}
