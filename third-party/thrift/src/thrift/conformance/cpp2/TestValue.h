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

#pragma once

#include <thrift/conformance/cpp2/internal/TestValue.h>

#include <thrift/conformance/cpp2/Any.h>
#include <thrift/conformance/cpp2/ThriftTypeInfo.h>
#include <thrift/conformance/if/gen-cpp2/test_value_types.h>

namespace apache::thrift::conformance {

template <typename W>
Any encodeValue(const Protocol& protocol, const EncodeValue& value) {
  // Encode the data.
  folly::IOBufQueue queue(folly::IOBufQueue::cacheChainLength());
  W writer;
  writer.setOutput(&queue);
  detail::invoke(*value.writes(), writer);

  // Build the result.
  Any result;
  result.type() = *value.type();
  setProtocol(protocol, result);
  result.data() = queue.moveAsValue();
  return result;
}

template <typename T>
EncodeValue asEncodeValue(const T& value) {
  EncodeValue result;
  result.type() = *getGeneratedThriftTypeInfo<T>().uri();

  detail::EncodeValueRecorder recorder(&result);
  value.write(&recorder);
  return result;
}

} // namespace apache::thrift::conformance
