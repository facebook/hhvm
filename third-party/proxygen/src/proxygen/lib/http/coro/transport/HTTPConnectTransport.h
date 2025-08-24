/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/coro/BoundedQueue.h>
#include <folly/io/coro/Transport.h>

#include "proxygen/lib/http/coro/transport/HTTPConnectStream.h"

namespace proxygen::coro {

/**
 * This class can be used as a folly::coro::Transport that is built over
 * an HTTP CONNECT tunnel (usually via a forward proxy).  Do not use this
 * directly for establishing an HTTPCoroSession via the proxy (eg, end-to-end
 * HTTPS). For that use HTTPClient::getHTTPSessionViaProxy();
 *
 * Example:
 *
 *  auto proxySession = co_await HTTPClient::getHTTPSession(
 *   evb, proxyHost, proxyPort, useTls, useQuic,
 *   connectTimeout, streamTimeout);
 *  auto reservation = proxySession->reserveRequest();
 *  if (!reservation) {
 *    // error
 *  }
 *  auto connectStream = co_await HTTPConnectStream::connectUnique(
 *    proxySession, std::move(*reservation), destinationHostAndPort,
 *    connectTimeout);
 *  auto transport = std::make_shared<HTTPConnectTransport>(
 *    std::move(connectStream));
 */
class HTTPConnectTransport
    : public folly::coro::TransportIf
    , public HTTPStreamSource::Callback {
 public:
  explicit HTTPConnectTransport(
      std::unique_ptr<HTTPConnectStream> connectStream);

  ~HTTPConnectTransport() override;

  /* TransportIf overrides */
  folly::EventBase* getEventBase() noexcept override {
    return connectStream_->eventBase_;
  }

  folly::coro::Task<size_t> read(folly::MutableByteRange buf,
                                 std::chrono::milliseconds timeout =
                                     std::chrono::milliseconds(0)) override;

  folly::coro::Task<size_t> read(folly::IOBufQueue& buf,
                                 size_t minReadSize,
                                 size_t newAllocationSize,
                                 std::chrono::milliseconds timeout =
                                     std::chrono::milliseconds(0)) override;

  folly::coro::Task<folly::Unit> write(
      folly::ByteRange buf,
      std::chrono::milliseconds timeout = std::chrono::milliseconds(0),
      folly::WriteFlags writeFlags = folly::WriteFlags::NONE,
      WriteInfo* writeInfo = nullptr) override;
  folly::coro::Task<folly::Unit> write(
      folly::IOBufQueue& ioBufQueue,
      std::chrono::milliseconds timeout = std::chrono::milliseconds(0),
      folly::WriteFlags writeFlags = folly::WriteFlags::NONE,
      WriteInfo* writeInfo = nullptr) override;

  folly::SocketAddress getLocalAddress() const noexcept override {
    return connectStream_->localAddr_;
  }

  folly::SocketAddress getPeerAddress() const noexcept override {
    return connectStream_->peerAddr_;
  }

  void close() override;
  void shutdownWrite() override;
  void closeWithReset() override;
  folly::AsyncTransport* getTransport() const override {
    // This is used for TCP socket info
    return nullptr;
  }
  const folly::AsyncTransportCertificate* getPeerCertificate() const override {
    // TODO: HTTPCoroSession needs to expose this?
    return nullptr;
  }

 private:
  void scheduleAsyncRead(uint32_t size, const folly::CancellationToken& ct);
  /* HTTPStreamSource::Callback overrides */
  void windowOpen(HTTPCodec::StreamID id) override;

  std::unique_ptr<HTTPConnectStream> connectStream_;
  TimedBaton flowControlWindowOpen_;
  std::shared_ptr<bool> deleted_{std::make_shared<bool>(false)};
  static constexpr uint8_t kMaxEvents{2};
  folly::coro::BoundedQueue<folly::Try<HTTPBodyEvent>,
                            /*SingleProducer=*/true,
                            /*SingleConsumer=*/true>
      bodyEvents_{kMaxEvents};
  bool writeInProgress_{false};
  bool pendingEOM_{false};
  bool pendingRead_{false};
};

} // namespace proxygen::coro
