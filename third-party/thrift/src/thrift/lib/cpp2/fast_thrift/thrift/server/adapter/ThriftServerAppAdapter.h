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
#include <optional>
#include <string>
#include <string_view>
#include <utility>
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
#include <thrift/lib/cpp2/fast_thrift/frame/ErrorCode.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/MetadataProtocol.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/util/ResponseMetadata.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/util/ResponseSerializer.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::fast_thrift::thrift {

/**
 * ThriftServerAppAdapter — base class for generated fast server handlers.
 *
 * The generated <Service>AppAdapter extends this class, inheriting pipeline
 * integration, metadata deserialization, and response writing.
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
 *   writeResponse / writeError / writeFrameworkError are accepted in Open
 *   and Closing (in-flight handler callbacks may still try to send a final
 *   response; the pipeline may swallow it if the wire is gone). Closed
 *   refuses all writes.
 *
 *   In-flight requests gate the transition to Closed and the firing of
 *   closeCallback. Every dispatched request increments an in-flight
 *   counter at handleRequestResponse entry; every response (success,
 *   declared/undeclared exception, framework error, or the not-completed
 *   exception fired by the FHC destructor) decrements via fireResponse.
 *   When onPipelineInactive / onException fires with in-flight > 0, the
 *   adapter stays in Closing and defers closeCallback until the last
 *   fireResponse drains the count. Upper-layer owners must hold the
 *   adapter Ptr until closeCallback fires — dropping earlier risks UAF
 *   from in-flight handler callbacks still writing through the adapter.
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

  // Sends a PAYLOAD frame. Caller (codegen / framework) is responsible for
  // building the data buffer (serialized presult, or nullptr) and the typed
  // ResponseRpcMetadata struct (populated via fillSuccessResponseMetadata /
  // fillAppErrorResponseMetadata / fillDeclaredExceptionMetadata, or by
  // hand). Metadata serialization is deferred into the pipeline
  // (ThriftInitialResponsePayload::toRocketFrame) so the adapter never has to
  // pre-serialize.
  //
  // Returns the pipeline write Result. Callers should propagate the Result.
  [[nodiscard]] channel_pipeline::Result writeResponse(
      uint32_t streamId,
      std::unique_ptr<folly::IOBuf> data,
      std::unique_ptr<apache::thrift::ResponseRpcMetadata> metadata) noexcept;

  // Sends an ERROR frame (framework-level error: bad metadata, unknown
  // method, parse failure, etc.). Distinct from writeResponse because the
  // wire shape differs: no payload metadata, the rocket error code is set on
  // the frame itself. Caller pre-serializes the error body via
  // serializeResponseRpcError (which also returns the matching ErrorCode).
  //
  [[nodiscard]] channel_pipeline::Result writeError(
      uint32_t streamId,
      std::unique_ptr<folly::IOBuf> errorData,
      apache::thrift::fast_thrift::frame::ErrorCode errorCode) noexcept;

  // === Codegen composition helpers ===
  //
  // Compose the writeResponse / writeError primitives above with the util/
  // serializer + metadata pieces into the four wire shapes the server emits
  // per RPC outcome:
  //   success                  → writeSuccessResponse   (PAYLOAD)
  //   declared exception       → writeDeclaredException (PAYLOAD)
  //   undeclared exception     → writeUnknownException  (PAYLOAD)
  //   framework dispatch error → writeFrameworkError    (ERROR)

  // Success: serialize presult into a payload buffer, build success
  // metadata, fire as PAYLOAD (errorCode=0, complete=true).
  template <typename Writer, typename Presult>
  [[nodiscard]] channel_pipeline::Result writeSuccessResponse(
      uint32_t streamId, const Presult& presult) noexcept {
    auto data = serializeResponse<Writer>(
        [&](Writer& w) { presult.write(&w); },
        [&](Writer& w) { return presult.serializedSizeZC(&w); });
    auto md = std::make_unique<apache::thrift::ResponseRpcMetadata>();
    fillSuccessResponseMetadata(*md);
    return writeResponse(streamId, std::move(data), std::move(md));
  }

  // Declared exception: caller has already populated the matching presult
  // slot via apache::thrift::detail::ap::insert_exn. We serialize the
  // presult, build declared-exception metadata (name/what from `ew`,
  // classification from codegen via getDeclaredExceptionClassification<Ex>),
  // and fire as PAYLOAD. Same wire family as writeSuccessResponse —
  // distinguished only by the metadata payloadMetadata variant
  // (declaredException vs responseMetadata).
  template <typename Writer, typename Presult>
  [[nodiscard]] channel_pipeline::Result writeDeclaredException(
      uint32_t streamId,
      const Presult& presult,
      const folly::exception_wrapper& ew,
      std::optional<apache::thrift::ErrorClassification>
          classification) noexcept {
    auto data = serializeResponse<Writer>(
        [&](Writer& w) { presult.write(&w); },
        [&](Writer& w) { return presult.serializedSizeZC(&w); });
    auto md = std::make_unique<apache::thrift::ResponseRpcMetadata>();
    fillDeclaredExceptionMetadata(
        *md,
        ew.class_name().toStdString(),
        ew.what().toStdString(),
        classification);
    return writeResponse(streamId, std::move(data), std::move(md));
  }

  // Undeclared exception (cascade fall-through): null data +
  // appUnknownException metadata carrying name/what/blame. PAYLOAD frame
  // because the request was valid and dispatched correctly — the exception
  // came from user code, not the framework. Client surfaces this as
  // TApplicationException.
  [[nodiscard]] channel_pipeline::Result writeUnknownException(
      uint32_t streamId,
      const folly::exception_wrapper& ew,
      apache::thrift::ErrorBlame blame =
          apache::thrift::ErrorBlame::SERVER) noexcept;

  // Framework dispatch error: the framework couldn't dispatch the request
  // (parse failure, wrong RPC kind, unknown method). Serializes a
  // ResponseRpcError body and fires as ERROR with the matching rocket error
  // code on the frame itself. Distinct wire shape from the three PAYLOAD
  // variants above — clients route ERROR through decodeErrorFrame().
  [[nodiscard]] channel_pipeline::Result writeFrameworkError(
      uint32_t streamId,
      apache::thrift::ResponseRpcErrorCode code,
      std::string message) noexcept;

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

  // Pushes a stream-0 ERROR(CONNECTION_CLOSE) through the pipeline (rsocket-
  // spec signal). Wire-only; no state mutation. Returns Error if the
  // pipeline is unwired or the write fails — caller should treat that as
  // the connection being already dead.
  [[nodiscard]] channel_pipeline::Result sendConnectionCloseErr() noexcept;

  [[nodiscard]] channel_pipeline::Result fireResponse(
      ThriftServerResponseMessage&& response) noexcept;
};

} // namespace apache::thrift::fast_thrift::thrift
