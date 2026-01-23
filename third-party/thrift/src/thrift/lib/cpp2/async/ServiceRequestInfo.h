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

#include <thrift/lib/cpp/concurrency/Thread.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift {

namespace syntax_graph {
class FunctionNode;
} // namespace syntax_graph

// This contains information about a request that is required in the thrift
// server prior to the AsyncProcessor::executeRequest interface.
struct ServiceRequestInfo {
  bool isSync; // True if this has thread='eb'
  RpcKind rpcKind; // Type of this request
  // The qualified function name is currently an input to TProcessorEventHandler
  // callbacks. We will refactor TProcessorEventHandler to remove the
  // requirement to pass this as a single string. T112104402
  const char* functionName_deprecated; // Qualified function name (includes
                                       // service name)
  std::optional<std::string>
      interactionName; // Interaction name if part of an interaction
  concurrency::PRIORITY priority; // Method priority set in the IDL
  std::optional<std::string>
      createdInteraction; // The name of the interaction created by the RPC
  const syntax_graph::FunctionNode* functionNode{nullptr}; // Schema node
};

using ServiceRequestInfoMap =
    folly::F14ValueMap<std::string, ServiceRequestInfo>;

} // namespace apache::thrift
