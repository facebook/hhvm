/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/io/coro/Transport.h>

#include "proxygen/lib/http/coro/HTTPCoroSession.h"
#include "proxygen/lib/http/coro/HTTPStreamSource.h"

namespace proxygen::coro {

/**
 * HTTPConnectStream represents an HTTP CONNECT tunnel built over an
 * HTTPCoroSession.  It works with all versions of HTTP according to their
 * CONNECT semantics.
 *
 * This object is used in conjunction with a transport.  There are two such
 * transports: HTTPConnectTransport (folly::coro::Transport) and
 * HTTPConnectAsyncTransport (folly::AsyncTransport).
 *
 * There are two APIs to establish a CONNECT tunnel
 *
 *  - connect, which can be used when another object (eg: HTTPCoroSessionPool)
 *    owns and manages the underlying HTTPCoroSession.  This allows multiplexing
 *    connect streams over protocols like HTTP/2 and HTTP/3.
 *
 *  - connectUnique, which will take ownership of the underlying
 *    HTTPCoroSession and be responsible for its destruction when the tunnel is
 *    done
 */
class HTTPConnectStream
    : public HTTPStreamSource::Callback
    , public LifecycleObserver {
 public:
  using RequestHeaderMap = std::map<std::string, std::string>;

  /* Establish a CONNECT tunnel on the session to the given authority.
   * It is assumed that session is owned by some other entity
   * (eg: HTTPCoroSessionPool).
   */
  static folly::coro::Task<std::unique_ptr<HTTPConnectStream>> connect(
      HTTPCoroSession* session,
      HTTPCoroSession::RequestReservation reservation,
      std::string authority,
      std::chrono::milliseconds timeout,
      RequestHeaderMap connectHeaders = RequestHeaderMap(),
      size_t egressBufferSize = 256 * 1024);

  /* Establish a CONNECT tunnel on the session to the given authority.
   * Takes ownership of the session and initiate closure when the close is
   * called.
   */
  static folly::coro::Task<std::unique_ptr<HTTPConnectStream>> connectUnique(
      HTTPCoroSession* session,
      HTTPCoroSession::RequestReservation reservation,
      std::string authority,
      std::chrono::milliseconds timeout,
      RequestHeaderMap connectHeaders = RequestHeaderMap(),
      size_t egressBufferSize = 256 * 1024);

  ~HTTPConnectStream() override;

  void setHTTPStreamSourceCallback(HTTPStreamSource::Callback* callback) {
    callback_ = callback;
  }

  void close();

  bool canRead() const;
  bool canWrite() const;
  void shutdownRead();
  void shutdownWrite();

  HTTPCoroSession* session_{nullptr};
  folly::EventBase* eventBase_;
  size_t egressBufferSize_;
  HTTPStreamSource* egressSource_{nullptr};
  std::shared_ptr<HTTPSourceHolder> ingressSource_;
  folly::SocketAddress localAddr_;
  folly::SocketAddress peerAddr_;
  folly::Optional<HTTPError> egressError_;

 private:
  enum class Ownership { Unique, Shared };
  HTTPConnectStream(Ownership ownership,
                    HTTPCoroSession* session,
                    RequestHeaderMap connectHeaders,
                    size_t egressBufferSize);

  folly::coro::Task<void> connectImpl(
      HTTPCoroSession* session,
      HTTPCoroSession::RequestReservation reservation,
      std::string authority,
      std::chrono::milliseconds timeout);

  /* HTTPCoroSession::InfoCallback overrides */
  void onDestroy(const HTTPCoroSession& /*sess*/) override {
    session_ = nullptr;
  }

  /* HTTPStreamSource::Callback overrides */
  void bytesProcessed(HTTPCodec::StreamID id,
                      size_t amount,
                      size_t toAck) override;
  void windowOpen(HTTPCodec::StreamID id) override;
  void sourceComplete(HTTPCodec::StreamID id,
                      folly::Optional<HTTPError> error) override;

  RequestHeaderMap connectHeaders_;
  HTTPStreamSource::Callback* callback_{nullptr};
};

} // namespace proxygen::coro
