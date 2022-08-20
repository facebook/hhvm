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

template <class Struct>
folly::IOBuf genListDouble() {
  Struct foo;
  foo.field3_ref().emplace(10'000, 1'000);
  return CompactSerializer::serialize<folly::IOBufQueue>(foo).moveAsValue();
}

template <class Struct>
folly::IOBuf genListI32() {
  Struct foo;
  foo.field4_ref().emplace(10'000, 1'000);
  return CompactSerializer::serialize<folly::IOBufQueue>(foo).moveAsValue();
}

const folly::IOBuf kListDoubleWithIndex = genListDouble<LazyFoo>();
const folly::IOBuf kListDoubleWithoutIndex = genListDouble<Foo>();
const folly::IOBuf kListI32WithIndex = genListI32<LazyFoo>();
const folly::IOBuf kListI32WithoutIndex = genListI32<Foo>();

// We only need to disable checksum for list<i32> with index, since
// `list<double>` is cheap to skip, we won't compute its index and checksum.
const folly::IOBuf kListI32WithIndexNoChecksum =
    genListI32<LazyFooNoChecksum>();

BENCHMARK(list_double_with_index_eager) {
  CompactSerializer::deserialize<Foo>(&kListDoubleWithIndex);
}
BENCHMARK_RELATIVE(list_double_with_index_lazy) {
  CompactSerializer::deserialize<LazyFoo>(&kListDoubleWithIndex);
}
BENCHMARK_RELATIVE(list_double_with_index_lazy_then_access_field) {
  CompactSerializer::deserialize<LazyFoo>(&kListDoubleWithIndex).field3_ref();
}
BENCHMARK(list_i32_with_index_eager) {
  CompactSerializer::deserialize<Foo>(&kListI32WithIndex);
}
BENCHMARK_RELATIVE(list_i32_with_index_lazy) {
  CompactSerializer::deserialize<LazyFoo>(&kListI32WithIndex);
}
BENCHMARK_RELATIVE(list_i32_with_index_lazy_without_checksum) {
  CompactSerializer::deserialize<LazyFooNoChecksum>(
      &kListI32WithIndexNoChecksum);
}
BENCHMARK_RELATIVE(list_i32_with_index_lazy_then_access_field) {
  CompactSerializer::deserialize<LazyFoo>(&kListI32WithIndex).field4_ref();
}
BENCHMARK_RELATIVE(
    list_i32_with_index_lazy_then_access_field_without_checksum) {
  CompactSerializer::deserialize<LazyFooNoChecksum>(
      &kListI32WithIndexNoChecksum)
      .field4_ref();
}
BENCHMARK(list_double_without_index_eager) {
  CompactSerializer::deserialize<Foo>(&kListDoubleWithIndex);
}
BENCHMARK_RELATIVE(list_double_without_index_lazy) {
  CompactSerializer::deserialize<LazyFoo>(&kListDoubleWithIndex);
}
BENCHMARK_RELATIVE(list_double_without_index_lazy_then_access_field) {
  CompactSerializer::deserialize<LazyFoo>(&kListDoubleWithIndex).field3_ref();
}
BENCHMARK(list_i32_without_index_eager) {
  CompactSerializer::deserialize<Foo>(&kListI32WithIndex);
}
BENCHMARK_RELATIVE(list_i32_without_index_lazy) {
  CompactSerializer::deserialize<LazyFoo>(&kListI32WithIndex);
}
BENCHMARK_RELATIVE(list_i32_without_index_lazy_then_access_field) {
  CompactSerializer::deserialize<LazyFoo>(&kListI32WithIndex).field4_ref();
}

} // namespace apache::thrift::test

int main(int argc, char** argv) {
  folly::init(&argc, &argv);
  folly::runBenchmarks();
}
