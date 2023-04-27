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

#include <thrift/conformance/data/internal/TestGenerator.h>

#include <cstdio>
#include <set>

#include <glog/logging.h>
#include <folly/io/IOBufQueue.h>
#include <thrift/conformance/cpp2/AnyRegistry.h>
#include <thrift/conformance/cpp2/Protocol.h>
#include <thrift/conformance/if/gen-cpp2/test_suite_types.h>
#include <thrift/lib/cpp2/Thrift.h>

namespace apache::thrift::conformance::data {

TestSuite createRoundTripSuite(
    const std::set<Protocol>& protocols,
    const AnyRegistry& registry = AnyRegistry::generated());

inline TestSuite createRoundTripSuite(
    const std::set<StandardProtocol>& protocols = detail::kDefaultProtocols,
    const AnyRegistry& registry = AnyRegistry::generated()) {
  return createRoundTripSuite(detail::toProtocols(protocols), registry);
}

inline TestSuite createRoundTripSuite(const AnyRegistry& registry) {
  return createRoundTripSuite(detail::kDefaultProtocols, registry);
}

template <class Writer, class T>
std::enable_if_t<apache::thrift::is_thrift_class_v<T>> serializeToFile(
    const T& s, std::FILE* f) {
  Writer writer;
  folly::IOBufQueue queue(folly::IOBufQueue::cacheChainLength());
  writer.setOutput(&queue);
  s.write(&writer);
  while (auto buf = queue.pop_front()) {
    CHECK_EQ(
        std::fwrite(buf->data(), sizeof(uint8_t), buf->length(), f),
        buf->length());
  }
}

} // namespace apache::thrift::conformance::data
