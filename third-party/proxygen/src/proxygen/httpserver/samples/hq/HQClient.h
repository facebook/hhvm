/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <list>
#include <memory>
#include <proxygen/httpclient/samples/curl/CurlClient.h>
#include <proxygen/httpserver/samples/hq/H1QUpstreamSession.h>
#include <proxygen/httpserver/samples/hq/HQCommandLine.h>
#include <proxygen/lib/http/session/HQUpstreamSession.h>
#include <quic/common/events/HighResQuicTimer.h>

namespace quic {

class QuicClientTransport;
class FileQLogger;

namespace samples {

class HQClient : private quic::QuicSocket::ConnectionSetupCallback {
 public:
  explicit HQClient(const HQToolClientParams& params);

  ~HQClient() override = default;

  int start();

 private:
  // Conn setup callback
  void onConnectionSetupError(quic::QuicError code) noexcept override;
  void onTransportReady() noexcept override;
  void onReplaySafe() noexcept override;

  proxygen::HTTPTransaction* newTransaction(
      proxygen::HTTPTransactionHandler* handler);

  void drainSession();

  proxygen::HTTPTransaction* sendRequest(const proxygen::URL& requestUrl);

  void sendRequests(bool closeSession, uint64_t numOpenableStreams);

  void sendKnobFrame(const folly::StringPiece str);

  class ConnectCallback : public proxygen::HQSession::ConnectCallback {
   public:
    explicit ConnectCallback(HQClient& client) : client_(client) {
    }
    void connectSuccess() override {
      client_.connectSuccess();
    }

    void onReplaySafe() override {
      VLOG(4) << "Connect Callback Replay Safe";
      client_.onReplaySafe();
    }

    void connectError(quic::QuicError error) override {
      LOG(FATAL) << "unreachable";
    }

   private:
    HQClient& client_;
  };

  void connectSuccess();

  void connectError(const quic::QuicError& error);

  void initializeQuicClient();

  void initializeQLogger();

  ConnectCallback connCb_{*this};

  const HQToolClientParams& params_;

  std::shared_ptr<quic::QuicClientTransport> quicClient_;

  QuicTimer::SharedPtr pacingTimer_;

  std::shared_ptr<FollyQuicEventBase> qEvb_;
  folly::EventBase evb_;

  // H3
  proxygen::HQUpstreamSession* hqSession_{nullptr};
  // Interop
  H1QUpstreamSession* h1qSession_{nullptr};

  std::list<std::unique_ptr<CurlService::CurlClient>> curls_;

  std::deque<folly::StringPiece> httpPaths_;

  std::deque<std::chrono::milliseconds> requestGaps_;

  bool failed_{false};

  bool replaySafe_{false};
};

int startClient(const HQToolClientParams& params);
} // namespace samples
} // namespace quic
