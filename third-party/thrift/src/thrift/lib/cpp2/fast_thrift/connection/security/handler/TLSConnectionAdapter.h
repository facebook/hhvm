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

#include <folly/ExceptionWrapper.h>
#include <folly/SocketAddress.h>
#include <folly/io/async/AsyncTransport.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineImpl.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/connection/security/common/Messages.h>

namespace apache::thrift::fast_thrift::connection::security::handler {

/**
 * Tail endpoint of the TLS pipeline and its single integration point with the
 * owner that drives it.
 *
 * submit() drives a freshly accepted transport through the pipeline as an
 * outbound write — the security stages process on the write path. When the
 * resolved transport returns on the read path, onRead() dispatches it to the
 * owner via the resolved trampoline; exceptions raised inside the pipeline are
 * dispatched the same way via the exception trampoline so the owner can
 * propagate them. The owner registers both through setOwner() — function
 * pointers, not std::function, so there is no per-accept allocation or
 * indirect-call overhead.
 */
class TLSConnectionAdapter {
 public:
  // Dispatches the resolved transport back to the owner once the pipeline
  // completes. Returns the downstream Result so backpressure / errors
  // propagate to the caller.
  using ResolvedFn = channel_pipeline::Result (*)(
      void*, folly::AsyncTransport::UniquePtr, folly::SocketAddress) noexcept;
  // Dispatches an exception raised inside the pipeline back to the owner.
  using ExceptionFn = void (*)(void*, folly::exception_wrapper&&) noexcept;

  TLSConnectionAdapter() noexcept = default;
  ~TLSConnectionAdapter() = default;
  TLSConnectionAdapter(const TLSConnectionAdapter&) = delete;
  TLSConnectionAdapter& operator=(const TLSConnectionAdapter&) = delete;
  TLSConnectionAdapter(TLSConnectionAdapter&&) = delete;
  TLSConnectionAdapter& operator=(TLSConnectionAdapter&&) = delete;

  // Register the owner and its dispatch trampolines. Must be called before the
  // pipeline runs.
  void setOwner(
      void* owner, ResolvedFn onResolved, ExceptionFn onException) noexcept {
    owner_ = owner;
    onResolved_ = onResolved;
    onException_ = onException;
  }

  void setPipeline(channel_pipeline::PipelineImpl* pipeline) noexcept {
    pipeline_ = pipeline;
  }

  // Drive a transport through the TLS pipeline. Enters as an outbound write at
  // the write-entry handler and flows toward the head.
  channel_pipeline::Result submit(TLSRequestMessage&& msg) noexcept {
    if (FOLLY_UNLIKELY(!pipeline_)) {
      return channel_pipeline::Result::Error;
    }
    return pipeline_->fireWrite(
        channel_pipeline::erase_and_box(std::move(msg)));
  }

  // Read-exit: the resolved transport returns here. Hand it back to the owner.
  channel_pipeline::Result onRead(
      channel_pipeline::TypeErasedBox&& msg) noexcept {
    auto resolved = msg.take<TLSResponseMessage>();
    DCHECK(onResolved_);
    return onResolved_(
        owner_, std::move(resolved.transport), std::move(resolved.clientAddr));
  }

  void onException(folly::exception_wrapper&& e) noexcept {
    DCHECK(onException_);
    onException_(owner_, std::move(e));
  }

  void onWriteReady() noexcept {}
  void handlerAdded() noexcept {}
  void handlerRemoved() noexcept {}
  void onPipelineActive() noexcept {}
  void onPipelineInactive() noexcept {}

 private:
  void* owner_{nullptr};
  ResolvedFn onResolved_{nullptr};
  ExceptionFn onException_{nullptr};
  channel_pipeline::PipelineImpl* pipeline_{nullptr};
};

} // namespace apache::thrift::fast_thrift::connection::security::handler
