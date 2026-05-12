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

#include <gtest/gtest.h>

#include <chrono>
#include <memory>
#include <stdexcept>
#include <string>

#include <fizz/client/AsyncFizzClient.h>
#include <fizz/client/FizzClientContext.h>
#include <folly/ExceptionWrapper.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/ScopedEventBaseThread.h>
#include <folly/synchronization/Baton.h>

#include <thrift/lib/cpp2/Flags.h>
#include <thrift/lib/cpp2/async/RocketClientChannel.h>
#include <thrift/lib/cpp2/fast_thrift/security/FizzServerCertConfig.h>
#include <thrift/lib/cpp2/fast_thrift/security/test/TestCert.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/FastThriftServer.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/test/if/gen-cpp2/FastThriftServer.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/test/if/gen-cpp2/FastThriftServerAsyncClient.h>
#include <thrift/lib/cpp2/security/extensions/ThriftParametersClientExtension.h>
#include <thrift/lib/cpp2/security/extensions/ThriftParametersContext.h>

THRIFT_FLAG_DECLARE_bool(rocket_client_binary_rpc_metadata_encoding);

namespace apache::thrift::fast_thrift::thrift::test::integration::test {

namespace ftt = ::apache::thrift::fast_thrift::thrift;
namespace integration =
    ::apache::thrift::fast_thrift::thrift::test::integration;
using ::apache::thrift::FastServiceHandler;
using ::apache::thrift::fast_thrift::thrift::test::integration::EchoResponse;
using ::apache::thrift::fast_thrift::thrift::test::integration::
    NotFoundException;
using ::apache::thrift::fast_thrift::thrift::test::integration::
    PermissionDeniedException;

namespace {

// User-implemented FastServiceHandler. Test sets flags to control behavior;
// methods call cb->result()/done()/exception() synchronously.
class TestHandler : public FastServiceHandler<integration::FastThriftServer> {
 public:
  bool throwNotFound{false};
  bool throwPermissionDenied{false};

  void async_eb_ping(ftt::FastHandlerCallbackPtr<void> cb) override {
    cb->done();
  }

  void async_eb_add(
      ftt::FastHandlerCallbackPtr<int64_t> cb, int64_t a, int64_t b) override {
    cb->result(a + b);
  }

  void async_eb_echo(
      ftt::FastHandlerCallbackPtr<std::unique_ptr<EchoResponse>> cb,
      std::unique_ptr<std::string> message) override {
    auto resp = std::make_unique<EchoResponse>();
    resp->message() = std::string("echoed:") + *message;
    cb->result(std::move(resp));
  }

  void async_eb_lookup(
      ftt::FastHandlerCallbackPtr<std::unique_ptr<EchoResponse>> cb,
      int32_t id) override {
    if (throwNotFound) {
      NotFoundException ex;
      ex.id() = id;
      ex.message() = "not found";
      cb->exception(
          folly::make_exception_wrapper<NotFoundException>(std::move(ex)));
      return;
    }
    auto resp = std::make_unique<EchoResponse>();
    resp->message() = "found";
    cb->result(std::move(resp));
  }

  void async_eb_secureLookup(
      ftt::FastHandlerCallbackPtr<std::unique_ptr<EchoResponse>> cb,
      int32_t /*id*/,
      std::unique_ptr<std::string> user) override {
    if (throwPermissionDenied) {
      PermissionDeniedException ex;
      ex.user() = *user;
      ex.reason() = "no perms";
      cb->exception(
          folly::make_exception_wrapper<PermissionDeniedException>(
              std::move(ex)));
      return;
    }
    auto resp = std::make_unique<EchoResponse>();
    resp->message() = "ok";
    cb->result(std::move(resp));
  }
};

// Establishes a fizz-client connection to `address` and returns the negotiated
// AsyncTransport. Blocks until handshake completes; returns nullptr on error.
folly::AsyncTransport::UniquePtr connectFizz(
    folly::EventBase* evb, const folly::SocketAddress& address) {
  struct Cb : public folly::AsyncSocket::ConnectCallback,
              public fizz::client::AsyncFizzClient::HandshakeCallback {
    folly::Baton<>& baton;
    folly::exception_wrapper& outErr;
    fizz::client::AsyncFizzClient::UniquePtr& outClient;
    Cb(folly::Baton<>& b,
       folly::exception_wrapper& e,
       fizz::client::AsyncFizzClient::UniquePtr& c)
        : baton(b), outErr(e), outClient(c) {}

    void connectSuccess() noexcept override {
      outClient->connect(
          this,
          /*verifier=*/nullptr,
          /*sni=*/folly::none,
          /*pskIdentity=*/folly::none,
          /*echConfigs=*/folly::none);
    }
    void connectErr(const folly::AsyncSocketException& ex) noexcept override {
      outErr = folly::exception_wrapper(ex);
      baton.post();
    }
    void fizzHandshakeSuccess(
        fizz::client::AsyncFizzClient*) noexcept override {
      baton.post();
    }
    void fizzHandshakeError(
        fizz::client::AsyncFizzClient*,
        folly::exception_wrapper ex) noexcept override {
      outErr = std::move(ex);
      baton.post();
    }
  };

  folly::Baton<> done;
  folly::exception_wrapper err;
  fizz::client::AsyncFizzClient::UniquePtr fizzClient;
  Cb cb(done, err, fizzClient);

  evb->runInEventBaseThreadAndWait([&] {
    auto sock = folly::AsyncSocket::newSocket(evb);
    auto fizzCtx = std::make_shared<fizz::client::FizzClientContext>();
    auto* sockPtr = sock.get();
    fizzClient.reset(
        new fizz::client::AsyncFizzClient(std::move(sock), fizzCtx));
    sockPtr->connect(&cb, address, /*timeout=*/std::chrono::seconds{5}.count());
  });

  done.wait();
  if (err) {
    return nullptr;
  }
  return folly::AsyncTransport::UniquePtr(fizzClient.release());
}

// Performs a fizz handshake that negotiates StopTLS V1 via the Thrift TLS
// extension. After the server tears down TLS, surfaces the underlying
// plaintext folly::AsyncSocket via connect()'s return value.
class FizzStopTLSConnector
    : public fizz::client::AsyncFizzClient::HandshakeCallback,
      public fizz::AsyncFizzBase::EndOfTLSCallback {
 public:
  ~FizzStopTLSConnector() override {
    // AsyncFizzClient uses DelayedDestruction — must be released on the
    // EVB thread that drove the handshake.
    if (client_ && evb_) {
      evb_->runInEventBaseThread([c = std::move(client_)]() mutable {});
    }
  }

  // Initiates the fizz handshake on `evb`'s thread and blocks the caller
  // until the StopTLS downgrade completes. Caller MUST NOT be running on
  // `evb`'s thread (this would deadlock by double-driving the EventBase).
  folly::AsyncSocket::UniquePtr connect(
      const folly::SocketAddress& address, folly::EventBase* evb) {
    evb_ = evb;

    evb->runInEventBaseThreadAndWait([&] {
      auto sock = folly::AsyncSocket::newSocket(evb_, address);
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
          /*verifier=*/nullptr,
          /*sni=*/folly::none,
          /*pskIdentity=*/folly::none,
          folly::Optional<std::vector<fizz::ech::ParsedECHConfig>>(folly::none),
          /*timeout=*/std::chrono::seconds{5});
    });
    return std::move(promise_).getSemiFuture().get();
  }

  void fizzHandshakeSuccess(
      fizz::client::AsyncFizzClient* client) noexcept override {
    client->setEndOfTLSCallback(this);
  }

  void fizzHandshakeError(
      fizz::client::AsyncFizzClient* /*unused*/,
      folly::exception_wrapper ex) noexcept override {
    promise_.setException(std::move(ex));
  }

  void endOfTLS(
      fizz::AsyncFizzBase* transport,
      std::unique_ptr<folly::IOBuf> /*postData*/) override {
    auto* sock = transport->getUnderlyingTransport<folly::AsyncSocket>();
    DCHECK(sock);
    auto fd = sock->detachNetworkSocket();
    auto zcId = sock->getZeroCopyBufId();
    auto plaintext =
        folly::AsyncSocket::UniquePtr(new folly::AsyncSocket(evb_, fd, zcId));
    promise_.setValue(std::move(plaintext));
  }

 private:
  fizz::client::AsyncFizzClient::UniquePtr client_;
  folly::Promise<folly::AsyncSocket::UniquePtr> promise_;
  folly::EventBase* evb_{nullptr};
};

} // namespace

class FastThriftServerTest : public ::testing::Test {
 protected:
  void SetUp() override {
    THRIFT_FLAG_SET_MOCK(rocket_client_binary_rpc_metadata_encoding, true);

    handler_ = std::make_shared<TestHandler>();

    ftt::FastThriftServerConfig config;
    config.address = folly::SocketAddress("::1", 0);
    config.numIOThreads = 1;

    server_ = std::make_unique<ftt::FastThriftServer>(std::move(config));
    server_->setInterface(handler_);
    server_->start();

    clientThread_ = std::make_unique<folly::ScopedEventBaseThread>();
  }

  void TearDown() override {
    clientThread_.reset();
    server_->stop();
    server_.reset();
  }

  std::unique_ptr<apache::thrift::Client<integration::FastThriftServer>>
  createClient() {
    auto* evb = clientThread_->getEventBase();
    std::unique_ptr<apache::thrift::Client<integration::FastThriftServer>>
        client;
    evb->runInEventBaseThreadAndWait([&] {
      auto socket = folly::AsyncSocket::newSocket(evb, server_->getAddress());
      auto channel =
          apache::thrift::RocketClientChannel::newChannel(std::move(socket));
      client = std::make_unique<
          apache::thrift::Client<integration::FastThriftServer>>(
          std::move(channel));
    });
    return client;
  }

  void destroyClientOnEvb(
      std::unique_ptr<apache::thrift::Client<integration::FastThriftServer>>&
          client) {
    clientThread_->getEventBase()->runInEventBaseThreadAndWait(
        [&] { client.reset(); });
  }

  // Runs a semifuture-returning lambda on the client EventBase and blocks
  // until the result is available. Returns the value on success and fails
  // the test on RPC error.
  template <typename Fn>
  auto syncCall(Fn&& fn) {
    using SemiFuture = std::invoke_result_t<Fn>;
    using T = typename SemiFuture::value_type;
    if constexpr (std::is_void_v<T>) {
      folly::Baton<> baton;
      auto* evb = clientThread_->getEventBase();
      evb->runInEventBaseThread([&] {
        fn().via(evb)
            .thenValue([&](folly::Unit) { baton.post(); })
            .thenError([&](const folly::exception_wrapper& ew) {
              ADD_FAILURE() << "RPC failed: " << folly::exceptionStr(ew);
              baton.post();
            });
      });
      baton.wait();
    } else {
      folly::Baton<> baton;
      T result{};
      auto* evb = clientThread_->getEventBase();
      evb->runInEventBaseThread([&] {
        fn().via(evb)
            .thenValue([&](T val) {
              result = std::move(val);
              baton.post();
            })
            .thenError([&](const folly::exception_wrapper& ew) {
              ADD_FAILURE() << "RPC failed: " << folly::exceptionStr(ew);
              baton.post();
            });
      });
      baton.wait();
      return result;
    }
  }

  // Like syncCall but expects the RPC to fail with the templated exception
  // type. Returns void; fails the test if the RPC succeeds or throws a
  // different exception type.
  template <typename ExpectedEx, typename Fn>
  void syncCallExpectException(Fn&& fn) {
    folly::Baton<> baton;
    bool gotExpected = false;
    std::string actual;
    auto* evb = clientThread_->getEventBase();
    evb->runInEventBaseThread([&] {
      fn().via(evb)
          .thenValue([&](auto&&) {
            ADD_FAILURE() << "RPC succeeded; expected exception "
                          << typeid(ExpectedEx).name();
            baton.post();
          })
          .thenError([&](const folly::exception_wrapper& ew) {
            if (ew.with_exception([](const ExpectedEx&) {})) {
              gotExpected = true;
            } else {
              actual = folly::exceptionStr(ew).toStdString();
            }
            baton.post();
          });
    });
    baton.wait();
    EXPECT_TRUE(gotExpected)
        << "Expected " << typeid(ExpectedEx).name() << " got: " << actual;
  }

  std::shared_ptr<TestHandler> handler_;
  std::unique_ptr<ftt::FastThriftServer> server_;
  std::unique_ptr<folly::ScopedEventBaseThread> clientThread_;
};

// ---------------------------------------------------------------------------
// Success paths over real loopback rocket client
// ---------------------------------------------------------------------------

TEST_F(FastThriftServerTest, PingRoundTrip) {
  auto client = createClient();
  syncCall([&] { return client->semifuture_ping(); });
  destroyClientOnEvb(client);
}

TEST_F(FastThriftServerTest, AddReturnsSum) {
  auto client = createClient();
  auto result = syncCall([&] { return client->semifuture_add(7, 35); });
  EXPECT_EQ(result, 42);
  destroyClientOnEvb(client);
}

TEST_F(FastThriftServerTest, EchoReturnsComplex) {
  auto client = createClient();
  auto resp = syncCall([&] { return client->semifuture_echo("hello"); });
  EXPECT_EQ(*resp.message(), "echoed:hello");
  destroyClientOnEvb(client);
}

// ---------------------------------------------------------------------------
// Declared exception path round-trips a typed exception
// ---------------------------------------------------------------------------

TEST_F(FastThriftServerTest, LookupDeclaredExceptionPropagates) {
  handler_->throwNotFound = true;
  auto client = createClient();
  syncCallExpectException<NotFoundException>(
      [&] { return client->semifuture_lookup(/*id=*/99); });
  destroyClientOnEvb(client);
}

TEST_F(FastThriftServerTest, SecureLookupSecondExceptionPropagates) {
  handler_->throwPermissionDenied = true;
  auto client = createClient();
  syncCallExpectException<PermissionDeniedException>([&] {
    return client->semifuture_secureLookup(/*id=*/5, std::string("alice"));
  });
  destroyClientOnEvb(client);
}

// ---------------------------------------------------------------------------
// TLS coverage — FastThriftServer with FizzServerConfig.
// ---------------------------------------------------------------------------

class FastThriftServerTlsTest : public ::testing::Test {
 protected:
  void SetUp() override {
    THRIFT_FLAG_SET_MOCK(rocket_client_binary_rpc_metadata_encoding, true);

    cert_ = security::test::makeTestCert();
    handler_ = std::make_shared<TestHandler>();

    ftt::FastThriftServerConfig config;
    config.address = folly::SocketAddress("::1", 0);
    config.numIOThreads = 1;

    server_ = std::make_unique<ftt::FastThriftServer>(std::move(config));

    security::FizzServerCertConfig sslConfig;
    sslConfig.certPem = cert_.certPem;
    sslConfig.keyPem = cert_.keyPem;
    // Skip mTLS — covered by the unit tests on FizzServerContextBuilder.
    sslConfig.clientAuth = fizz::server::ClientAuthMode::None;
    server_->setSSLConfig(std::move(sslConfig));

    server_->setInterface(handler_);
    server_->start();

    clientThread_ = std::make_unique<folly::ScopedEventBaseThread>();
  }

  void TearDown() override {
    clientThread_.reset();
    server_->stop();
    server_.reset();
  }

  security::test::TestCert cert_;
  std::shared_ptr<TestHandler> handler_;
  std::unique_ptr<ftt::FastThriftServer> server_;
  std::unique_ptr<folly::ScopedEventBaseThread> clientThread_;
};

TEST_F(FastThriftServerTlsTest, RoundTripOverTls) {
  auto* evb = clientThread_->getEventBase();
  auto transport = connectFizz(evb, server_->getAddress());
  ASSERT_NE(transport, nullptr);

  std::unique_ptr<apache::thrift::Client<integration::FastThriftServer>> client;
  evb->runInEventBaseThreadAndWait([&] {
    auto channel =
        apache::thrift::RocketClientChannel::newChannel(std::move(transport));
    client =
        std::make_unique<apache::thrift::Client<integration::FastThriftServer>>(
            std::move(channel));
  });

  folly::Baton<> done;
  EchoResponse echoed;
  folly::exception_wrapper rpcErr;
  evb->runInEventBaseThread([&] {
    client->semifuture_echo("over fizz")
        .via(evb)
        .thenValue([&](EchoResponse r) {
          echoed = std::move(r);
          done.post();
        })
        .thenError([&](const folly::exception_wrapper& ew) {
          rpcErr = ew;
          done.post();
        });
  });
  ASSERT_TRUE(done.try_wait_for(std::chrono::seconds{10}));
  EXPECT_FALSE(rpcErr) << rpcErr.what();
  EXPECT_EQ(*echoed.message(), "echoed:over fizz");

  evb->runInEventBaseThreadAndWait([&] { client.reset(); });
}

TEST_F(FastThriftServerTlsTest, PlaintextClientFailsToConnect) {
  // A non-TLS client connects but its first frame will not parse as a
  // ClientHello; the server's fizz handshake fails and the connection is
  // dropped. The RPC should error out (not hang or succeed).
  auto* evb = clientThread_->getEventBase();
  std::unique_ptr<apache::thrift::Client<integration::FastThriftServer>> client;
  evb->runInEventBaseThreadAndWait([&] {
    auto socket = folly::AsyncSocket::newSocket(evb, server_->getAddress());
    auto channel =
        apache::thrift::RocketClientChannel::newChannel(std::move(socket));
    client =
        std::make_unique<apache::thrift::Client<integration::FastThriftServer>>(
            std::move(channel));
  });

  folly::Baton<> done;
  folly::exception_wrapper rpcErr;
  evb->runInEventBaseThread([&] {
    client->semifuture_ping()
        .via(evb)
        .thenValue([&](folly::Unit) { done.post(); })
        .thenError([&](const folly::exception_wrapper& ew) {
          rpcErr = ew;
          done.post();
        });
  });
  ASSERT_TRUE(done.try_wait_for(std::chrono::seconds{10}));
  EXPECT_TRUE(rpcErr) << "plaintext client should fail against TLS server";

  evb->runInEventBaseThreadAndWait([&] { client.reset(); });
}

// ---------------------------------------------------------------------------
// STOPTLS V1 — server tears down TLS after handshake; RPCs continue plaintext
// over the same FD with peer/self cert info preserved.
// ---------------------------------------------------------------------------

class FastThriftServerStopTlsTest : public ::testing::Test {
 protected:
  void SetUp() override {
    THRIFT_FLAG_SET_MOCK(rocket_client_binary_rpc_metadata_encoding, true);

    cert_ = security::test::makeTestCert();
    handler_ = std::make_shared<TestHandler>();

    ftt::FastThriftServerConfig config;
    config.address = folly::SocketAddress("::1", 0);
    config.numIOThreads = 1;

    server_ = std::make_unique<ftt::FastThriftServer>(std::move(config));

    security::FizzServerCertConfig sslConfig;
    sslConfig.certPem = cert_.certPem;
    sslConfig.keyPem = cert_.keyPem;
    sslConfig.clientAuth = fizz::server::ClientAuthMode::None;
    server_->setSSLConfig(std::move(sslConfig));

    security::ThriftTlsConfig thriftConfig;
    thriftConfig.enableStopTLS = true;
    server_->setThriftConfig(std::move(thriftConfig));

    server_->setInterface(handler_);
    server_->start();

    clientThread_ = std::make_unique<folly::ScopedEventBaseThread>();
  }

  void TearDown() override {
    clientThread_.reset();
    server_->stop();
    server_.reset();
  }

  security::test::TestCert cert_;
  std::shared_ptr<TestHandler> handler_;
  std::unique_ptr<ftt::FastThriftServer> server_;
  std::unique_ptr<folly::ScopedEventBaseThread> clientThread_;
};

TEST_F(FastThriftServerStopTlsTest, FallsBackToTLSWhenClientDoesNotRequest) {
  // Server has enableStopTLS=true, but the client uses the plain fizz
  // connector (no Thrift extension). The server's extension's
  // getNegotiatedStopTLS() must return false, and the connection must
  // continue over the encrypted fizz transport. Catches a regression where
  // the server tears down TLS based on its own config rather than the
  // negotiation result.
  auto* evb = clientThread_->getEventBase();
  auto transport = connectFizz(evb, server_->getAddress());
  ASSERT_NE(transport, nullptr);

  std::unique_ptr<apache::thrift::Client<integration::FastThriftServer>> client;
  evb->runInEventBaseThreadAndWait([&] {
    auto channel =
        apache::thrift::RocketClientChannel::newChannel(std::move(transport));
    client =
        std::make_unique<apache::thrift::Client<integration::FastThriftServer>>(
            std::move(channel));
  });

  folly::Baton<> done;
  EchoResponse echoed;
  folly::exception_wrapper rpcErr;
  evb->runInEventBaseThread([&] {
    client->semifuture_echo("over fizz")
        .via(evb)
        .thenValue([&](EchoResponse r) {
          echoed = std::move(r);
          done.post();
        })
        .thenError([&](const folly::exception_wrapper& ew) {
          rpcErr = ew;
          done.post();
        });
  });
  ASSERT_TRUE(done.try_wait_for(std::chrono::seconds{10}));
  EXPECT_FALSE(rpcErr) << rpcErr.what();
  EXPECT_EQ(*echoed.message(), "echoed:over fizz");

  evb->runInEventBaseThreadAndWait([&] { client.reset(); });
}

TEST_F(FastThriftServerStopTlsTest, RoundTripAfterStopTLSDowngrade) {
  auto* evb = clientThread_->getEventBase();

  // connect() runs the fizz handshake on the EVB thread and blocks here
  // until StopTLS downgrade completes. Must NOT run inside a
  // runInEventBaseThread lambda (would double-drive evb).
  FizzStopTLSConnector connector;
  auto plaintext = connector.connect(server_->getAddress(), evb);
  ASSERT_NE(plaintext, nullptr);

  std::unique_ptr<apache::thrift::Client<integration::FastThriftServer>> client;
  evb->runInEventBaseThreadAndWait([&] {
    auto channel = apache::thrift::RocketClientChannel::newChannel(
        folly::AsyncTransport::UniquePtr(plaintext.release()));
    client =
        std::make_unique<apache::thrift::Client<integration::FastThriftServer>>(
            std::move(channel));
  });

  folly::Baton<> done;
  EchoResponse echoed;
  folly::exception_wrapper rpcErr;
  evb->runInEventBaseThread([&] {
    client->semifuture_echo("after stoptls")
        .via(evb)
        .thenValue([&](EchoResponse r) {
          echoed = std::move(r);
          done.post();
        })
        .thenError([&](const folly::exception_wrapper& ew) {
          rpcErr = ew;
          done.post();
        });
  });
  ASSERT_TRUE(done.try_wait_for(std::chrono::seconds{10}));
  EXPECT_FALSE(rpcErr) << rpcErr.what();
  EXPECT_EQ(*echoed.message(), "echoed:after stoptls");

  evb->runInEventBaseThreadAndWait([&] { client.reset(); });
}

} // namespace apache::thrift::fast_thrift::thrift::test::integration::test
