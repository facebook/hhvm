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

#include <thrift/lib/cpp2/protocol/Object.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/lib/cpp2/test/Structs.h>

#include <folly/Benchmark.h>
#include <folly/init/Init.h>

namespace apache::thrift::protocol {

using ::thrift::benchmark::ComplexStruct;
using Tag = type::struct_t<ComplexStruct>;

const auto obj = asValueStruct<Tag>(create<ComplexStruct>()).as_object();

BENCHMARK(Serialization) {
  auto buf = serializeObject<apache::thrift::BinaryProtocolWriter>(obj);
  BinaryProtocolReader reader;
  reader.setInput(buf.get());
  ComplexStruct t;
  t.read(&reader);
}

BENCHMARK_RELATIVE(ToThriftStruct) {
  fromObjectStruct<Tag>(obj);
}

} // namespace apache::thrift::protocol

int main(int argc, char** argv) {
  folly::Init init(&argc, &argv);
  folly::runBenchmarks();
}
