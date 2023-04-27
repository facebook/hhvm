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

#include <folly/io/async/EventBase.h>
#include <folly/portability/GTest.h>
#include <thrift/lib/cpp2/async/Cpp2Channel.h>
#include <thrift/lib/cpp2/async/RequestCallback.h>

#include <memory>
#include <folly/io/async/AsyncSocket.h>
#include <proxygen/httpserver/HTTPServer.h>
#include <proxygen/httpserver/HTTPServerOptions.h>
#include <proxygen/httpserver/ScopedHTTPServer.h>
#include <thrift/lib/cpp2/transport/core/ThriftClient.h>
#include <thrift/lib/cpp2/transport/http2/client/H2ClientConnection.h>

using namespace proxygen;

namespace apache {
namespace thrift {

TEST(H2ClientConnectionTest, ServerCloseSocketImmediate) {
  folly::SocketAddress saddr;
  saddr.setFromLocalPort(static_cast<uint16_t>(0));
  proxygen::HTTPServer::IPConfig cfg{
      saddr, proxygen::HTTPServer::Protocol::HTTP2};

  auto server =
      proxygen::ScopedHTTPServer::start(cfg, proxygen::HTTPServerOptions{});
  ASSERT_NE(nullptr, server);
  const auto port = server->getPort();
  ASSERT_NE(0, port);

  folly::EventBase evb;
  folly::SocketAddress addr;
  addr.setFromLocalPort(port);
  folly::AsyncSocket::UniquePtr sock(new folly::AsyncSocket(&evb, addr));
  auto conn = H2ClientConnection::newHTTP2Connection(std::move(sock));
  EXPECT_TRUE(conn->good());

  server.reset(); // server closes connection

  evb.loop();
  EXPECT_FALSE(conn->good());
  EXPECT_EQ(nullptr, conn->getTransport());
}

class H2TestRequestHandler : public RequestHandler {
 public:
  explicit H2TestRequestHandler(folly::EventBase& controlEvb)
      : controlEvb_(controlEvb) {}

  void onRequest(std::unique_ptr<HTTPMessage>) noexcept override {}

  void onBody(std::unique_ptr<folly::IOBuf>) noexcept override {}

  void onUpgrade(UpgradeProtocol) noexcept override {}

  void onEOM() noexcept override {
    // Initiate GOAWAY.
    downstream_->getTransaction()->getTransport().drain();

    // Notify control thread.
    controlEvb_.terminateLoopSoon();
    // TODO

    // try {
    //   ResponseBuilder r(downstream_);
    //   (*handlerPtr_)(*request_, requestBody_.move(), r);
    //   r.sendWithEOM();
    // } catch (const std::exception& ex) {
    //   ResponseBuilder(downstream_)
    //       .status(500, "Internal Server Error")
    //       .body(ex.what())
    //       .sendWithEOM();
    // } catch (...) {
    //   ResponseBuilder(downstream_)
    //       .status(500, "Internal Server Error")
    //       .body("Unknown exception thrown")
    //       .sendWithEOM();
    // }
  }

  void requestComplete() noexcept override {}

  void onError(ProxygenError) noexcept override {}

  void resume() noexcept {
    eventBase_->runInEventBaseThread(
        [this] { ResponseBuilder(downstream_).status(404, "").sendWithEOM(); });
  }

  void setEventBase(folly::EventBase* evb) { eventBase_ = evb; }

 private:
  std::unique_ptr<HTTPMessage> request_;
  folly::IOBufQueue requestBody_;
  folly::EventBase& controlEvb_;
  folly::EventBase* eventBase_;
};

class H2TestHandlerFactory : public RequestHandlerFactory {
 public:
  explicit H2TestHandlerFactory(H2TestRequestHandler* handler)
      : testHandler_(handler) {}

  void onServerStart(folly::EventBase* evb) noexcept override {
    testHandler_->setEventBase(evb);
  }

  void onServerStop() noexcept override {}

  RequestHandler* onRequest(RequestHandler*, HTTPMessage*) noexcept override {
    return testHandler_;
  }

 private:
  H2TestRequestHandler* testHandler_;
};

class TestRequestCallback : public apache::thrift::RequestCallback {
 public:
  explicit TestRequestCallback(folly::Promise<ClientReceiveState> promise)
      : promise_(std::move(promise)) {}

  void requestSent() final {}
  void replyReceived(ClientReceiveState&& state) final {
    promise_.setValue(std::move(state));
  }
  void requestError(ClientReceiveState&& state) final {
    promise_.setValue(std::move(state));
  }

 private:
  folly::Promise<ClientReceiveState> promise_;
};

TEST(H2ClientConnectionTest, H2GoAway) {
  folly::SocketAddress saddr;
  saddr.setFromLocalPort(static_cast<uint16_t>(0));
  HTTPServer::IPConfig cfg{saddr, HTTPServer::Protocol::HTTP2};

  folly::EventBase evb;
  H2TestRequestHandler handler(evb);
  std::unique_ptr<H2TestHandlerFactory> factory =
      std::make_unique<H2TestHandlerFactory>(&handler);
  HTTPServerOptions opts;
  opts.threads = 1;
  opts.handlerFactories.push_back(std::move(factory));

  auto server = ScopedHTTPServer::start(cfg, std::move(opts));
  ASSERT_NE(nullptr, server);
  const auto port = server->getPort();
  ASSERT_NE(0, port);

  folly::SocketAddress addr;
  addr.setFromLocalPort(port);
  folly::AsyncSocket::UniquePtr sock(new folly::AsyncSocket(&evb, addr));
  auto conn = H2ClientConnection::newHTTP2Connection(std::move(sock));
  auto channel = apache::thrift::ThriftClient::Ptr(
      new apache::thrift::ThriftClient(std::move(conn)));
  channel->setProtocolId(apache::thrift::protocol::T_COMPACT_PROTOCOL);
  channel->setTimeout(5000);
  EXPECT_TRUE(channel->good());

  evb.loopOnce();

  // Send request
  apache::thrift::RequestCallback::Context callbackContext;
  callbackContext.protocolId = channel->getProtocolId();
  folly::Promise<ClientReceiveState> p;
  auto fut = p.getFuture();
  auto cb = toRequestClientCallbackPtr(
      std::make_unique<TestRequestCallback>(std::move(p)),
      std::move(callbackContext));
  auto header = std::make_shared<THeader>();
  channel->sendRequestResponse(
      RpcOptions(),
      MethodMetadata("foo"),
      SerializedRequest(folly::IOBuf::create(0)),
      std::move(header),
      std::move(cb));

  // Loop until server receives a message and sends us GOAWAY.
  evb.loopForever();

  // Wait 1s for the GOAWAY to be processed by the client.
  evb.runAfterDelay([&] { evb.terminateLoopSoon(); }, 1000);
  evb.loop();

  // The channel is now not good due to being drained.
  EXPECT_FALSE(channel->good());
  // The initial request should be still not responded to.
  EXPECT_FALSE(fut.isReady());

  // Send the response for the initial request.
  handler.resume();

  // Wait for the response.
  auto state = std::move(fut).getVia(&evb);
  EXPECT_TRUE(state.isException());
  EXPECT_EQ(
      state.exception().what(),
      "apache::thrift::transport::TTransportException: Bad status: 404 Not Found");
}

} // namespace thrift
} // namespace apache
