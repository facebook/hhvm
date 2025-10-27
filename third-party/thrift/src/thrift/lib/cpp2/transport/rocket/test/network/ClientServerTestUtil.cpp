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

#include <thrift/lib/cpp2/transport/rocket/test/network/ClientServerTestUtil.h>

#include <algorithm>
#include <chrono>
#include <future>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

#include <gtest/gtest.h>

#include <folly/Conv.h>
#include <folly/Function.h>
#include <folly/Optional.h>
#include <folly/SocketAddress.h>
#include <folly/executors/InlineExecutor.h>
#include <folly/fibers/Baton.h>
#include <folly/fibers/FiberManagerMap.h>
#include <folly/futures/Future.h>
#include <folly/io/IOBuf.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/EventBase.h>
#include <folly/io/async/ScopedEventBaseThread.h>

#include <wangle/acceptor/Acceptor.h>
#include <wangle/acceptor/ServerSocketConfig.h>

#include <thrift/lib/cpp2/async/ServerSinkBridge.h>
#include <thrift/lib/cpp2/async/StreamCallbacks.h>
#include <thrift/lib/cpp2/transport/core/TryUtil.h>
#include <thrift/lib/cpp2/transport/rocket/Types.h>
#include <thrift/lib/cpp2/transport/rocket/client/RocketClient.h>
#include <thrift/lib/cpp2/transport/rocket/framing/test/Util.h>
#include <thrift/lib/cpp2/transport/rocket/payload/PayloadSerializer.h>
#include <thrift/lib/cpp2/transport/rocket/server/RocketBiDiClientCallback.h>
#include <thrift/lib/cpp2/transport/rocket/server/RocketServerConnection.h>
#include <thrift/lib/cpp2/transport/rocket/server/RocketServerFrameContext.h>
#include <thrift/lib/cpp2/transport/rocket/server/RocketServerHandler.h>
#include <thrift/lib/cpp2/transport/rocket/server/RocketSinkClientCallback.h>
#include <thrift/lib/cpp2/transport/rocket/server/RocketStreamClientCallback.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_constants.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::rocket::test {

namespace {
constexpr int32_t kClientVersion = 7;
constexpr int32_t kServerVersion = 10;
std::pair<std::unique_ptr<folly::IOBuf>, std::unique_ptr<folly::IOBuf>>
makeTestResponse(
    std::unique_ptr<folly::IOBuf> requestMetadata,
    std::unique_ptr<folly::IOBuf> requestData) {
  std::pair<std::unique_ptr<folly::IOBuf>, std::unique_ptr<folly::IOBuf>>
      response;

  folly::StringPiece data(requestData->coalesce());
  constexpr folly::StringPiece kMetadataEchoPrefix{"metadata_echo:"};
  constexpr folly::StringPiece kDataEchoPrefix{"data_echo:"};

  if (data.removePrefix("sleep_ms:")) {
    // Sleep, then echo back request.
    std::chrono::milliseconds sleepFor(folly::to<uint32_t>(data));
    std::this_thread::sleep_for(sleepFor); // sleep override
  } else if (data.removePrefix("error:")) {
    // Reply with a specific kind of error.
  } else if (data.startsWith(kMetadataEchoPrefix)) {
    // Reply with echoed metadata in the response payload.
    auto responseMetadata = requestData->clone();
    responseMetadata->trimStart(kMetadataEchoPrefix.size());
    response =
        std::make_pair(std::move(responseMetadata), std::move(requestData));
  } else if (data.startsWith(kDataEchoPrefix)) {
    // Reply with echoed data in the response payload.
    auto responseData = requestData->clone();
    responseData->trimStart(kDataEchoPrefix.size());
    response =
        std::make_pair(std::move(requestMetadata), std::move(responseData));
  }

  // If response payload is not set at this point, simply echo back what client
  // sent.
  if (!response.first && !response.second) {
    response =
        std::make_pair(std::move(requestMetadata), std::move(requestData));
  }

  return response;
}
} // namespace

RequestSetupMetadata RocketTestClient::makeTestSetupMetadata(
    MetadataOpaqueMap<std::string, std::string> md) {
  RequestSetupMetadata meta;
  meta.opaque() = {};
  *meta.opaque() = std::move(md);
  meta.maxVersion() = kClientVersion;
  meta.encodeMetadataUsingBinary() = false;
  return meta;
}

RocketTestClient::RocketTestClient(const folly::SocketAddress& serverAddr)
    : evb_(*evbThread_.getEventBase()),
      fm_(folly::fibers::getFiberManager(evb_)),
      serverAddr_(serverAddr) {
  connect();
}

RocketTestClient::~RocketTestClient() {
  disconnect();
}

folly::Try<Payload> RocketTestClient::sendRequestResponseSync(
    Payload request,
    std::chrono::milliseconds timeout,
    RocketClient::WriteSuccessCallback* writeSuccessCallback) {
  folly::Try<Payload> response;
  folly::fibers::Baton baton;

  evb_.runInEventBaseThread([&] {
    fm_.addTaskFinally(
        [&] {
          return client_->sendRequestResponseSync(
              std::move(request), timeout, writeSuccessCallback);
        },
        [&](folly::Try<folly::Try<Payload>>&& r) {
          response = collapseTry(std::move(r));
          baton.post();
        });
  });

  baton.wait();
  return response;
}

folly::Try<void> RocketTestClient::sendRequestFnfSync(Payload request) {
  folly::Try<void> response;
  folly::fibers::Baton baton;

  evb_.runInEventBaseThread([&] {
    fm_.addTaskFinally(
        [&] { return client_->sendRequestFnfSync(std::move(request)); },
        [&](folly::Try<folly::Try<void>>&& r) {
          response = collapseTry(std::move(r));
          baton.post();
        });
  });

  baton.wait();
  return response;
}

folly::Try<ClientBufferedStream<Payload>>
RocketTestClient::sendRequestStreamSync(Payload request) {
  constexpr std::chrono::milliseconds kFirstResponseTimeout{500};
  constexpr std::chrono::milliseconds kChunkTimeout{500};

  class TestStreamCallback final
      : public ::apache::thrift::detail::ClientStreamBridge::
            FirstResponseCallback {
   public:
    TestStreamCallback(
        std::chrono::milliseconds chunkTimeout,
        folly::Promise<ClientBufferedStream<Payload>> p)
        : chunkTimeout_(chunkTimeout), p_(std::move(p)) {}

    // ClientCallback interface
    void onFirstResponse(
        FirstResponsePayload&& firstPayload,
        ::apache::thrift::detail::ClientStreamBridge::ClientPtr
            clientStreamBridge) override {
      if (getRange(*firstPayload.payload) == "error:application") {
        p_.setException(
            folly::make_exception_wrapper<thrift::detail::EncodedError>(
                std::move(firstPayload.payload)));
      } else {
        p_.setValue(
            ClientBufferedStream<Payload>(
                std::move(clientStreamBridge),
                [](folly::Try<StreamPayload>&& v) {
                  if (v.hasValue()) {
                    return folly::Try<Payload>(
                        Payload::makeFromData(std::move(v->payload)));
                  } else if (v.hasException()) {
                    return folly::Try<Payload>(std::move(v.exception()));
                  } else {
                    return folly::Try<Payload>();
                  }
                },
                {100, 0}));
      }
      delete this;
    }

    void onFirstResponseError(folly::exception_wrapper ew) override {
      p_.setException(std::move(ew));
      delete this;
    }

   private:
    std::chrono::milliseconds chunkTimeout_;
    folly::Promise<ClientBufferedStream<Payload>> p_;
  };

  folly::Promise<ClientBufferedStream<Payload>> p;
  auto sf = p.getSemiFuture();

  auto clientCallback = new TestStreamCallback(kChunkTimeout, std::move(p));

  evb_.runInEventBaseThread([&] {
    fm_.addTask([&] {
      client_->sendRequestStream(
          std::move(request),
          kFirstResponseTimeout,
          kChunkTimeout,
          0,
          ::apache::thrift::detail::ClientStreamBridge::create(clientCallback));
    });
  });

  return folly::makeTryWith([&] {
    return std::move(sf).via(&folly::InlineExecutor::instance()).get();
  });
}

void RocketTestClient::sendRequestSink(
    SinkClientCallback* callback, Payload request) {
  evb_.runInEventBaseThread(
      [this, request = std::move(request), callback]() mutable {
        fm_.addTask([this, request = std::move(request), callback]() mutable {
          constexpr std::chrono::milliseconds kFirstResponseTimeout{500};
          client_->sendRequestSink(
              std::move(request), kFirstResponseTimeout, callback);
        });
      });
}

void RocketTestClient::reconnect() {
  disconnect();
  connect();
}

void RocketTestClient::connect() {
  evb_.runInEventBaseThreadAndWait([this] {
    folly::AsyncSocket::UniquePtr socket(
        new folly::AsyncSocket(&evb_, serverAddr_));
    client_ =
        RocketClient::create(evb_, std::move(socket), makeTestSetupMetadata());
  });
}

void RocketTestClient::disconnect() {
  evb_.runInEventBaseThread([client = std::move(client_)] {});
}

void RocketTestClient::verifyVersion() {
  if (client_ && client_->getServerVersion() != -1) {
    EXPECT_EQ(
        std::min(kClientVersion, kServerVersion), client_->getServerVersion());
  }
}

namespace {
class RocketTestServerAcceptor final : public wangle::Acceptor {
 public:
  RocketTestServerAcceptor(
      folly::Function<std::unique_ptr<RocketServerHandler>()>
          frameHandlerFactory,
      std::promise<void> shutdownPromise)
      : Acceptor(std::make_shared<wangle::ServerSocketConfig>()),
        frameHandlerFactory_(std::move(frameHandlerFactory)),
        shutdownPromise_(std::move(shutdownPromise)) {}

  ~RocketTestServerAcceptor() override { EXPECT_EQ(0, connections_); }

  void onNewConnection(
      folly::AsyncTransport::UniquePtr socket,
      const folly::SocketAddress*,
      const std::string&,
      wangle::SecureTransportType,
      const wangle::TransportInfo&) override {
    auto* connection = new RocketServerConnection(
        std::move(socket),
        frameHandlerFactory_(),
        memoryTracker_, // (ingress)
        memoryTracker_, // (egress)
        streamMetricCallback_);

    getConnectionManager()->addConnection(connection);
  }

  void onConnectionsDrained() override { shutdownPromise_.set_value(); }

  void onConnectionAdded(const wangle::ManagedConnection*) override {
    ++connections_;
  }

  void onConnectionRemoved(const wangle::ManagedConnection* conn) override {
    if (expectedRemainingStreams_ != folly::none) {
      if (auto rconn = dynamic_cast<const RocketServerConnection*>(conn)) {
        EXPECT_EQ(expectedRemainingStreams_, rconn->getNumStreams());
      }
    }

    --connections_;
  }

  void setExpectedRemainingStreams(size_t size) {
    expectedRemainingStreams_ = size;
  }

 private:
  folly::Function<std::unique_ptr<RocketServerHandler>()> frameHandlerFactory_;
  std::promise<void> shutdownPromise_;
  size_t connections_{0};
  folly::Optional<size_t> expectedRemainingStreams_ = folly::none;
  MemoryTracker memoryTracker_;
  NoopStreamMetricCallback streamMetricCallback_;
};
} // namespace

class RocketTestServer::RocketTestServerHandler : public RocketServerHandler {
 public:
  explicit RocketTestServerHandler(
      folly::EventBase& ioEvb,
      const MetadataOpaqueMap<std::string, std::string>& expectedSetupMetadata)
      : ioEvb_(ioEvb), expectedSetupMetadata_(expectedSetupMetadata) {}
  void handleSetupFrame(
      SetupFrame&& frame, IRocketServerConnection& connection) final {
    folly::io::Cursor cursor(frame.payload().buffer());
    // Validate Rocket protocol key
    uint32_t protocolKey;
    const bool success = cursor.tryReadBE<uint32_t>(protocolKey);
    EXPECT_TRUE(success);
    EXPECT_TRUE(
        protocolKey == 1 ||
        protocolKey == RpcMetadata_constants::kRocketProtocolKey() ||
        frame.rocketMimeTypes());
    if (protocolKey != 1 &&
        protocolKey != RpcMetadata_constants::kRocketProtocolKey()) {
      cursor.retreat(4);
    }
    // Validate RequestSetupMetadata
    RequestSetupMetadata meta;
    size_t unpackedSize =
        PayloadSerializer::getInstance()->unpack(meta, cursor, false);
    EXPECT_EQ(unpackedSize, frame.payload().metadataSize());
    EXPECT_EQ(expectedSetupMetadata_, meta.opaque().value_or({}));
    version_ = std::min(kServerVersion, meta.maxVersion().value_or(0));
    ServerPushMetadata serverMeta;
    serverMeta.set_setupResponse();
    serverMeta.setupResponse()->version() = version_;
    connection.sendMetadataPush(
        PayloadSerializer::getInstance()->packCompact(serverMeta));
  }

  void handleRequestResponseFrame(
      RequestResponseFrame&& frame, RocketServerFrameContext&& context) final {
    auto dam = splitMetadataAndData(frame.payload());
    auto payload = std::move(frame.payload());
    auto dataPiece = getRange(*dam.second);

    if (dataPiece.removePrefix("error:application")) {
      return context.sendError(
          RocketException(
              ErrorCode::APPLICATION_ERROR, "Application error occurred"),
          nullptr);
    }

    auto response =
        makeTestResponse(std::move(dam.first), std::move(dam.second));
    auto responsePayload = Payload::makeFromMetadataAndData(
        std::move(response.first), std::move(response.second));
    return context.sendPayload(
        std::move(responsePayload), Flags().next(true).complete(true), nullptr);
  }

  void handleRequestFnfFrame(
      RequestFnfFrame&&, RocketServerFrameContext&&) final {}

  void handleRequestStreamFrame(
      RequestStreamFrame&& frame,
      RocketServerFrameContext&&,
      RocketStreamClientCallback* clientCallback) final {
    class TestRocketStreamServerCallback final : public StreamServerCallback {
     public:
      TestRocketStreamServerCallback(
          StreamClientCallback* clientCallback, size_t n, size_t nEchoHeaders)
          : clientCallback_(clientCallback),
            n_(n),
            nEchoHeaders_(nEchoHeaders) {}

      bool onStreamRequestN(uint64_t tokens) override {
        while (tokens-- && i_++ < n_) {
          auto alive = clientCallback_->onStreamNext(
              StreamPayload{
                  folly::IOBuf::copyBuffer(folly::to<std::string>(i_)), {}});
          DCHECK(alive);
        }
        if (i_ == n_ && iEchoHeaders_ == nEchoHeaders_) {
          clientCallback_->onStreamComplete();
          delete this;
          return false;
        }
        return true;
      }

      void onStreamCancel() override { delete this; }

      bool onSinkHeaders(HeadersPayload&& payload) override {
        auto metadata_ref = payload.payload.otherMetadata();
        EXPECT_TRUE(metadata_ref);
        if (metadata_ref) {
          EXPECT_EQ(
              folly::to<std::string>(++iEchoHeaders_),
              (*metadata_ref)["expected_header"]);
        }
        auto alive = clientCallback_->onStreamHeaders(std::move(payload));
        DCHECK(alive);
        if (i_ == n_ && iEchoHeaders_ == nEchoHeaders_) {
          clientCallback_->onStreamComplete();
          delete this;
          return false;
        }
        return true;
      }

      void resetClientCallback(StreamClientCallback& clientCallback) override {
        clientCallback_ = &clientCallback;
      }

     private:
      StreamClientCallback* clientCallback_;
      size_t i_{0};
      size_t iEchoHeaders_{0};
      const size_t n_;
      const size_t nEchoHeaders_;
    };
    std::unique_ptr<folly::IOBuf> buffer =
        std::move(frame.payload()).data()->clone();
    folly::StringPiece data(buffer->coalesce());
    if (data.removePrefix("error:application")) {
      clientCallback->onFirstResponseError(
          folly::make_exception_wrapper<
              thrift::detail::EncodedFirstResponseError>(FirstResponsePayload(
              folly::IOBuf::copyBuffer("error:application"), {})));
      return;
    }
    const size_t nHeaders =
        data.removePrefix("generateheaders:") ? folly::to<size_t>(data) : 0;
    const size_t nEchoHeaders =
        data.removePrefix("echoheaders:") ? folly::to<size_t>(data) : 0;
    const size_t n = nHeaders || nEchoHeaders
        ? 0
        : (data.removePrefix("generate:") ? folly::to<size_t>(data) : 500);
    auto* serverCallback =
        new TestRocketStreamServerCallback(clientCallback, n, nEchoHeaders);
    {
      auto alive = clientCallback->onFirstResponse(
          FirstResponsePayload{
              folly::IOBuf::copyBuffer(folly::to<std::string>(0)), {}},
          nullptr /* evb */,
          serverCallback);
      DCHECK(alive);
    }

    for (size_t i = 1; i <= nHeaders; ++i) {
      HeadersPayloadContent header;
      header.otherMetadata() = {{"expected_header", folly::to<std::string>(i)}};
      auto alive = clientCallback->onStreamHeaders({std::move(header), {}});
      DCHECK(alive);
    }
    if (n == 0 && nEchoHeaders == 0) {
      std::ignore = serverCallback->onStreamRequestN(0);
    }
  }

  void handleRequestChannelFrame(
      RequestChannelFrame&&,
      RocketServerFrameContext&&,
      ChannelRequestCallbackFactory factory) final {
    auto clientCallback = factory.create<RocketSinkClientCallback>();
    apache::thrift::detail::SinkConsumerImpl impl{
        [](folly::coro::AsyncGenerator<folly::Try<StreamPayload>&&> asyncGen)
            -> folly::coro::Task<folly::Try<StreamPayload>> {
          int current = 0;
          while (auto item = co_await asyncGen.next()) {
            auto payload = (*item).value();
            auto data = folly::to<int32_t>(
                folly::StringPiece(payload.payload->coalesce()));
            EXPECT_EQ(current++, data);
          }
          co_return folly::Try<StreamPayload>(StreamPayload(
              folly::IOBuf::copyBuffer(folly::to<std::string>(current)), {}));
        },
        10,
        std::chrono::milliseconds::zero(),
        {}};
    auto serverCallback = apache::thrift::detail::ServerSinkBridge::create(
        std::move(impl), ioEvb_, clientCallback);

    clientCallback->onFirstResponse(
        FirstResponsePayload{
            folly::IOBuf::copyBuffer(folly::to<std::string>(0)), {}},
        nullptr /* evb */,
        serverCallback.get());
    co_withExecutor(
        threadManagerThread_.getEventBase(),
        folly::coro::co_invoke(
            &apache::thrift::detail::ServerSinkBridge::start,
            std::move(serverCallback)))
        .start();
  }

  void connectionClosing() final {}

  int32_t getVersion() const final { return version_; }

 private:
  folly::EventBase& ioEvb_;
  const MetadataOpaqueMap<std::string, std::string>& expectedSetupMetadata_;
  folly::ScopedEventBaseThread threadManagerThread_;
  int32_t version_{0};
};

RocketTestServer::RocketTestServer()
    : evb_(*ioThread_.getEventBase()),
      listeningSocket_(new folly::AsyncServerSocket(&evb_)) {
  std::promise<void> shutdownPromise;
  shutdownFuture_ = shutdownPromise.get_future();

  acceptor_ = std::make_unique<RocketTestServerAcceptor>(
      [this] {
        return std::make_unique<RocketTestServerHandler>(
            evb_, expectedSetupMetadata_);
      },
      std::move(shutdownPromise));
  start();
}

RocketTestServer::~RocketTestServer() {
  stop();
}

void RocketTestServer::start() {
  folly::via(&evb_, [this] {
    acceptor_->init(listeningSocket_.get(), &evb_);
    listeningSocket_->bind(0 /* bind to any port */);
    listeningSocket_->listen(128 /* tcpBacklog */);
    listeningSocket_->startAccepting();
  }).wait();
}

void RocketTestServer::stop() {
  // Ensure socket and acceptor are destroyed in EventBase thread
  folly::via(&evb_, [listeningSocket = std::move(listeningSocket_)] {});
  // Wait for server to drain connections as gracefully as possible.
  shutdownFuture_.wait();
  folly::via(&evb_, [acceptor = std::move(acceptor_)] {});
}

uint16_t RocketTestServer::getListeningPort() const {
  return listeningSocket_->getAddress().getPort();
}

wangle::ConnectionManager* RocketTestServer::getConnectionManager() const {
  return acceptor_->getConnectionManager();
}

void RocketTestServer::setExpectedRemainingStreams(size_t n) {
  if (auto acceptor =
          dynamic_cast<RocketTestServerAcceptor*>(acceptor_.get())) {
    acceptor->setExpectedRemainingStreams(n);
  }
}

void RocketTestServer::setExpectedSetupMetadata(
    MetadataOpaqueMap<std::string, std::string> md) {
  expectedSetupMetadata_ = std::move(md);
}

} // namespace apache::thrift::rocket::test
