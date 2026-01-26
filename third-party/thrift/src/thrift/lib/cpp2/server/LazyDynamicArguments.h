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

#include <stdexcept>
#include <string_view>
#include <vector>

#include <folly/container/F14Map.h>
#include <folly/io/IOBuf.h>
#include <folly/synchronization/DelayedInit.h>

#include <thrift/lib/cpp/protocol/TProtocolTypes.h>
#include <thrift/lib/cpp2/dynamic/DynamicValue.h>
#include <thrift/lib/cpp2/schema/SyntaxGraph.h>

namespace apache::thrift {

/**
 * Provides lazy access to method arguments as DynamicValue objects.
 *
 * Arguments are deserialized on-demand from the serialized request buffer
 * using schema information. This allows interceptors to inspect arguments
 * dynamically without knowing compile-time types.
 *
 * Usage:
 *   auto* args = requestInfo.dynamicArguments;
 *   if (args) {
 *     // Access by index
 *     if (auto arg0 = args->get(0)) {
 *       LOG(INFO) << "First argument: " << arg0->debugString();
 *     }
 *     // Or by parameter name
 *     if (auto userId = args->get("userId")) {
 *       LOG(INFO) << "userId: " << userId->debugString();
 *     }
 *   }
 */
class LazyDynamicArguments {
 public:
  LazyDynamicArguments(
      const folly::IOBuf* serializedRequestBuffer,
      const syntax_graph::FunctionNode* functionNode,
      protocol::PROTOCOL_TYPES protocolType);

  /**
   * Returns the number of parameters for this function.
   */
  std::size_t count() const;

  /**
   * Get argument by 0-based index. Lazily deserializes on first access.
   * Missing parameters are filled with default values.
   * @throws std::out_of_range if index is out of bounds.
   */
  dynamic::DynamicConstRef get(std::size_t index) const;

  /**
   * Get argument by parameter name. Lazily deserializes on first access.
   * Missing parameters are filled with default values.
   * @throws std::out_of_range if parameter name is not found.
   */
  dynamic::DynamicConstRef get(std::string_view paramName) const;

 private:
  using ArgsMap = folly::F14FastMap<FieldId, dynamic::DynamicValue>;

  const ArgsMap& ensureDeserialized() const;

  const folly::IOBuf* serializedRequestBuffer_;
  const syntax_graph::FunctionNode* functionNode_;
  protocol::PROTOCOL_TYPES protocolType_;

  // Lazy cache for deserialized arguments, keyed by FieldId
  mutable folly::DelayedInit<ArgsMap> cachedArgs_;
};

} // namespace apache::thrift
