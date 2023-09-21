/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/httpserver/samples/hq/HQClient.h>

#include <fstream>
#include <ostream>
#include <string>
#include <thread>

#include <folly/io/async/AsyncTimeout.h>
#include <folly/io/async/EventBaseManager.h>
#include <folly/io/async/ScopedEventBaseThread.h>
#include <folly/json.h>

#include <proxygen/httpserver/samples/hq/FizzContext.h>
#include <proxygen/httpserver/samples/hq/H1QUpstreamSession.h>
#include <proxygen/httpserver/samples/hq/HQLoggerHelper.h>
#include <proxygen/httpserver/samples/hq/InsecureVerifierDangerousDoNotUseInProduction.h>
#include <proxygen/lib/http/codec/HTTP1xCodec.h>
#include <proxygen/lib/utils/UtilInl.h>
#include <quic/api/QuicSocket.h>
#include <quic/client/QuicClientTransport.h>
#include <quic/congestion_control/CongestionControllerFactory.h>
#include <quic/fizz/client/handshake/FizzClientQuicHandshakeContext.h>
#include <quic/logging/FileQLogger.h>

namespace quic::samples {

HQClient::HQClient(const HQToolClientParams& params) : params_(params) {
  if (params_.transportSettings.pacingEnabled) {
    pacingTimer_ = TimerHighRes::newTimer(
        &evb_, params_.transportSettings.pacingTimerResolution);
  }
}

int HQClient::start() {

  initializeQuicClient();
  initializeQLogger();

  // TODO: turn on cert verification
  LOG(INFO) << "HQClient connecting to " << params_.remoteAddress->describe();
  quicClient_->start(this, nullptr);

  // This is to flush the CFIN out so the server will see the handshake as
  // complete.
  evb_.loopForever();
  if (params_.migrateClient) {
    quicClient_->onNetworkSwitch(
        std::make_unique<quic::QuicAsyncUDPSocketWrapper>(&evb_));
    sendRequests(true, quicClient_->getNumOpenableBidirectionalStreams());
  }
  evb_.loop();

  return failed_ ? -1 : 0;
}

void HQClient::onConnectionSetupError(quic::QuicError code) noexcept {
  LOG(ERROR) << "Failed to establish QUIC connection: " << code.message;
  quicClient_->setConnectionSetupCallback(nullptr);
  connectError(code);
}

void HQClient::onTransportReady() noexcept {
  auto alpn = quicClient_->getAppProtocol();
  if (alpn && alpn == proxygen::kHQ) {
    h1qSession_ = new H1QUpstreamSession(quicClient_);
    connectSuccess();
  } else {
    wangle::TransportInfo tinfo;
    hqSession_ = new proxygen::HQUpstreamSession(params_.txnTimeout,
                                                 params_.connectTimeout,
                                                 nullptr, // controller
                                                 tinfo,
                                                 nullptr);
    hqSession_->setConnectCallback(&connCb_);
    quicClient_->setConnectionCallback(hqSession_);
    quicClient_->setConnectionSetupCallback(hqSession_);
    hqSession_->setSocket(quicClient_);
    hqSession_->startNow();
    // TODO: get rid HQUpstreamSession::ConnectCallback
    if (replaySafe_) {
      hqSession_->onReplaySafe();
    }
    hqSession_->onTransportReady(); // invokes connectSuccess()
  }
}

proxygen::HTTPTransaction* HQClient::newTransaction(
    proxygen::HTTPTransactionHandler* handler) {
  proxygen::HTTPTransaction* txn = nullptr;
  if (h1qSession_) {
    txn = h1qSession_->newTransaction(handler);
  } else {
    txn = hqSession_->newTransaction(handler);
  }
  return txn;
}

proxygen::HTTPTransaction* FOLLY_NULLABLE
HQClient::sendRequest(const proxygen::URL& requestUrl) {
  std::unique_ptr<CurlService::CurlClient> client =
      std::make_unique<CurlService::CurlClient>(&evb_,
                                                params_.httpMethod,
                                                requestUrl,
                                                nullptr,
                                                params_.httpHeaders,
                                                params_.httpBody,
                                                false,
                                                params_.httpVersion.major,
                                                params_.httpVersion.minor);

  client->setLogging(params_.logResponse);
  client->setHeadersLogging(params_.logResponseHeaders);
  auto txn = newTransaction(client.get());
  if (!txn) {
    return nullptr;
  }

  if (!params_.outdir.empty()) {
    bool canWrite = false;
    // default output file name
    std::string filename = "hq.out";
    // try to get the name from the path
    folly::StringPiece path = requestUrl.getPath();
    size_t offset = proxygen::findLastOf(path, '/');
    if (offset != std::string::npos && (offset + 1) != path.size()) {
      filename = std::string(path.subpiece(offset + 1));
    }
    filename = folly::to<std::string>(params_.outdir, "/", filename);
    canWrite = client->saveResponseToFile(filename);
    if (!canWrite) {
      LOG(ERROR) << "Can not write output to file '" << filename
                 << "' printing to stdout instead";
    }
  }
  client->sendRequest(txn);
  curls_.emplace_back(std::move(client));
  return txn;
}

void HQClient::drainSession() {
  if (h1qSession_) {
    h1qSession_->drain();
  } else {
    hqSession_->drain();
    hqSession_->closeWhenIdle();
  }
}

void HQClient::sendRequests(bool closeSession, uint64_t numOpenableStreams) {
  VLOG(10) << "http-version:" << params_.httpVersion;
  do {
    proxygen::URL requestUrl(httpPaths_.front().str(), /*secure=*/true);
    sendRequest(requestUrl);
    httpPaths_.pop_front();
    numOpenableStreams--;
  } while (!params_.sendRequestsSequentially && !httpPaths_.empty() &&
           numOpenableStreams > 0);
  if (closeSession && httpPaths_.empty()) {
    drainSession();
  }
  // If there are still pending requests to be sent sequentially, schedule a
  // callback on the first EOM to try to make one more request. That callback
  // will keep scheduling itself until there are no more requests.
  if (params_.sendRequestsSequentially && !httpPaths_.empty()) {
    auto callSendRequestsAfterADelay = [&]() {
      if (params_.gapBetweenRequests.count() > 0) {
        evb_.runAfterDelay(
            [&]() {
              uint64_t numOpenable =
                  quicClient_->getNumOpenableBidirectionalStreams();
              if (numOpenable > 0) {
                sendRequests(true, numOpenable);
              };
            },
            params_.gapBetweenRequests.count());
      } else {
        uint64_t numOpenable =
            quicClient_->getNumOpenableBidirectionalStreams();
        if (numOpenable > 0) {
          sendRequests(true, numOpenable);
        };
      }
    };
    CHECK(!curls_.empty());
    curls_.back()->setEOMFunc(callSendRequestsAfterADelay);
  }
}
static std::function<void()> selfSchedulingRequestRunner;

void HQClient::connectSuccess() {
  if (params_.sendKnobFrame) {
    sendKnobFrame("Hello, World from Client!");
  }
  uint64_t numOpenableStreams =
      quicClient_->getNumOpenableBidirectionalStreams();
  CHECK_GT(numOpenableStreams, 0);
  httpPaths_.insert(
      httpPaths_.end(), params_.httpPaths.begin(), params_.httpPaths.end());
  sendRequests(!params_.migrateClient, numOpenableStreams);
  // If there are still pending requests to be send in parallel, schedule a
  // callback on the first EOM to try to make some more. That callback will keep
  // scheduling itself until there are no more requests.
  if (!params_.sendRequestsSequentially && !httpPaths_.empty()) {
    selfSchedulingRequestRunner = [&]() {
      uint64_t numOpenable = quicClient_->getNumOpenableBidirectionalStreams();
      if (numOpenable > 0) {
        sendRequests(true, numOpenable);
      };
      if (!httpPaths_.empty()) {
        auto rtt = std::chrono::duration_cast<std::chrono::milliseconds>(
            quicClient_->getTransportInfo().srtt);
        evb_.timer().scheduleTimeoutFn(
            selfSchedulingRequestRunner,
            std::max(rtt, std::chrono::milliseconds(1)));
      }
    };
    CHECK(!curls_.empty());
    curls_.back()->setEOMFunc(selfSchedulingRequestRunner);
  }
}

void HQClient::sendKnobFrame(const folly::StringPiece str) {
  if (str.empty()) {
    return;
  }
  uint64_t knobSpace = 0xfaceb00c;
  uint64_t knobId = 100;
  Buf buf(folly::IOBuf::create(str.size()));
  memcpy(buf->writableData(), str.data(), str.size());
  buf->append(str.size());
  VLOG(10) << "Sending Knob Frame to peer. KnobSpace: " << std::hex << knobSpace
           << " KnobId: " << std::dec << knobId << " Knob Blob" << str;
  const auto knobSent = quicClient_->setKnob(0xfaceb00c, 100, std::move(buf));
  if (knobSent.hasError()) {
    LOG(ERROR) << "Failed to send Knob frame to peer. Received error: "
               << knobSent.error();
  }
}

void HQClient::onReplaySafe() noexcept {
  VLOG(4) << "Transport replay safe";
  replaySafe_ = true;
  evb_.terminateLoopSoon();
}

void HQClient::connectError(const quic::QuicError& error) {
  LOG(ERROR) << "HQClient failed to connect, error=" << toString(error.code)
             << ", msg=" << error.message;
  failed_ = true;
  evb_.terminateLoopSoon();
}

void HQClient::initializeQuicClient() {
  auto sock = std::make_unique<quic::QuicAsyncUDPSocketWrapper>(&evb_);
  auto client = std::make_shared<quic::QuicClientTransport>(
      &evb_,
      std::move(sock),
      quic::FizzClientQuicHandshakeContext::Builder()
          .setFizzClientContext(
              createFizzClientContext(params_, params_.earlyData))
          .setCertificateVerifier(
              std::make_unique<
                  proxygen::InsecureVerifierDangerousDoNotUseInProduction>())
          .setPskCache(params_.pskCache)
          .build());
  client->setPacingTimer(pacingTimer_);
  client->setHostname(params_.host);
  client->addNewPeerAddress(params_.remoteAddress.value());
  if (params_.localAddress.has_value()) {
    client->setLocalAddress(*params_.localAddress);
  }
  client->setCongestionControllerFactory(
      std::make_shared<quic::DefaultCongestionControllerFactory>());
  client->setTransportSettings(params_.transportSettings);
  client->setSupportedVersions(params_.quicVersions);

  quicClient_ = std::move(client);
}

void HQClient::initializeQLogger() {
  if (!quicClient_) {
    return;
  }
  // Not used immediately, but if not set
  // the qlogger wont be able to report. Checking early
  if (params_.qLoggerPath.empty()) {
    return;
  }

  auto qLogger = std::make_shared<HQLoggerHelper>(
      params_.qLoggerPath, params_.prettyJson, quic::VantagePoint::Client);
  quicClient_->setQLogger(std::move(qLogger));
}

int startClient(const HQToolClientParams& params) {
  HQClient client(params);
  return client.start();
}

} // namespace quic::samples
