/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/session/HTTPSessionAcceptor.h>

#include <cstdlib>

#include <folly/io/async/AsyncSSLSocket.h>
#include <folly/io/async/test/MockAsyncServerSocket.h>
#include <folly/io/async/test/MockAsyncSocket.h>
#include <folly/logging/xlog.h>
#include <proxygen/lib/http/session/test/HTTPSessionMocks.h>
#include <proxygen/lib/utils/TestUtils.h>

using namespace proxygen;
using namespace testing;

using folly::AsyncSocket;
using folly::AsyncSSLSocket;
using folly::SocketAddress;
using folly::test::MockAsyncSocket;

namespace {

const std::string kTestDir = getContainingDirectory(XLOG_FILENAME).str();

}

class HTTPTargetSessionAcceptor : public HTTPSessionAcceptor {
 public:
  explicit HTTPTargetSessionAcceptor(const AcceptorConfiguration& accConfig)
      : HTTPSessionAcceptor(accConfig) {
  }

  HTTPTransaction::Handler* newHandler(HTTPTransaction& /*txn*/,
                                       HTTPMessage* /*msg*/) noexcept override {
    return new MockHTTPHandler();
  }

  void onCreate(const HTTPSessionBase& session) override {
    EXPECT_EQ(expectedProto_,
              getCodecProtocolString(session.getCodecProtocol()));
    sessionsCreated_++;
  }

  void connectionReady(AsyncSocket::UniquePtr sock,
                       const SocketAddress& clientAddr,
                       const std::string& nextProtocolName,
                       SecureTransportType secureTransportType,
                       wangle::TransportInfo& tinfo) {
    HTTPSessionAcceptor::connectionReady(std::move(sock),
                                         clientAddr,
                                         nextProtocolName,
                                         secureTransportType,
                                         tinfo);
  }

  void onSessionCreationError(ProxygenError /*error*/) override {
    sessionCreationErrors_++;
  }

  uint32_t sessionsCreated_{0};
  uint32_t sessionCreationErrors_{0};
  std::string expectedProto_;
};

class HTTPSessionAcceptorTestBase
    : public ::testing::TestWithParam<const char*> {
 public:
  virtual void setupSSL() {
    setenv("test_cert_pem_path", (kTestDir + "test_cert1.pem").c_str(), 0);
    setenv("test_cert_key_path", (kTestDir + "test_cert1.key").c_str(), 0);
    sslCtxConfig_.setCertificate(std::getenv("test_cert_pem_path"),
                                 std::getenv("test_cert_key_path"),
                                 "");

    sslCtxConfig_.isDefault = true;
    sslCtxConfig_.clientVerification =
        folly::SSLContext::VerifyClientCertificate::DO_NOT_REQUEST;
    config_->sslContextConfigs.emplace_back(sslCtxConfig_);
  }

  void SetUp() override {
    config_ = std::make_unique<AcceptorConfiguration>();
    SocketAddress address("127.0.0.1", 0);
    config_->bindAddress = address;
    setupSSL();
    newAcceptor();
  }

  void newAcceptor() {
    acceptor_ = std::make_unique<HTTPTargetSessionAcceptor>(*config_);
    EXPECT_CALL(mockServerSocket_, addAcceptCallback(_, _, _));
    acceptor_->init(&mockServerSocket_, &eventBase_);
  }

 protected:
  std::unique_ptr<AcceptorConfiguration> config_;
  wangle::SSLContextConfig sslCtxConfig_;
  std::unique_ptr<HTTPTargetSessionAcceptor> acceptor_;
  folly::EventBase eventBase_;
  folly::test::MockAsyncServerSocket mockServerSocket_;
};

class HTTPSessionAcceptorTestNPN : public HTTPSessionAcceptorTestBase {};
class HTTPSessionAcceptorTestNPNPlaintext
    : public HTTPSessionAcceptorTestBase {};
class HTTPSessionAcceptorTestNPNJunk : public HTTPSessionAcceptorTestBase {};

// Verify HTTPSessionAcceptor creates the correct codec based on NPN
TEST_P(HTTPSessionAcceptorTestNPN, Npn) {
  std::string proto(GetParam());
  if (proto == "") {
    acceptor_->expectedProto_ = "http/1.1";
  } else if (proto.find("h2") != std::string::npos) {
    acceptor_->expectedProto_ = "http/2";
  } else {
    acceptor_->expectedProto_ = proto;
  }

  auto ctx = std::make_shared<folly::SSLContext>();
  AsyncSSLSocket::UniquePtr sock(new AsyncSSLSocket(ctx, &eventBase_));
  SocketAddress clientAddress;
  wangle::TransportInfo tinfo;
  acceptor_->connectionReady(
      std::move(sock), clientAddress, proto, SecureTransportType::TLS, tinfo);
  EXPECT_EQ(acceptor_->sessionsCreated_, 1);
  EXPECT_EQ(acceptor_->sessionCreationErrors_, 0);
}

std::array<char const*, 4> protos1{"h2-14", "h2", "http/1.1", ""};
INSTANTIATE_TEST_SUITE_P(NPNPositive,
                         HTTPSessionAcceptorTestNPN,
                         ::testing::ValuesIn(protos1));

// Verify HTTPSessionAcceptor creates the correct plaintext codec
TEST_P(HTTPSessionAcceptorTestNPNPlaintext, PlaintextProtocols) {
  std::string proto(GetParam());
  config_->plaintextProtocol = proto;
  newAcceptor();
  if (proto == "h2c") {
    acceptor_->expectedProto_ = "http/2";
  } else {
    acceptor_->expectedProto_ = proto;
  }
  AsyncSocket::UniquePtr sock(new AsyncSocket(&eventBase_));
  SocketAddress clientAddress;
  wangle::TransportInfo tinfo;
  acceptor_->connectionReady(
      std::move(sock), clientAddress, "", SecureTransportType::NONE, tinfo);
  EXPECT_EQ(acceptor_->sessionsCreated_, 1);
  EXPECT_EQ(acceptor_->sessionCreationErrors_, 0);
}

std::array<char const*, 1> protos2{"h2c"};
INSTANTIATE_TEST_SUITE_P(NPNPlaintext,
                         HTTPSessionAcceptorTestNPNPlaintext,
                         ::testing::ValuesIn(protos2));

// Verify HTTPSessionAcceptor closes the socket on invalid NPN
TEST_F(HTTPSessionAcceptorTestNPNJunk, Npn) {
  std::string proto("/http/1.1");
  MockAsyncSocket::UniquePtr sock(new MockAsyncSocket(&eventBase_));
  SocketAddress clientAddress;
  wangle::TransportInfo tinfo;
  EXPECT_CALL(*sock.get(), closeNow());
  acceptor_->connectionReady(
      std::move(sock), clientAddress, proto, SecureTransportType::NONE, tinfo);
  EXPECT_EQ(acceptor_->sessionsCreated_, 0);
  EXPECT_EQ(acceptor_->sessionCreationErrors_, 1);
}

TEST_F(HTTPSessionAcceptorTestNPN, AcceptorConfigCapture) {
  newAcceptor();
  config_.reset();
  acceptor_->expectedProto_ = "http/1.1";
  AsyncSocket::UniquePtr sock(new AsyncSocket(&eventBase_));
  SocketAddress clientAddress;
  wangle::TransportInfo tinfo;
  acceptor_->connectionReady(
      std::move(sock), clientAddress, "", SecureTransportType::NONE, tinfo);
}
