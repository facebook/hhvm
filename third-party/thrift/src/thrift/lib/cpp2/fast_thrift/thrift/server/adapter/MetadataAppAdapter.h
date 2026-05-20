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

#include <memory>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/adapter/ThriftServerAppAdapter.h>
#include <thrift/lib/thrift/gen-cpp2/metadata_types.h>

namespace apache::thrift::fast_thrift::thrift {

/**
 * Built-in adapter that serves a single RPC: getThriftServiceMetadata.
 *
 * The ThriftServiceMetadataResponse is built once at server startup (by
 * FastThriftServer calling handler_->getServiceMetadata) and cached, then
 * re-serialized into the negotiated protocol on each call.
 *
 * One instance is minted per connection (so it can hold per-connection
 * pipeline state inherited from ThriftServerAppAdapter) but every instance
 * shares the same immutable response via shared_ptr — no extra copies, no
 * extra walks.
 *
 * Mounted into a ThriftServerCompositeAppAdapter alongside the user handler
 * when FastThriftServerConfig::enableMetadataService is set.
 */
class MetadataAppAdapter final : public ThriftServerAppAdapter {
 public:
  using Ptr = std::
      unique_ptr<MetadataAppAdapter, folly::DelayedDestruction::Destructor>;

  // `response` is built once at server startup and shared across all
  // connection-scoped MetadataAppAdapter instances. Must be non-null.
  explicit MetadataAppAdapter(
      std::shared_ptr<
          const apache::thrift::metadata::ThriftServiceMetadataResponse>
          response);

  const apache::thrift::metadata::ThriftServiceMetadataResponse& response()
      const noexcept {
    return *response_;
  }

 private:
  std::shared_ptr<const apache::thrift::metadata::ThriftServiceMetadataResponse>
      response_;

  // Static thunk registered via addMethodHandler. Switches on protocol id and
  // dispatches to the templated impl that fills the presult and writes the
  // success response.
  static channel_pipeline::Result handleGetThriftServiceMetadata(
      ThriftServerAppAdapter* self,
      uint32_t streamId,
      std::unique_ptr<folly::IOBuf> requestData,
      apache::thrift::ProtocolId protocol) noexcept;

  template <typename Writer>
  channel_pipeline::Result writeMetadataResponse(uint32_t streamId) noexcept;
};

} // namespace apache::thrift::fast_thrift::thrift
