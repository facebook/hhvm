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

#include <string_view>
#include <type_traits>
#include <typeinfo>

#include <folly/Demangle.h>

#include <thrift/lib/cpp2/async/AsyncProcessor.h>
#include <thrift/lib/cpp2/async/ResponseChannel.h>

namespace apache::thrift {
/**
 * A class encapsulating utilities that are useful for custom implementations of
 * AsyncProcessor or AsyncProcessorFactory.
 */
class AsyncProcessorHelper {
  AsyncProcessorHelper() = delete;

 public:
  static void sendUnknownMethodError(
      ResponseChannelRequest::UniquePtr req, std::string_view methodName);
  static void sendInvalidInteractionIdError(
      ResponseChannelRequest::UniquePtr request,
      std::string_view methodName,
      int64_t interactionId);

  template <
      typename Metadata,
      typename = std::enable_if_t<
          std::is_base_of_v<AsyncProcessorFactory::MethodMetadata, Metadata>>>
  static const Metadata& expectMetadataOfType(
      const AsyncProcessorFactory::MethodMetadata& methodMetadata) {
    DCHECK(dynamic_cast<const Metadata*>(&methodMetadata) != nullptr)
        << "Received MethodMetadata of an unknown type: "
        << folly::demangle(typeid(methodMetadata).name());
    return static_cast<const Metadata&>(methodMetadata);
  }

  // Helper code to bridge from a ServerRequest to calling the
  // AsyncProcessor::execute method.
  static void executeRequest(ServerRequest&& request);

  // Helper code to choose a valid resource pool.
  static SelectPoolResult selectResourcePool(
      const ServerRequest& request,
      const AsyncProcessorFactory::MethodMetadata& methodMetadata);
};

} // namespace apache::thrift
