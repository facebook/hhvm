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

#include <folly/io/async/AsyncTimeout.h>
#include <folly/io/async/EventBaseManager.h>
#include <folly/io/async/ScopedEventBaseThread.h>
#include <folly/json/json.h>

#include <proxygen/httpserver/samples/hq/FizzContext.h>
#include <proxygen/httpserver/samples/hq/H1QUpstreamSession.h>
#include <proxygen/httpserver/samples/hq/HQLoggerHelper.h>
#include <proxygen/httpserver/samples/hq/InsecureVerifierDangerousDoNotUseInProduction.h>
#include <proxygen/lib/utils/UtilInl.h>
#include <quic/api/QuicSocket.h>
#include <quic/client/QuicClientTransport.h>
#include <quic/common/udpsocket/FollyQuicAsyncUDPSocket.h>
#include <quic/congestion_control/CongestionControllerFactory.h>
#include <quic/fizz/client/handshake/FizzClientQuicHandshakeContext.h>

namespace quic::samples {

HQClient::HQClient(const HQToolClientParams& params)
    : params_(params), qEvb_(std::make_shared<FollyQuicEventBase>(&evb_)) {
  if (params_.transportSettings.pacingEnabled) {
    pacingTimer_ = std::make_shared<HighResQuicTimer>(
        &evb_, params_.transportSettings.pacingTimerResolution);
  }
}

int HQClient::start(const folly::SocketAddress& localAddress) {

  initializeQuicClient(*params_.remoteAddress, localAddress);
  initializeQLogger();

  // TODO: turn on cert verification
  LOG(INFO) << "HQClient connecting to " << params_.remoteAddress->describe();
  quicClient_->start(this, nullptr);
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

  if (onBodyFunc_) {
    client->setOnBodyFunc(onBodyFunc_.value());
  }
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
      if (params_.migrateClient && httpPaths_.size() % 2 == 0) {
        auto newSock = std::make_unique<FollyQuicAsyncUDPSocket>(qEvb_);
        auto bindRes = newSock->bind(folly::SocketAddress("::", 0));
        CHECK(!bindRes.hasError());
        auto startProbeRes =
            quicClient_->startPathProbe(std::move(newSock), this);
        CHECK(!startProbeRes.hasError()) << startProbeRes.error();
      }
      std::chrono::milliseconds gap = requestGaps_.front();
      requestGaps_.pop_front();
      if (gap.count() > 0) {
        evb_.runAfterDelay(
            [&]() {
              uint64_t numOpenable =
                  quicClient_->getNumOpenableBidirectionalStreams();
              if (numOpenable > 0) {
                sendRequests(true, numOpenable);
              };
            },
            gap.count());
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
  for (auto const& s : params_.requestGaps) {
    requestGaps_.emplace_back(folly::to<uint32_t>(s));
  }
  if (requestGaps_.size() == 1) {
    // #gaps must be one less than #paths, since gaps occur between downloads.
    // Already one gap in dequeue, so copy in #paths-2 more.
    for (int32_t i = 0; i < static_cast<int32_t>(httpPaths_.size()) - 2; ++i) {
      requestGaps_.emplace_back(requestGaps_.front());
    }
  }
  // Check that there is exactly one gap between each path download.
  // Ignore gaps_ms flag with only one httpPath.
  if (httpPaths_.size() > 1 && httpPaths_.size() != requestGaps_.size() + 1) {
    throw std::runtime_error(
        "Number of gaps must be one (same gap between all paths) or one less "
        "than number of paths.");
  }

  sendRequests(!params_.migrateClient, numOpenableStreams);
  // If there are still pending requests to be send in parallel, schedule a
  // callback on the first EOM to try to make some more. That callback will
  // keep scheduling itself until there are no more requests.
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
  BufPtr buf(folly::IOBuf::create(str.size()));
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
}

void HQClient::connectError(const quic::QuicError& error) {
  LOG(ERROR) << "HQClient failed to connect, error=" << toString(error.code)
             << ", msg=" << error.message;
  failed_ = true;
  evb_.terminateLoopSoon();
}

void HQClient::initializeQuicClient(const folly::SocketAddress& remoteAddress,
                                    const folly::SocketAddress& localAddress) {
  auto sock = std::make_unique<FollyQuicAsyncUDPSocket>(qEvb_);
  auto handshakeContextBuilder =
      quic::FizzClientQuicHandshakeContext::Builder()
          .setFizzClientContext(
              createFizzClientContext(params_,
                                      params_.supportedAlpns,
                                      params_.earlyData,
                                      params_.certificateFilePath,
                                      params_.keyFilePath))
          .setPskCache(params_.pskCache);

  if (!params_.verifyServerCert) {
    handshakeContextBuilder =
        std::move(handshakeContextBuilder)
            .setCertificateVerifier(
                std::make_unique<
                    proxygen::InsecureVerifierDangerousDoNotUseInProduction>());
  }

  auto client = std::make_shared<quic::QuicClientTransport>(
      qEvb_,
      std::move(sock),
      std::move(handshakeContextBuilder).build(),
      params_.clientCidLength);
  client->setPacingTimer(pacingTimer_);
  client->setHostname(params_.host);
  client->addNewPeerAddress(remoteAddress);
  client->setLocalAddress(localAddress);
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

void HQClient::onPathValidationResult(const PathInfo& pathInfo) {
  if (pathInfo.status == PathStatus::Validated) {
    LOG(INFO) << fmt::format(
        "Path probe successful. Migrating connection to: local address = {}, "
        "peer address = {}",
        pathInfo.localAddress.describe(),
        pathInfo.peerAddress.describe());
    auto migrationRes = quicClient_->migrateConnection(pathInfo.id);
    if (migrationRes.hasError()) {
      LOG(ERROR) << "Failed to migrate connection: " << migrationRes.error();
    }
  } else {
    LOG(ERROR) << "Path probe timed out. Deleting the path.";
    auto removeRes = quicClient_->removePath(pathInfo.id);
    if (removeRes.hasError()) {
      LOG(ERROR) << "Failed to remove path: " << removeRes.error();
      failed_ = true;
      evb_.terminateLoopSoon();
    }
  }
}

int startClient(const HQToolClientParams& params) {
  HQClient client(params);

  folly::SocketAddress localAddr;
  if (params.localAddress.has_value()) {
    localAddr = *params.localAddress;
  } else {
    if (params.remoteAddress.has_value() &&
        params.remoteAddress->getFamily() == AF_INET) {
      localAddr = folly::SocketAddress("0.0.0.0", 0);
    } else {
      localAddr = folly::SocketAddress("::", 0);
    }
  }

  return client.start(localAddr);
}

} // namespace quic::samples
