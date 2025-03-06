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

#include <thrift/lib/cpp2/frozen/FrozenUtil.h>
#include <thrift/lib/cpp2/protocol/Object.h>
#include <thrift/lib/cpp2/test/ObjectBenchUtils.h>
#include <thrift/lib/cpp2/test/Structs.h>

#include <glog/logging.h>
#include <folly/Benchmark.h>
#include <folly/BenchmarkUtil.h>
#include <folly/init/Init.h>
#include <folly/portability/GFlags.h>

using namespace apache::thrift;
using namespace thrift::benchmark;
using namespace apache::thrift::test::utils;

FBTHRIFT_GEN_SERDE()

constexpr std::size_t QUEUE_ALLOC_SIZE = 16 * 1024 * 1024;

DEFINE_string(operation, "", "The operation to profile.");
DEFINE_string(type, "", "The type to profile.");
DEFINE_string(protocol, "Binary", "The protocol to serialize/deserialize with");

template <
    typename T,
    typename ProtocolWriter,
    typename ProtocolReader,
    typename F>
void call(F&& f) {
  const TestingObject& input = get_serde<T, ProtocolWriter, ProtocolReader>();
  while (true) {
    f(input);
    get_queue().clearAndTryReuseLargestBuffer();
  }
}

int main(int argc, char** argv) {
  gflags::SetUsageMessage(
      "Profile a Protocol.Object benchmark:\nSpecify:\n\t- --protocol: Binary, Compact\n\t- --operation: encode, read_all, ...\n\t- --type: e.g. Empty, ComplexStruct, ...");
  folly::Init init(&argc, &argv);

  get_queue().preallocate(QUEUE_ALLOC_SIZE, QUEUE_ALLOC_SIZE);

  const auto op = FLAGS_operation;
  if (op == "") {
    LOG(ERROR)
        << "Missing operation: Choose from an operation in ObjectBenchUtils.h"
        << op;
    return 1;
  }

  const auto type = FLAGS_type;
  if (type == "") {
    LOG(ERROR) << "Missing type: Choose from a type in ObjectBenchUtils.h"
               << op;
    return 1;
  }

  const auto protocol = FLAGS_protocol;
  if (protocol == "") {
    LOG(ERROR)
        << "Missing Protocol: Choose from a protocol in ObjectBenchUtils.h"
        << op;
    return 1;
  }

#define FBTHRIFT_REGISTER_OP(PROT_NAME, OP, WRITER, READER, T) \
  if (op == #OP && protocol == #PROT_NAME && type == #T) {     \
    call<T, WRITER, READER>(                                   \
        [](const auto& input) { OP<WRITER, READER>(input); }); \
    return 0;                                                  \
  }

  FBTHRIFT_GEN_PROTOCOL_BENCHMARKS(FBTHRIFT_REGISTER_OP)

  LOG(ERROR) << "No matching benchmark found";
  return 2;
}
