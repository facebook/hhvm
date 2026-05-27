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

#include <chrono>
#include <memory>
#include <optional>
#include <utility>

#include <folly/ExceptionWrapper.h>
#include <folly/SocketAddress.h>
#include <folly/container/F14Map.h>
#include <folly/io/IOBuf.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/AsyncSocketException.h>
#include <folly/io/async/AsyncTransport.h>
#include <folly/io/async/DelayedDestruction.h>
#include <folly/logging/xlog.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineImpl.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/detail/ContextImpl.h>
#include <thrift/lib/cpp2/fast_thrift/connection/security/common/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/security/HandshakeTimeout.h>
#include <thrift/lib/cpp2/fast_thrift/security/TLSPrefixPeeker.h>

namespace apache::thrift::fast_thrift::connection::security::handler {

/**
 * Inner-pipeline middle handler used in SSLPolicy::PERMITTED mode; always
 * the first handler after ConnectionTLSHandler's head role. Peeks the
 * leading bytes of every accepted connection to classify it as TLS or
 * plaintext, then routes:
 *
 *   TLS-looking → ctx.fireRead(msg)
 *       Flows to the next inner handler (FizzHandshakeHandler).
 *
 *   Plaintext   → ctx.pipeline()->fireReadToTailHandler(msg)
 *       Bypasses Fizz + StopTLS and lands directly at the inner pipeline's
 *       tail (ConnectionTLSHandler in its tail role), which forwards the
 *       raw socket back out to the outer pipeline.
 *
 * Before routing, setPreReceivedData() is called on the AsyncSocket so the
 * peeked bytes are replayed to whoever reads next (Fizz ClientHello parser
 * for TLS, application protocol for plaintext).
 *
 * Peek errors and timeouts are logged at DBG3 and the connection is
 * dropped — matches FizzHandshakeHandler error policy.
 */
class TLSClassifier {
 public:
  // handshakeTimeout is the per-connection deadline shared with the
  // downstream Fizz handler. std::nullopt = unbounded. A fresh
  // HandshakeTimeout is constructed per connection in onRead so each one
  // captures its own absolute deadline at peek start.
  explicit TLSClassifier(
      std::optional<std::chrono::milliseconds> handshakeTimeout) noexcept
      : handshakeTimeout_(handshakeTimeout) {}

  ~TLSClassifier() = default;
  TLSClassifier(const TLSClassifier&) = delete;
  TLSClassifier& operator=(const TLSClassifier&) = delete;
  TLSClassifier(TLSClassifier&&) = delete;
  TLSClassifier& operator=(TLSClassifier&&) = delete;

  template <typename Context>
  void handlerAdded(Context& ctx) noexcept {
    ctx_ = &ctx;
  }

  template <typename Context>
  void handlerRemoved(Context& /*ctx*/) noexcept {
    ctx_ = nullptr;
  }

  // === Inbound ===

  template <typename Context>
  channel_pipeline::Result onRead(
      Context& ctx, channel_pipeline::TypeErasedBox&& msg) noexcept {
    auto incoming = msg.take<TLSPipelineMessage>();

    // We expect a plain AsyncSocket — anything else (already-negotiated
    // transport) is forwarded as-is.
    auto* asyncSocket =
        dynamic_cast<folly::AsyncSocket*>(incoming.transport.get());
    if (FOLLY_UNLIKELY(asyncSocket == nullptr)) {
      return ctx.fireRead(channel_pipeline::erase_and_box(std::move(incoming)));
    }

    (void)incoming.transport.release();
    folly::AsyncSocket::UniquePtr socket(asyncSocket);
    auto clientAddr = std::move(incoming.clientAddr);

    apache::thrift::fast_thrift::security::TLSPrefixPeeker::UniquePtr peeker(
        new apache::thrift::fast_thrift::security::TLSPrefixPeeker(
            std::move(socket),
            apache::thrift::fast_thrift::security::HandshakeTimeout{
                handshakeTimeout_},
            [this](
                apache::thrift::fast_thrift::security::TLSPrefixPeeker*
                    p) noexcept { inFlight_.erase(p); },
            [this, clientAddr](
                folly::AsyncSocket::UniquePtr peekedSocket,
                std::unique_ptr<folly::IOBuf> peekedBytes,
                bool looksLikeTLS,
                const folly::exception_wrapper& ex) noexcept {
              onPeekComplete(
                  std::move(peekedSocket),
                  std::move(peekedBytes),
                  looksLikeTLS,
                  ex,
                  clientAddr);
            }));
    auto* raw = peeker.get();
    inFlight_.emplace(raw, std::move(peeker));
    raw->start();

    // Message absorbed; downstream handlers see it again only on peek success.
    return channel_pipeline::Result::Success;
  }

  template <typename Context>
  void onException(Context& ctx, folly::exception_wrapper&& e) noexcept {
    ctx.fireException(std::move(e));
  }

  template <typename Context>
  void onPipelineActive(Context& /*ctx*/) noexcept {}

  template <typename Context>
  void onReadReady(Context& /*ctx*/) noexcept {}

  // === Outbound (passthrough) ===

  template <typename Context>
  channel_pipeline::Result onWrite(
      Context& ctx, channel_pipeline::TypeErasedBox&& msg) noexcept {
    return ctx.fireWrite(std::move(msg));
  }

  template <typename Context>
  void onPipelineInactive(Context& /*ctx*/) noexcept {
    // Drain into a local map so each peeker's synchronous self-remove via
    // onTerminal (inFlight_.erase) doesn't perturb iteration. After the
    // loop the local map's UniquePtr deleters call each peeker's destroy().
    auto drained = std::move(inFlight_);
    for (auto& [_, peeker] : drained) {
      folly::DelayedDestruction::DestructorGuard guard(peeker.get());
      peeker->cancel();
    }
  }

  template <typename Context>
  void onWriteReady(Context& /*ctx*/) noexcept {}

 private:
  void onPeekComplete(
      folly::AsyncSocket::UniquePtr socket,
      std::unique_ptr<folly::IOBuf> peekedBytes,
      bool looksLikeTLS,
      const folly::exception_wrapper& ex,
      const folly::SocketAddress& clientAddr) noexcept {
    if (ex || !socket) {
      XLOG(DBG3) << "TLS peek failed for " << clientAddr.describe() << ": "
                 << (ex ? ex.what().toStdString() : std::string("null"));
      return;
    }
    if (FOLLY_UNLIKELY(!ctx_)) {
      // handlerRemoved fired before this callback (pipeline tearing down).
      return;
    }

    // Replay the peeked bytes so whoever reads next (Fizz for TLS, app
    // protocol for plaintext) sees them as if they were freshly received.
    if (peekedBytes && peekedBytes->length() > 0) {
      socket->setPreReceivedData(std::move(peekedBytes));
    }

    TLSPipelineMessage forwarded{
        .transport = folly::AsyncTransport::UniquePtr(socket.release()),
        .clientAddr = clientAddr,
        .extension = nullptr,
    };

    auto result = looksLikeTLS
        ? ctx_->fireRead(channel_pipeline::erase_and_box(std::move(forwarded)))
        : ctx_->pipeline()->fireReadToTailHandler(
              channel_pipeline::erase_and_box(std::move(forwarded)));

    switch (result) {
      case channel_pipeline::Result::Success:
        return;
      case channel_pipeline::Result::Backpressure:
        XLOG_EVERY_MS(WARN, 1000)
            << (looksLikeTLS ? "Downstream backpressure routing TLS connection "
                               "from "
                             : "Downstream backpressure routing plaintext "
                               "connection from ")
            << clientAddr.describe();
        return;
      case channel_pipeline::Result::Error:
        XLOG(WARN) << "Downstream rejected "
                   << (looksLikeTLS ? "TLS-bound" : "plaintext")
                   << " connection from " << clientAddr.describe();
        return;
    }
  }

  std::optional<std::chrono::milliseconds> handshakeTimeout_;
  folly::F14FastMap<
      apache::thrift::fast_thrift::security::TLSPrefixPeeker*,
      apache::thrift::fast_thrift::security::TLSPrefixPeeker::UniquePtr>
      inFlight_;
  channel_pipeline::detail::ContextImpl* ctx_{nullptr};
};

} // namespace apache::thrift::fast_thrift::connection::security::handler
