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

#include <utility>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineImpl.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/connection/security/common/Messages.h>

namespace apache::thrift::fast_thrift::connection::security::handler {

/**
 * Head endpoint of the TLS pipeline.
 *
 * The pipeline runs its work on the write path: a freshly accepted transport
 * is driven in as a write at the tail and flows config → classifier →
 * handshake → stopTLS before arriving here. This endpoint is therefore the
 * single point all security outcomes converge on — handshaked, classified
 * plaintext, or StopTLS-downgraded.
 *
 * It collapses the resolved request into a response and turns it around onto
 * the read path back to the tail adapter — the single request→response
 * conversion for the pipeline. Per-connection finalization (policy
 * enforcement, security-outcome observability) belongs here too, since every
 * path converges at this point.
 */
class TLSFinalizer {
 public:
  TLSFinalizer() = default;
  ~TLSFinalizer() = default;
  TLSFinalizer(const TLSFinalizer&) = delete;
  TLSFinalizer& operator=(const TLSFinalizer&) = delete;
  TLSFinalizer(TLSFinalizer&&) = delete;
  TLSFinalizer& operator=(TLSFinalizer&&) = delete;

  void setPipeline(channel_pipeline::PipelineImpl* pipeline) noexcept {
    pipeline_ = pipeline;
  }

  // Write-exit: every security stage has run and the transport is resolved.
  // Collapse the request down to a response and turn it around onto the read
  // path so it returns to the tail adapter.
  channel_pipeline::Result onWrite(
      channel_pipeline::TypeErasedBox&& msg) noexcept {
    if (FOLLY_UNLIKELY(!pipeline_)) {
      return channel_pipeline::Result::Error;
    }
    auto request = msg.take<TLSRequestMessage>();
    TLSResponseMessage response{
        .transport = std::move(request.transport),
        .clientAddr = std::move(request.clientAddr),
    };
    return pipeline_->fireRead(
        channel_pipeline::erase_and_box(std::move(response)));
  }

  void onReadReady() noexcept {}
  void handlerAdded() noexcept {}
  void handlerRemoved() noexcept {}
  void onPipelineActive() noexcept {}
  void onPipelineInactive() noexcept {}

 private:
  channel_pipeline::PipelineImpl* pipeline_{nullptr};
};

} // namespace apache::thrift::fast_thrift::connection::security::handler
