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

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <utility>

#include <folly/io/IOBuf.h>
#include <folly/logging/xlog.h>

#include <thrift/lib/cpp2/fast_thrift/frame/ErrorCode.h>
#include <thrift/lib/cpp2/fast_thrift/frame/FrameType.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/ComposedFrame.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/MetadataProtocol.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/context/ThriftConnContext.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::fast_thrift::thrift {

/**
 * A refusal of the setup exchange: the frame-layer code to close with, and why.
 * The reason reaches the client in the ERROR frame's body, so a refusal says
 * what went wrong rather than arriving bare.
 */
struct SetupRejection {
  apache::thrift::fast_thrift::frame::ErrorCode code;
  std::string reason;
};

/**
 * The connection setup exchange, as the thrift pipeline sees it.
 *
 * Held behind a pointer because the payload must stay pointer-sized: the
 * inbound variant is inline storage on the per-request message, so carrying
 * this by value would grow every request by the size of a
 * RequestSetupMetadata.
 *
 * What the server will answer with is not here — that is a separate outbound
 * message, and handlers stamp their part of it as it passes them on the way
 * out. The one thing a handler decides on this side is whether there is going
 * to be an answer at all.
 */
struct ConnectionSetupData {
  // Non-owning. The connection-context handler owns the context for the
  // pipeline's lifetime, which outlives the setup exchange. Null when the
  // server runs without a per-connection context.
  //
  // Mutable: a handler that resolves something once per connection stamps it
  // here for everything downstream to read.
  ThriftConnContext* connContext{nullptr};
  apache::thrift::RequestSetupMetadata clientSetup;

  // Set to refuse the connection. The terminus answers with this instead of a
  // SETUP response, so a refusing handler forwards the message like any other
  // and does not write anything itself.
  std::optional<SetupRejection> reject;
};

namespace detail {
// The setup payload is inbound-only — its answer is a separate outbound
// message — but the payload variant dispatches toRocketFrame uniformly across
// its alternatives, so it has to declare one.
[[noreturn]] inline apache::thrift::fast_thrift::frame::ComposedFrame
noRocketFrameForLifecyclePayload() {
  XLOG(FATAL) << "connection-lifecycle payloads are never serialized outbound";
}
} // namespace detail

/**
 * Inbound: the client's SETUP frame has been parsed and version-negotiated,
 * and the server has not yet answered it.
 *
 * Travels head→tail like a request, so handlers see it in registration order.
 * A handler inspects the client's setup and may refuse the connection by
 * setting `reject`; either way it forwards. ThriftServerSetupHandler is its
 * terminus and turns whichever outcome into the outbound answer.
 */
struct ThriftConnectionSetupPayload {
  static constexpr apache::thrift::RpcKind kRpcKind =
      apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE;
  using RocketFrame = apache::thrift::fast_thrift::frame::ComposedFrame;

  std::unique_ptr<ConnectionSetupData> setup;

  RocketFrame toRocketFrame(rocket::server::MetadataProtocol) && {
    detail::noRocketFrameForLifecyclePayload();
  }
  const apache::thrift::RequestRpcMetadata* getRequestRpcMetadata()
      const noexcept {
    return nullptr;
  }
};

/**
 * Outbound: the answer to the setup exchange, on its way to the client as a
 * connection-level METADATA_PUSH.
 *
 * Carried structured rather than serialized so it stays open to contribution:
 * the terminus fills the fields the server negotiated, and handlers stamp the
 * fields they own as it passes them on the way out. It becomes bytes only at
 * toRocketFrame, once nobody can add to it any more.
 */
struct ThriftSetupResponsePayload {
  using RocketFrame = apache::thrift::fast_thrift::frame::ComposedFrame;

  std::unique_ptr<apache::thrift::SetupResponse> response;

  RocketFrame toRocketFrame(rocket::server::MetadataProtocol) && noexcept;

  const apache::thrift::ResponseRpcMetadata* getResponseRpcMetadata()
      const noexcept {
    return nullptr;
  }
};

/**
 * Outbound: the connection setup was refused. Distinct from ThriftErrorPayload
 * because a refused connection is not a failed request — nothing counted it
 * inbound, so counting it as a response would unbalance the active-request
 * gauge. Same stream-0 ERROR frame on the wire.
 */
struct ThriftSetupRejectionPayload {
  using RocketFrame = apache::thrift::fast_thrift::frame::ComposedFrame;

  std::unique_ptr<folly::IOBuf> reason;
  uint32_t errorCode{0};

  RocketFrame toRocketFrame(rocket::server::MetadataProtocol) && noexcept {
    return {
        .frameType = apache::thrift::fast_thrift::frame::FrameType::ERROR,
        .streamId = 0,
        .metadata = nullptr,
        .data = std::move(reason),
        .errorCode = errorCode,
    };
  }
  const apache::thrift::ResponseRpcMetadata* getResponseRpcMetadata()
      const noexcept {
    return nullptr;
  }
};

} // namespace apache::thrift::fast_thrift::thrift
