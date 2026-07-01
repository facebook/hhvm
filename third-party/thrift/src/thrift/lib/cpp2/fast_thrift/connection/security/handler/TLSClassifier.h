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

namespace fast_security = ::apache::thrift::fast_thrift::security;

/**
 * Inner-pipeline middle handler used in SSLPolicy::PERMITTED mode. Peeks
 * the first 9 bytes of every accepted connection to classify it as TLS or
 * plaintext, then routes:
 *
 *   TLS-looking → ctx.fireWrite(msg)
 *       Flows to the next stage on the work path (FizzHandshakeHandler).
 *
 *   Plaintext   → ctx.pipeline()->fireWriteToHeadHandler(msg)
 *       Bypasses Fizz + StopTLS and lands directly at the head (TLSFinalizer),
 *       which turns the raw socket around onto the read path for handoff.
 *
 * Before routing, setPreReceivedData() is called on the AsyncSocket so the
 * peeked bytes are replayed to whoever reads next (Fizz ClientHello parser
 * for TLS, application protocol for plaintext).
 *
 * Reads the per-connection handshake-timeout budget off incoming
 * msg.tlsParams (stamped upstream by TLSConfigHandler). No Observer
 * dependency lives here.
 *
 * Peek errors and timeouts are logged at DBG3 and the connection is
 * dropped — matches FizzHandshakeHandler error policy.
 */
class TLSClassifier {
 public:
  TLSClassifier() = default;

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

  // === Outbound (work path) ===

  template <typename Context>
  channel_pipeline::Result onWrite(
      Context& ctx, channel_pipeline::TypeErasedBox&& msg) noexcept {
    auto incoming = msg.take<TLSRequestMessage>();

    auto* asyncSocket =
        dynamic_cast<folly::AsyncSocket*>(incoming.transport.get());
    if (FOLLY_UNLIKELY(asyncSocket == nullptr)) {
      return ctx.fireWrite(
          channel_pipeline::erase_and_box(std::move(incoming)));
    }

    (void)incoming.transport.release();
    folly::AsyncSocket::UniquePtr socket(asyncSocket);
    auto clientAddr = std::move(incoming.clientAddr);
    auto handshakeTimeout = incoming.tlsParams
        ? incoming.tlsParams->handshakeTimeout
        : std::nullopt;

    fast_security::TLSPrefixPeeker::UniquePtr peeker(
        new fast_security::TLSPrefixPeeker(
            std::move(socket),
            fast_security::HandshakeTimeout{handshakeTimeout},
            [this](fast_security::TLSPrefixPeeker* p) noexcept {
              inFlight_.erase(p);
            },
            [this, clientAddr, params = std::move(incoming.tlsParams)](
                folly::AsyncSocket::UniquePtr peekedSocket,
                std::unique_ptr<folly::IOBuf> peekedBytes,
                bool looksLikeTLS,
                const folly::exception_wrapper& ex) mutable noexcept {
              onPeekComplete(
                  std::move(peekedSocket),
                  std::move(peekedBytes),
                  looksLikeTLS,
                  ex,
                  clientAddr,
                  std::move(params));
            }));
    auto* raw = peeker.get();
    inFlight_.emplace(raw, std::move(peeker));
    raw->start();

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

  // === Inbound (passthrough) ===

  template <typename Context>
  channel_pipeline::Result onRead(
      Context& ctx, channel_pipeline::TypeErasedBox&& msg) noexcept {
    return ctx.fireRead(std::move(msg));
  }

  template <typename Context>
  void onPipelineInactive(Context& /*ctx*/) noexcept {
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
      const folly::SocketAddress& clientAddr,
      std::shared_ptr<const fast_security::TLSParams> tlsParams) noexcept {
    if (ex || !socket) {
      XLOG(DBG3) << "TLS peek failed for " << clientAddr.describe() << ": "
                 << (ex ? ex.what().toStdString() : std::string("null"));
      return;
    }
    if (FOLLY_UNLIKELY(!ctx_)) {
      return;
    }

    if (peekedBytes && peekedBytes->length() > 0) {
      socket->setPreReceivedData(std::move(peekedBytes));
    }

    TLSRequestMessage forwarded{
        .transport = folly::AsyncTransport::UniquePtr(socket.release()),
        .clientAddr = clientAddr,
        .tlsParams = std::move(tlsParams),
        .extension = nullptr,
    };

    auto result = looksLikeTLS
        ? ctx_->fireWrite(channel_pipeline::erase_and_box(std::move(forwarded)))
        : ctx_->pipeline()->fireWriteToHeadHandler(
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

  folly::F14FastMap<
      fast_security::TLSPrefixPeeker*,
      fast_security::TLSPrefixPeeker::UniquePtr>
      inFlight_;
  channel_pipeline::detail::ContextImpl* ctx_{nullptr};
};

} // namespace apache::thrift::fast_thrift::connection::security::handler
