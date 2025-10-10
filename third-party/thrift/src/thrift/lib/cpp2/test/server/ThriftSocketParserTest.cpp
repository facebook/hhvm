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

#include <memory>
#include <string>

#include <gtest/gtest.h>
#include <fizz/client/AsyncFizzClient.h>
#include <folly/Optional.h>
#include <folly/io/async/AsyncSSLSocket.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/EventBase.h>
#include <folly/io/async/test/TestSSLServer.h>
#include <folly/test/TestUtils.h>
#include <folly/testing/TestUtil.h>
#include <thrift/lib/cpp2/PluggableFunction.h>
#include <thrift/lib/cpp2/async/RocketClientChannel.h>
#include <thrift/lib/cpp2/security/extensions/ThriftParametersClientExtension.h>
#include <thrift/lib/cpp2/server/Cpp2Worker.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>
#include <thrift/lib/cpp2/test/gen-cpp2/TestService.h>
#include <thrift/lib/cpp2/test/gen-cpp2/TestServiceAsyncClient.h>
#include <thrift/lib/cpp2/test/util/TestHandler.h>
#include <thrift/lib/cpp2/test/util/TestThriftServerFactory.h>
#include <thrift/lib/cpp2/transport/rocket/server/RocketRoutingHandler.h>
#include <thrift/lib/cpp2/util/ScopedServerThread.h>
#include <wangle/acceptor/ServerSocketConfig.h>

using namespace apache::thrift;
using namespace apache::thrift::test;
using namespace apache::thrift::util;
using namespace apache::thrift::detail;
using folly::test::find_resource;

static int isSocketTLS = 0;

THRIFT_PLUGGABLE_FUNC_SET_TEST(
    std::string, getSocketParser, folly::AsyncTransport& socket) {
  if (socket.getUnderlyingTransport<folly::AsyncSSLSocket>()) {
    isSocketTLS = 1;
    return "aligned";
  }
  if (socket.getUnderlyingTransport<fizz::AsyncFizzBase>()) {
    isSocketTLS = 1;
    return "aligned";
  }
  isSocketTLS = 0;
  return "strategy";
}

namespace {
class FizzStopTLSConnector
    : public fizz::client::AsyncFizzClient::HandshakeCallback,
      public fizz::AsyncFizzBase::EndOfTLSCallback {
 public:
  folly::AsyncSocket::UniquePtr connect(
      const folly::SocketAddress& address, folly::EventBase* eb) {
    eb_ = eb;

    auto sock = folly::AsyncSocket::newSocket(eb_, address);
    auto ctx = std::make_shared<fizz::client::FizzClientContext>();
    ctx->setSupportedAlpns({"rs"});
    auto thriftParametersContext =
        std::make_shared<apache::thrift::ThriftParametersContext>();
    thriftParametersContext->setUseStopTLS(true);
    auto extension =
        std::make_shared<apache::thrift::ThriftParametersClientExtension>(
            thriftParametersContext);

    client_.reset(new fizz::client::AsyncFizzClient(
        std::move(sock), std::move(ctx), std::move(extension)));
    client_->connect(
        this,
        nullptr,
        folly::none,
        folly::none,
        folly::Optional<std::vector<fizz::ech::ParsedECHConfig>>(folly::none),
        std::chrono::milliseconds(100));
    return promise_.getFuture().getVia(eb_);
  }

  void fizzHandshakeSuccess(
      fizz::client::AsyncFizzClient* client) noexcept override {
    client->setEndOfTLSCallback(this);
  }

  void fizzHandshakeError(
      fizz::client::AsyncFizzClient* /* unused */,
      folly::exception_wrapper ex) noexcept override {
    promise_.setException(ex);
    FAIL();
  }

  void endOfTLS(
      fizz::AsyncFizzBase* transport, std::unique_ptr<folly::IOBuf>) override {
    auto sock = transport->getUnderlyingTransport<folly::AsyncSocket>();
    DCHECK(sock);

    auto fd = sock->detachNetworkSocket();
    auto zcId = sock->getZeroCopyBufId();

    // create new socket
    auto plaintextTransport =
        folly::AsyncSocket::UniquePtr(new folly::AsyncSocket(eb_, fd, zcId));
    promise_.setValue(std::move(plaintextTransport));
  }

  fizz::client::AsyncFizzClient::UniquePtr client_;
  folly::Promise<folly::AsyncSocket::UniquePtr> promise_;
  folly::EventBase* eb_{};
};
} // namespace

TEST(ThriftSocketParser, SocketParserTLS) {
  auto server = TestThriftServerFactory<TestHandler>().create();
  server->setSSLPolicy(SSLPolicy::REQUIRED);
  auto sslConfig = std::make_shared<wangle::SSLContextConfig>();
  sslConfig->setNextProtocols({"rs"});
  sslConfig->setCertificate(
      find_resource(folly::test::kTestCert).string(),
      find_resource(folly::test::kTestKey).string(),
      "");
  sslConfig->clientVerification =
      folly::SSLContext::VerifyClientCertificate::DO_NOT_REQUEST;
  server->setSSLConfig(std::move(sslConfig));
  ThriftTlsConfig thriftConfig;
  thriftConfig.enableThriftParamsNegotiation = true;
  thriftConfig.enableStopTLS = true;
  server->setThriftConfig(thriftConfig);
  server->setAcceptorFactory(
      std::make_shared<DefaultThriftAcceptorFactory>(server.get()));
  ScopedServerThread sst(std::move(server));

  folly::EventBase base;
  FizzStopTLSConnector connector;
  auto transport = connector.connect(*sst.getAddress(), &base);
  // Note we only use stop tls with rocket
  apache::thrift::Client<TestService> client(
      RocketClientChannel::newChannel(std::move(transport)));

  std::string response;
  client.sync_sendResponse(response, 64);
  EXPECT_EQ(response, "test64");
  EXPECT_EQ(isSocketTLS, 0);
  base.loopOnce();
}

TEST(ThriftSocketParser, SockeParserNoTLS) {
  auto server = TestThriftServerFactory<TestHandler>().create();
  server->setSSLPolicy(SSLPolicy::REQUIRED);
  auto sslConfig = std::make_shared<wangle::SSLContextConfig>();
  sslConfig->setNextProtocols({"rs"});
  sslConfig->setCertificate(
      find_resource(folly::test::kTestCert).string(),
      find_resource(folly::test::kTestKey).string(),
      "");
  sslConfig->clientVerification =
      folly::SSLContext::VerifyClientCertificate::DO_NOT_REQUEST;
  server->setSSLConfig(std::move(sslConfig));
  ThriftTlsConfig thriftConfig;
  thriftConfig.enableThriftParamsNegotiation = true;
  thriftConfig.enableStopTLS = false;
  server->setThriftConfig(thriftConfig);
  server->setAcceptorFactory(
      std::make_shared<DefaultThriftAcceptorFactory>(server.get()));
  ScopedServerThread sst(std::move(server));

  folly::EventBase base;
  auto sock = folly::AsyncSocket::newSocket(&base, *sst.getAddress());
  auto ctx = std::make_shared<fizz::client::FizzClientContext>();
  ctx->setSupportedAlpns({"rs"});
  auto client = fizz::client::AsyncFizzClient::UniquePtr(
      new fizz::client::AsyncFizzClient(std::move(sock), std::move(ctx)));
  client->connect(
      nullptr,
      nullptr,
      folly::none,
      folly::none,
      folly::Optional<std::vector<fizz::ech::ParsedECHConfig>>(folly::none),
      std::chrono::milliseconds(100));

  apache::thrift::Client<TestService> testClient(
      RocketClientChannel::newChannel(std::move(client)));

  std::string response;
  testClient.sync_sendResponse(response, 64);
  EXPECT_EQ(response, "test64");
  EXPECT_EQ(isSocketTLS, 1);
  base.loopOnce();
}
