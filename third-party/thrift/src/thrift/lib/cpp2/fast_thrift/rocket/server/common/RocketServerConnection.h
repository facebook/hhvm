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

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/BufferAllocator.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineImpl.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/adapter/RocketServerAppAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/transport/TransportHandler.h>

namespace apache::thrift::fast_thrift::rocket::server {

/**
 * RocketServerConnection — owns the rocket pipeline and its
 * resources for a single server connection.
 *
 * Groups the rocket-layer objects that together form the transport
 * underneath a thrift server pipeline:
 *
 *   TransportHandler → [rocket handlers] → RocketServerAppAdapter
 *
 * The ThriftServerTransportAdapter holds a unique_ptr to this struct and
 * can close the connection when the thrift pipeline encounters an error.
 */
struct RocketServerConnection {
  rocket::server::RocketServerAppAdapter::Ptr appAdapter{
      new rocket::server::RocketServerAppAdapter()};
  transport::TransportHandler::Ptr transportHandler;
  channel_pipeline::PipelineImpl::Ptr pipeline;
  channel_pipeline::SimpleBufferAllocator allocator;

  /**
   * Close the connection and tear down the rocket pipeline.
   */
  void close(folly::exception_wrapper&& e) noexcept {
    if (transportHandler) {
      // Clear the close callback before calling onClose to prevent re-entry:
      // onClose() triggers the callback, which would call removeConnection(),
      // which calls close() again — causing double-close and iterator
      // invalidation in closeAllConnections().
      transportHandler->setCloseCallback(nullptr);
      transportHandler->onClose(std::move(e));
      transportHandler.reset();
    }
    if (pipeline) {
      pipeline->close();
      pipeline.reset();
    }
  }
};

} // namespace apache::thrift::fast_thrift::rocket::server
