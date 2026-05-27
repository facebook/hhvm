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

#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include <folly/ExceptionWrapper.h>
#include <folly/Portability.h>
#include <folly/Synchronized.h>
#include <folly/container/F14Map.h>
#include <folly/io/IOBuf.h>
#include <folly/io/async/DelayedDestruction.h>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineImpl.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/Messages.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::fast_thrift::thrift {

/**
 * ThriftServerAppAdapter — base class for generated fast server handlers.
 *
 * The generated <Service>AppAdapter extends this class, inheriting pipeline
 * integration, request dispatch, and the single writeResponse write entry
 * point. Payload construction lives in ResponsePayloads.h — callers build a
 * ThriftServerResponseMessage there, then hand it to writeResponse().
 *
 * Satisfies TailEndpointHandler concept (onRead / onException) and the
 * ThriftAppAdapter concept consumed by ThriftServerCompositeAppAdapter.
 *
 * State machine (one-directional; adapter is per-connection and never reused):
 *   Created   --setPipeline-->        Ready
 *   Ready     --onPipelineActive-->   Open
 *   Open      --onException(NOT_OPEN
 *               | END_OF_FILE
 *               | INTERRUPTED
 *               | TIMED_OUT)-->       Closing
 *   Open      --onException(other)--> Closed
 *   Closing   --onPipelineInactive--> Closed
 *   Open      --onPipelineInactive--> Closed
 *
 *   onRead is accepted only in Open. Closing/Closed reject new requests.
 *   writeResponse is accepted in Open and Closing (in-flight handler
 *   callbacks may still try to send a final response; the pipeline may
 *   swallow it if the wire is gone). Closed refuses all writes.
 *
 *   In-flight requests gate the transition to Closed and the firing of
 *   closeCallback. Every dispatched request increments an in-flight counter
 *   at handleRequestResponse entry; every writeResponse decrements it. When
 *   onPipelineInactive / onException fires with in-flight > 0, the adapter
 *   stays in Closing and defers closeCallback until the last writeResponse
 *   drains the count. Upper-layer owners must hold the adapter Ptr until
 *   closeCallback fires — dropping earlier risks UAF from in-flight handler
 *   callbacks still writing through the adapter.
 */
class ThriftServerAppAdapter : public folly::DelayedDestruction {
 public:
  // Canonical owning-pointer alias. Adapters are folly::DelayedDestruction
  // objects, so they can't be deleted with plain delete. Every site that
  // owns an adapter (FastThriftServer's per-connection state, factory
  // returns, composite children) should use this alias.
  using Ptr = std::
      unique_ptr<ThriftServerAppAdapter, folly::DelayedDestruction::Destructor>;

  // Per-method handler signature for SINGLE_REQUEST_SINGLE_RESPONSE RPCs.
  // The dispatcher pre-extracts the components codegen actually needs from
  // the inbound frame so the handler doesn't have to reach back into
  // ThriftServerRequestMessage / ParsedFrame.
  //   streamId       — Rocket stream id; echo into the response so the client
  //                    can match it to the in-flight request.
  //   data           — request payload IOBuf, ready to feed into a protocol
  //                    reader for argument deserialization.
  //   protocol       — wire protocol id (Binary / Compact / ...) for the
  //                    codec.
  //   requestContext — per-request context stamped by
  //                    ThriftServerRequestContextHandler; ownership moves into
  //                    the FastHandlerCallback so it outlives async handlers.
  using RequestResponseProcessFn = channel_pipeline::Result (*)(
      ThriftServerAppAdapter*,
      uint32_t streamId,
      std::unique_ptr<folly::IOBuf> data,
      apache::thrift::ProtocolId protocol,
      std::unique_ptr<ThriftRequestContext> requestContext) noexcept;

  enum class State : uint8_t {
    Created,
    Ready,
    Open,
    Closing,
    Closed,
  };

  State state() const noexcept { return state_; }

  ThriftServerAppAdapter() = default;
  ThriftServerAppAdapter(ThriftServerAppAdapter&&) = delete;
  ThriftServerAppAdapter& operator=(ThriftServerAppAdapter&&) = delete;

  void setCloseCallback(std::function<void()> cb);

  // Server-initiated graceful drain. Sends a stream-0
  // ERROR(CONNECTION_CLOSE) so the peer stops issuing new REQUEST_*
  // frames, then flips Open → Closing and defers closeCallback until
  // inFlight_ drains. If the wire send fails (pipeline already dead),
  // skips the drain wait and tears down immediately — there's no path
  // for in-flight responses to land. Must be called on the pipeline's
  // EventBase.
  void startDrain() noexcept;

  void setPipeline(channel_pipeline::PipelineImpl* pipeline) noexcept;

  channel_pipeline::PipelineImpl* pipeline() const noexcept {
    return pipeline_;
  }

  // === TailEndpointHandler interface ===

  channel_pipeline::Result onRead(
      channel_pipeline::TypeErasedBox&& msg) noexcept;

  void onException(folly::exception_wrapper&& e) noexcept;

  void handlerAdded() noexcept {}
  void handlerRemoved() noexcept {}
  void onPipelineActive() noexcept;
  void onPipelineInactive() noexcept;
  void onWriteReady() noexcept {}

  // Single write entry point. Fires the message through the pipeline,
  // decrements in-flight, and finalizes a deferred close when in-flight
  // hits zero. Refuses writes in Closed.
  //
  // Build the message via the free functions in
  // thrift/server/util/ResponsePayloads.h (makeResponseMessage,
  // makeErrorMessage, makeFrameworkErrorMessage, makeUnknownExceptionMessage,
  // makeSuccessResponseMessage<>, makeDeclaredExceptionMessage<>, ...).
  [[nodiscard]] channel_pipeline::Result writeResponse(
      ThriftServerResponseMessage&& message) noexcept;

  // Methods registered via addMethodHandler — consumed by the composite
  // to build its name -> owner routing map.
  std::vector<std::string_view> methodNames() const noexcept;

 protected:
  void addMethodHandler(
      std::string_view name, RequestResponseProcessFn handler);

  channel_pipeline::PipelineImpl* pipeline_{nullptr};

  ~ThriftServerAppAdapter() override;

  folly::EventBase* getEventBase() const { return evb_; }

 private:
  channel_pipeline::Result handleRequestResponse(
      ThriftServerRequestMessage&& request) noexcept;
  FOLLY_NOINLINE channel_pipeline::Result handleWrongRpcKind(
      uint32_t streamId, apache::thrift::RpcKind kind) noexcept;
  FOLLY_NOINLINE channel_pipeline::Result handleUnknownMethod(
      uint32_t streamId, std::string_view methodName) noexcept;
  FOLLY_NOINLINE channel_pipeline::Result handleMissingPipeline() noexcept;
  FOLLY_NOINLINE channel_pipeline::Result handleConnectionClosed() noexcept;
  FOLLY_NOINLINE void handleDeferredClose() noexcept;

  folly::EventBase* evb_{nullptr};
  folly::F14FastMap<std::string, RequestResponseProcessFn> dispatch_;
  folly::Synchronized<std::function<void()>> closeCallback_;
  State state_{State::Created};
  uint32_t inFlight_{0};
  bool closeDeferred_{false};

  void fireCloseCallback() noexcept;
};

} // namespace apache::thrift::fast_thrift::thrift
