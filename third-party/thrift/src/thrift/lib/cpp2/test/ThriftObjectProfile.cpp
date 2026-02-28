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

#define FBTHRIFT_DEFINE_STR(NAME, ...) #NAME,

constexpr auto TYPE_OPTIONS = {
    FBTHRIFT_FOR_EACH_UNIQUE_TYPE(FBTHRIFT_DEFINE_STR) "Unknown"};

constexpr auto OPERATION_OPTIONS = {
    FBTHRIFT_FOR_EACH_UNIQUE_OPERATION(FBTHRIFT_DEFINE_STR) "Unknown"};

constexpr auto PROTOCOL_OPTIONS = {
    FBTHRIFT_FOR_EACH_UNIQUE_PROTOCOL(FBTHRIFT_DEFINE_STR) "Unknown"};

constexpr auto IMPLEMENTATION_OPTIONS = {
    "Protocol", "protocol", "Native", "native"};

std::string join_options_as_csv(const auto& list) {
  return folly::join(", ", list);
}

DEFINE_string(operation, "", "The operation to profile.");
DEFINE_string(type, "", "The type to profile.");
DEFINE_string(
    protocol,
    "Binary",
    "The protocol used to to serialize/deserialize Protocol.Object/Value with");
DEFINE_string(impl, "protocol", "The implementation for Object");

template <
    typename T,
    typename ProtocolWriter,
    typename ProtocolReader,
    typename F>
void call(F&& f) {
  const TestingObject& input = get_serde<T, ProtocolWriter, ProtocolReader>();
  while (true) {
    f(input);
  }
}

int main(int argc, char** argv) {
  gflags::SetUsageMessage(
      "Profile a Protocol.Object benchmark:\nSpecify:\n\t- --protocol: Binary, Compact\n\t- --operation: encode, read_all, ...\n\t- --type: e.g. Empty, ComplexStruct, ...\n\t- --impl: .e.g. protocol");
  folly::Init init(&argc, &argv);

  get_queue().preallocate(QUEUE_ALLOC_SIZE, QUEUE_ALLOC_SIZE);

  const auto op = FLAGS_operation;
  if (op == "") {
    LOG(ERROR) << "Missing operation. Choose from: "
               << join_options_as_csv(OPERATION_OPTIONS);
    return 1;
  }

  const auto type = FLAGS_type;
  if (type == "") {
    LOG(ERROR) << "Missing type. Choose from "
               << join_options_as_csv(TYPE_OPTIONS);
    return 1;
  }

  const auto protocol = FLAGS_protocol;
  if (std::find(PROTOCOL_OPTIONS.begin(), PROTOCOL_OPTIONS.end(), protocol) ==
      PROTOCOL_OPTIONS.end()) {
    LOG(ERROR) << "Invalid protocol. Choose from: "
               << join_options_as_csv(PROTOCOL_OPTIONS);
    return 1;
  }

  const auto impl = FLAGS_impl;
  if (std::find(
          IMPLEMENTATION_OPTIONS.begin(), IMPLEMENTATION_OPTIONS.end(), impl) ==
      IMPLEMENTATION_OPTIONS.end()) {
    LOG(ERROR) << "Invalid implementation. Choose from: "
               << join_options_as_csv(IMPLEMENTATION_OPTIONS);
    return 1;
  }

#define FBTHRIFT_REGISTER_OP(PROT_NAME, OP, WRITER, READER, T)            \
  if (op == #OP && protocol == #PROT_NAME && type == #T) {                \
    if (impl == "protocol" || impl == "Protocol") {                       \
      call<T, WRITER, READER>(                                            \
          [](const auto& input) { OP<WRITER, READER>(input); });          \
      return 0;                                                           \
    }                                                                     \
    if (impl == "native" || impl == "Native") {                           \
      call<T, WRITER, READER>(                                            \
          [](const auto& input) { OP##_native<WRITER, READER>(input); }); \
      return 0;                                                           \
    }                                                                     \
  }

  FBTHRIFT_GEN_PROTOCOL_BENCHMARKS(FBTHRIFT_REGISTER_OP)

  LOG(ERROR) << "No matching benchmark found";
  return 2;
}
