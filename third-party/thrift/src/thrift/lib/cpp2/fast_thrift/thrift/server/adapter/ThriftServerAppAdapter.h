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

#include <folly/ExceptionWrapper.h>
#include <folly/Synchronized.h>
#include <folly/container/F14Map.h>
#include <folly/io/IOBuf.h>
#include <folly/io/async/DelayedDestruction.h>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineImpl.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/frame/ErrorCode.h>
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
 * Satisfies TailEndpointHandler concept (onRead / onException).
 */
class ThriftServerAppAdapter : public folly::DelayedDestruction {
 public:
  // Per-method handler signature for SINGLE_REQUEST_SINGLE_RESPONSE RPCs.
  // The dispatcher pre-extracts the components codegen actually needs from
  // the inbound frame so the handler doesn't have to reach back into
  // ThriftServerRequestMessage / ParsedFrame.
  //   streamId — Rocket stream id; echo into the response so the client can
  //              match it to the in-flight request.
  //   data     — request payload IOBuf, ready to feed into a protocol reader
  //              for argument deserialization.
  //   protocol — wire protocol id (Binary / Compact / ...) for the codec.
  using RequestResponseProcessFn = channel_pipeline::Result (*)(
      ThriftServerAppAdapter*,
      uint32_t streamId,
      std::unique_ptr<folly::IOBuf> data,
      apache::thrift::ProtocolId protocol) noexcept;

  ThriftServerAppAdapter() = default;
  ThriftServerAppAdapter(ThriftServerAppAdapter&&) = delete;
  ThriftServerAppAdapter& operator=(ThriftServerAppAdapter&&) = delete;

  void setCloseCallback(std::function<void()> cb);

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
  void onPipelineActive() noexcept {}
  void onPipelineInactive() noexcept {}
  void onWriteReady() noexcept {}

  // Sends a PAYLOAD frame. Caller (codegen / framework) is responsible for
  // building the data buffer (serialized presult, or nullptr) and metadata
  // buffer (success / declared exception / app error metadata) — the adapter
  // only frames them onto the pipeline.
  //
  // Returns the pipeline write Result. On Error, the pipeline is closed
  // before returning. Synchronous callers should propagate; async callers
  // (callbacks) may discard the result since the close has already happened.
  [[nodiscard]] channel_pipeline::Result writeResponse(
      uint32_t streamId,
      std::unique_ptr<folly::IOBuf> data,
      std::unique_ptr<folly::IOBuf> metadata,
      bool complete) noexcept;

  // Sends an ERROR frame (framework-level error: bad metadata, unknown
  // method, parse failure, etc.). Distinct from writeResponse because the
  // wire shape differs: no payload metadata, the rocket error code is set on
  // the frame itself. Caller pre-serializes the error body via
  // serializeResponseRpcError (which also returns the matching ErrorCode).
  //
  // Same Error-handling contract as writeResponse: pipeline closes on Error.
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

  // Success: serialize presult into a payload buffer, attach the cached
  // default success metadata, fire as PAYLOAD (errorCode=0, complete=true).
  template <typename Writer, typename Presult>
  [[nodiscard]] channel_pipeline::Result writeSuccessResponse(
      uint32_t streamId, const Presult& presult) noexcept {
    auto data = serializeResponse<Writer>(
        [&](Writer& w) { presult.write(&w); },
        [&](Writer& w) { return presult.serializedSizeZC(&w); });
    return writeResponse(
        streamId,
        std::move(data),
        getDefaultSuccessMetadata(),
        /*complete=*/true);
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
    auto md = makeDeclaredExceptionMetadata(
        ew.class_name().toStdString(),
        ew.what().toStdString(),
        std::move(classification));
    return writeResponse(
        streamId, std::move(data), std::move(md), /*complete=*/true);
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

 protected:
  void addMethodHandler(
      std::string_view name, RequestResponseProcessFn handler);

  channel_pipeline::Result handleRequestResponse(
      ThriftServerRequestMessage&& request) noexcept;

  channel_pipeline::PipelineImpl* pipeline_{nullptr};

  ~ThriftServerAppAdapter() override;

  folly::EventBase* getEventBase() const { return evb_; }

 private:
  folly::EventBase* evb_{nullptr};
  folly::F14FastMap<std::string, RequestResponseProcessFn> dispatch_;
  folly::Synchronized<std::function<void()>> closeCallback_;

  [[nodiscard]] channel_pipeline::Result fireResponse(
      ThriftServerResponseMessage&& response) noexcept;
};

} // namespace apache::thrift::fast_thrift::thrift
