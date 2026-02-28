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

#include <thrift/lib/cpp2/gen/module_metadata_h.h>
#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>
#include <thrift/lib/cpp2/protocol/Protocol.h>
#include <thrift/lib/cpp2/runtime/SchemaRegistry.h>
#include <thrift/lib/cpp2/schema/SchemaV1.h>
#include <thrift/lib/cpp2/schema/SyntaxGraph.h>
#include <thrift/lib/cpp2/schema/detail/Merge.h>
#include <thrift/lib/cpp2/transport/core/ThriftChannelIf.h>
#include <thrift/lib/cpp2/transport/core/ThriftRequest.h>

namespace apache::thrift::detail {

/**
 * Helper function to look up a FunctionNode from a service's schema.
 * Returns nullptr if the schema is not available or function is not found.
 */
template <typename ServiceTag>
const syntax_graph::FunctionNode* getFunctionNode(
    std::string_view functionName) {
  try {
    const auto& service = SchemaRegistry::get().getNode<ServiceTag>();
    for (const auto& fn : service.functions()) {
      if (fn.name() == functionName) {
        return &fn;
      }
    }
  } catch (const std::out_of_range&) {
    // Schema not available for this service
  }
  return nullptr;
}

/**
 * Helper function to look up a FunctionNode from an interaction's schema.
 * The interactionName is the name of the interaction, and functionName is the
 * method within it.
 * Returns nullptr if the schema is not available or function is not found.
 */
template <typename ServiceTag>
const syntax_graph::FunctionNode* getInteractionFunctionNode(
    std::string_view interactionName, std::string_view functionName) {
  try {
    const auto& service = SchemaRegistry::get().getNode<ServiceTag>();
    // Look through functions to find the interaction factory
    for (const auto& fn : service.functions()) {
      const auto& response = fn.response();
      if (const auto* interaction = response.interaction()) {
        if (interaction->definition().name() == interactionName) {
          for (const auto& interactionFn : interaction->functions()) {
            if (interactionFn.name() == functionName) {
              return &interactionFn;
            }
          }
        }
      }
    }
  } catch (const std::out_of_range&) {
    // Schema not available for this service
  }
  return nullptr;
}

} // namespace apache::thrift::detail
