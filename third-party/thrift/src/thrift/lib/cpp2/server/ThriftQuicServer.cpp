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

#include <quic/server/async_tran/QuicAsyncTransportServer.h>
#include <thrift/lib/cpp2/server/Cpp2Worker.h>
#include <thrift/lib/cpp2/server/ThriftQuicServer.h>

namespace {

const quic::TransportSettings& getQuicTransportSettings() {
  static quic::TransportSettings ts = ([]() {
    quic::TransportSettings ts_;
    ts_.advertisedInitialConnectionWindowSize = 60 * 1024 * 1024;
    ts_.advertisedInitialBidiLocalStreamWindowSize = 60 * 1024 * 1024;
    ts_.advertisedInitialMaxStreamsBidi = 1;
    ts_.advertisedInitialMaxStreamsUni = 0;
    ts_.numGROBuffers_ = quic::kMaxNumGROBuffers;
    ts_.writeConnectionDataPacketsLimit = 50;
    ts_.batchingMode = quic::QuicBatchingMode::BATCHING_MODE_GSO;
    ts_.maxBatchSize = 50;
    ts_.initCwndInMss = 100;
    ts_.maxCwndInMss = quic::kLargeMaxCwndInMss;
    ts_.maxRecvBatchSize = 64;
    ts_.shouldRecvBatch = true;
    ts_.shouldUseRecvmmsgForBatchRecv = true;
    return ts_;
  })();

  return ts;
}

} // namespace

namespace apache::thrift {

ThriftQuicServer::ThriftQuicServer() = default;

ThriftQuicServer::~ThriftQuicServer() = default;

void ThriftQuicServer::stop() {
  ThriftServer::stop();
  if (quicServer_) {
    quicServer_->shutdown();
  }
}

void ThriftQuicServer::startAdditionalServers() {
  // add hook into quic server
  quicServer_ = std::make_unique<::quic::QuicAsyncTransportServer>(
      [this](folly::AsyncTransport::UniquePtr asyncTransport) {
        auto* evb = asyncTransport->getEventBase();
        // get worker associated with evb
        auto** worker = evbToWorker_.get(*evb);
        if (!worker) {
          // worker destructed or hasn't begun yet, either way close to
          // prevent connection from lingering
          asyncTransport->closeWithReset();
          return;
        }

        auto clientAddr = asyncTransport->getPeerAddress();
        auto alpn = asyncTransport->getApplicationProtocol();
        (*worker)->onNewConnection(
            std::move(asyncTransport),
            &clientAddr,
            alpn,
            wangle::SecureTransportType::TLS,
            wangle::TransportInfo());
      });
  auto keepalives = ioThreadPool_->getAllEventBases();
  std::vector<folly::EventBase*> evbs(keepalives.size());
  std::transform(
      keepalives.begin(),
      keepalives.end(),
      evbs.begin(),
      [](folly::Executor::KeepAlive<folly::EventBase>& in) {
        return in->getEventBase();
      });

  auto server_addr = socket_ ? socket_->getAddress() : getAddress();
  CHECK(server_addr.isInitialized());

  auto wangleConfig = getServerSocketConfig();
  auto fizzCertManager = std::shared_ptr<fizz::server::CertManager>(
      wangle::FizzConfigUtil::createCertManager(wangleConfig, nullptr)
          .release());
  auto fizzContext = wangle::FizzConfigUtil::createFizzContext(wangleConfig);
  fizzContext->setCertManager(fizzCertManager);

  quicServer_->setFizzContext(std::move(fizzContext));
  quicServer_->setTransportSettings(getQuicTransportSettings());
  quicServer_->start(server_addr, evbs);
}

} // namespace apache::thrift
