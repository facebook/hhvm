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
#include <utility>

#include <folly/ExceptionWrapper.h>
#include <folly/lang/Hint.h>
#include <folly/logging/xlog.h>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/frame/ErrorCode.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/SetupResponseBuilder.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/ConnectionPayloads.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/Event.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/util/SetupMessages.h>

namespace apache::thrift::fast_thrift::thrift {

/**
 * ThriftServerSetupHandler — the boundary between the connection-setup
 * protocol and the application.
 *
 * Sits immediately above the tail adapter, downstream of every other handler.
 * Two consequences: the application tail only ever sees requests, and by the
 * time the setup message arrives here every handler upstream has had its say.
 * It is the terminus of the setup message — nothing below needs to see it.
 *
 * On the setup message it negotiates the thrift protocol version against the
 * client's advertised range and writes the answer back outbound. Only the
 * fields the server itself negotiated are filled here; the rest of the
 * response is open for handlers to stamp as it passes them on the way out.
 *
 * It is also where a refusal becomes an answer: a client whose range does not
 * overlap the server's is refused here, as is one that a handler upstream
 * rejected by setting ConnectionSetupData::reject.
 *
 * A refused connection never reaches this handler: a handler that refuses
 * writes the error outbound and does not forward, so the message stops
 * upstream. The one refusal this handler originates is its own serialization
 * failure.
 */
template <typename Context>
class ThriftServerSetupHandler {
 public:
  // HandlerLifecycle
  void handlerAdded(Context& /*ctx*/) noexcept {}
  void handlerRemoved(Context& /*ctx*/) noexcept {}
  void onPipelineActive(Context& /*ctx*/) noexcept {}
  void onPipelineInactive(Context& /*ctx*/) noexcept {}
  void onReadReady(Context& /*ctx*/) noexcept {}
  void onWriteReady(Context& /*ctx*/) noexcept {}

  channel_pipeline::Result onRead(
      Context& ctx, channel_pipeline::TypeErasedBox&& msg) noexcept {
    auto& request = msg.get<ThriftServerRequestMessage>();

    if (FOLLY_UNLIKELY(request.payload.is<ThriftConnectionSetupPayload>())) {
      return answerSetup(
          ctx, *request.payload.get<ThriftConnectionSetupPayload>().setup);
    }
    return ctx.fireRead(std::move(msg));
  }

  channel_pipeline::Result onWrite(
      Context& ctx, channel_pipeline::TypeErasedBox&& msg) noexcept {
    return ctx.fireWrite(std::move(msg));
  }

  void onException(Context& ctx, folly::exception_wrapper&& e) noexcept {
    ctx.fireException(std::move(e));
  }

 private:
  channel_pipeline::Result answerSetup(
      Context& ctx, ConnectionSetupData& setup) noexcept {
    // A handler upstream already decided against this connection; its reason
    // is the answer.
    if (FOLLY_UNLIKELY(setup.reject.has_value())) {
      return refuseSetup(ctx, setup.reject->code, setup.reject->reason);
    }

    const auto negotiatedVersion = negotiateVersion(setup.clientSetup);
    if (FOLLY_UNLIKELY(!negotiatedVersion.hasValue())) {
      return refuseSetup(
          ctx,
          negotiatedVersion.error(),
          "Could not negotiate the client's SETUP metadata");
    }

    // Only what the server itself negotiated. The fields handlers own are
    // stamped on the way out, so nothing here can be disturbed and nothing
    // here waits on them.
    auto response = std::make_unique<apache::thrift::SetupResponse>();
    response->version() = *negotiatedVersion;
    // fast_thrift has no compression codec yet, so advertising support would
    // be untruthful. Flip once per-request compression lands.
    response->zstdSupported() = false;

    const auto writeResult = ctx.fireWrite(
        channel_pipeline::erase_and_box(
            makeSetupResponseMessage(std::move(response))));
    // Only a hard failure matters here: it leaves the client waiting on an
    // answer that can never arrive. Backpressure is not a refusal and not a
    // throttle request either — the transport reports it for any write still
    // in flight, so relaying it as a read result would pause reads behind
    // every response.
    if (FOLLY_UNLIKELY(writeResult == channel_pipeline::Result::Error)) {
      XLOG(WARN) << "Failed to deliver the SETUP response";
      return channel_pipeline::Result::Error;
    }
    return channel_pipeline::Result::Success;
  }

  // Answers the setup exchange with a refusal. The refusal rides the same
  // terminal path as any other close, so the connection close handler owns
  // writing the frame and sequencing the teardown behind it — both run
  // synchronously here, before this returns.
  //
  // Error, not Success: the layer below reads a successful setup as the
  // exchange having completed and announces the connection as established.
  // A refused connection must never be announced.
  channel_pipeline::Result refuseSetup(
      Context& ctx,
      apache::thrift::fast_thrift::frame::ErrorCode errorCode,
      std::string_view reason) noexcept {
    ctx.fireEvent(
        ThriftServerEventType::CloseConnection,
        channel_pipeline::TypeErasedBox(
            ThriftServerCloseConnectionEvent{
                .rejection = SetupRejection{errorCode, std::string(reason)}}));
    return channel_pipeline::Result::Error;
  }
};

} // namespace apache::thrift::fast_thrift::thrift
