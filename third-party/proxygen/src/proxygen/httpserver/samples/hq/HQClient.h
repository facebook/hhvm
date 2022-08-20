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
#include <proxygen/httpserver/samples/hq/HQCommandLine.h>
#include <proxygen/lib/http/session/HQUpstreamSession.h>
#include <quic/common/Timers.h>

namespace quic {

class QuicClientTransport;
class FileQLogger;

namespace samples {

class HQClient : private proxygen::HQSession::ConnectCallback {
 public:
  explicit HQClient(const HQToolClientParams& params);

  ~HQClient() override = default;

  int start();

 private:
  proxygen::HTTPTransaction* sendRequest(const proxygen::URL& requestUrl);

  void sendRequests(bool closeSession, uint64_t numOpenableStreams);

  void sendKnobFrame(const folly::StringPiece str);

  void connectSuccess() override;

  void onReplaySafe() override;

  void connectError(quic::QuicError error) override;

  void initializeQuicClient();

  void initializeQLogger();

  const HQToolClientParams& params_;

  std::shared_ptr<quic::QuicClientTransport> quicClient_;

  TimerHighRes::SharedPtr pacingTimer_;

  folly::EventBase evb_;

  proxygen::HQUpstreamSession* session_;

  std::list<std::unique_ptr<CurlService::CurlClient>> curls_;

  std::deque<folly::StringPiece> httpPaths_;

  bool failed_{false};
};

int startClient(const HQToolClientParams& params);
} // namespace samples
} // namespace quic
