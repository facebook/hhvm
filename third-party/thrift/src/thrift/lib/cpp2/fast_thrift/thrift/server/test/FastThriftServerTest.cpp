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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <cstddef>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>

#include <fizz/client/AsyncFizzClient.h>
#include <fizz/client/FizzClientContext.h>
#include <folly/ExceptionWrapper.h>
#include <folly/SocketAddress.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/ScopedEventBaseThread.h>
#include <folly/synchronization/Baton.h>

#include <thrift/lib/cpp2/Flags.h>
#include <thrift/lib/cpp2/async/RocketClientChannel.h>
#include <thrift/lib/cpp2/async/RpcOptions.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/security/FizzServerCertConfig.h>
#include <thrift/lib/cpp2/fast_thrift/security/test/TestCert.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/FastThriftServer.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/extension/ThriftExtension.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/extension/ThriftExtensionPipelineHandler.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/framework/FastServerModule.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/framework/NativeThriftHandlerAllowlist.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/framework/ThriftPipelineHandler.h>
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

// Handler that retains the ping callback instead of completing it. Tests
// drive completion on their own schedule, after stop() has begun, to
// reproduce the race where an in-flight FHC would write back through a
// freed adapter.
class DeferredPingHandler
    : public FastServiceHandler<integration::FastThriftServer> {
 public:
  void async_eb_ping(ftt::FastHandlerCallbackPtr<void> cb) override {
    evb_ = cb->getEventBase();
    callback_ = std::move(cb);
    pingStarted_.post();
  }

  ftt::FastHandlerCallbackPtr<void> takeCallback() {
    return std::move(callback_);
  }

  folly::EventBase* getEventBase() const { return evb_; }

  folly::Baton<> pingStarted_;

 private:
  ftt::FastHandlerCallbackPtr<void> callback_;
  folly::EventBase* evb_{nullptr};
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
    if (enableChecksum_) {
      // The checksum handler records the response algorithm on the per-request
      // context, so it requires request-context wiring.
      config.enableRequestContext = true;
      config.enableChecksum = true;
    }

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
  bool enableChecksum_{false};
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
// Checksum handler wired into the real server pipeline (enableChecksum).
// ---------------------------------------------------------------------------

class FastThriftServerChecksumTest : public FastThriftServerTest {
 protected:
  void SetUp() override {
    enableChecksum_ = true;
    FastThriftServerTest::SetUp();
  }
};

// With the checksum handler in the pipeline, ordinary (no-checksum) requests
// must still round-trip untouched.
TEST_F(FastThriftServerChecksumTest, NonChecksumTrafficRoundTrips) {
  auto client = createClient();
  auto resp = syncCall([&] { return client->semifuture_echo("plain"); });
  EXPECT_EQ(*resp.message(), "echoed:plain");
  destroyClientOnEvb(client);
}

// A request carrying an XXH3_64 checksum is validated by the server handler and
// the response echoes a checksum the client accepts — a full round-trip.
TEST_F(FastThriftServerChecksumTest, XXH3ChecksumRequestRoundTrips) {
  auto client = createClient();
  apache::thrift::RpcOptions options;
  options.setChecksum(apache::thrift::RpcOptions::Checksum::XXH3_64);
  auto resp =
      syncCall([&] { return client->semifuture_echo(options, "checksummed"); });
  EXPECT_EQ(*resp.message(), "echoed:checksummed");
  destroyClientOnEvb(client);
}

// Regression for the lifetime bug fixed by graceful drain. Before the fix,
// stop() synchronously dropped per-connection FastConnection state
// (including the ThriftServerAppAdapter) while a handler still held a
// FastHandlerCallbackPtr. The deferred completion then wrote a response
// through the freed adapter — a UAF caught by ASAN/TSAN. After the fix,
// stop() drives onException on every adapter;
// ThriftServerConnectionCloseHandler holds the pipeline open until in-flight
// responses drain, and stop() blocks on connectionsDrainedBaton_ until every
// adapter has fired its closeCallback. The retained callback can safely
// complete during stop.
TEST(
    FastThriftServerStandaloneTest,
    DeferredPingCallbackAfterStopDoesNotUseFreedAdapter) {
  THRIFT_FLAG_SET_MOCK(rocket_client_binary_rpc_metadata_encoding, true);

  auto handler = std::make_shared<DeferredPingHandler>();
  ftt::FastThriftServerConfig config;
  config.address = folly::SocketAddress("::1", 0);
  config.numIOThreads = 1;

  ftt::FastThriftServer server(std::move(config));
  server.setInterface(handler);
  server.start();

  folly::ScopedEventBaseThread clientThread;
  auto* evb = clientThread.getEventBase();
  std::unique_ptr<apache::thrift::Client<integration::FastThriftServer>> client;
  evb->runInEventBaseThreadAndWait([&] {
    auto socket = folly::AsyncSocket::newSocket(evb, server.getAddress());
    auto channel =
        apache::thrift::RocketClientChannel::newChannel(std::move(socket));
    client =
        std::make_unique<apache::thrift::Client<integration::FastThriftServer>>(
            std::move(channel));
  });

  folly::Baton<> clientDone;
  evb->runInEventBaseThread([&] {
    client->semifuture_ping()
        .via(evb)
        .thenValue([&](folly::Unit) { clientDone.post(); })
        .thenError([&](const folly::exception_wrapper&) { clientDone.post(); });
  });

  ASSERT_TRUE(handler->pingStarted_.try_wait_for(std::chrono::seconds{5}));

  std::thread stopThread([&] { server.stop(); });

  // stop() may close the wire, but the retained in-flight callback must not
  // dereference a freed per-connection adapter when it completes.
  ASSERT_NE(handler->getEventBase(), nullptr);
  handler->getEventBase()->runInEventBaseThreadAndWait([&] {
    auto callback = handler->takeCallback();
    ASSERT_NE(callback, nullptr);
    std::move(callback)->done();
  });
  stopThread.join();

  evb->runInEventBaseThreadAndWait([&] { client.reset(); });
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

// ---------------------------------------------------------------------------
// Shared IO thread pool — embedder-supplied pool via setIOThreadPool.
// ---------------------------------------------------------------------------

namespace {

ftt::FastThriftServerConfig makeLoopbackConfig() {
  ftt::FastThriftServerConfig config;
  config.address = folly::SocketAddress("::1", 0);
  // Set to a value distinct from the embedder pool's size to make it obvious
  // in tests that the supplied pool is what gets used, not
  // config_.numIOThreads.
  config.numIOThreads = 4;
  return config;
}

// Build a min==max IO pool. setIOThreadPool requires this shape; encoding
// the convention in a test helper keeps the test sites tight.
std::shared_ptr<folly::IOThreadPoolExecutor> makeFixedSizePool(size_t threads) {
  return std::make_shared<folly::IOThreadPoolExecutor>(
      /*maxThreads=*/threads, /*minThreads=*/threads);
}

namespace cp = ::apache::thrift::fast_thrift::channel_pipeline;

// Observer extension: counts the requests and responses it sees, then lets
// them continue. When given a shared globalSeq counter, it records the sequence
// number of its first request into firstRequestSeq; extensions registered in
// order see strictly increasing first-request sequences (requests flow
// head→tail), which pins registration order == pipeline order.
struct CountingExtension {
  std::atomic<int>* requests{nullptr};
  std::atomic<int>* responses{nullptr};
  std::atomic<int>* globalSeq{nullptr};
  std::atomic<int>* firstRequestSeq{nullptr};

  ftt::RequestVerdict onRequest(
      const ftt::ThriftRequestView& /*request*/) noexcept {
    if (requests) {
      requests->fetch_add(1, std::memory_order_relaxed);
    }
    if (globalSeq && firstRequestSeq) {
      int seq = globalSeq->fetch_add(1, std::memory_order_relaxed);
      int expected = -1;
      firstRequestSeq->compare_exchange_strong(expected, seq);
    }
    return ftt::RequestVerdict::proceed();
  }

  void onResponse(const ftt::ThriftResponseView& /*response*/) noexcept {
    if (responses) {
      responses->fetch_add(1, std::memory_order_relaxed);
    }
  }
};

// Read/write extension that rejects every request. Reads the method name and
// stamps a header (exercising both halves of the mutator's access) then returns
// a rejecting verdict, which short-circuits the pipeline: the service handler
// must never run and the client receives an application error.
struct RejectingExtension {
  std::atomic<int>* rejections{nullptr};

  ftt::RequestVerdict onRequest(ftt::ThriftRequestMutator& request) noexcept {
    request.setHeader(
        "x-rejected-by-extension", std::string(request.methodName()));
    if (rejections) {
      rejections->fetch_add(1, std::memory_order_relaxed);
    }
    return ftt::RequestVerdict::reject<std::runtime_error>(
        "denied by extension");
  }
};

// Drives one `add(7, 35)` RPC against `addr` and returns the result, so tests
// that only care about exercising the server pipeline stay tight.
int64_t addRoundTrip(const folly::SocketAddress& addr) {
  folly::ScopedEventBaseThread clientThread;
  auto* evb = clientThread.getEventBase();
  std::unique_ptr<apache::thrift::Client<integration::FastThriftServer>> client;
  evb->runInEventBaseThreadAndWait([&] {
    auto socket = folly::AsyncSocket::newSocket(evb, addr);
    auto channel =
        apache::thrift::RocketClientChannel::newChannel(std::move(socket));
    client =
        std::make_unique<apache::thrift::Client<integration::FastThriftServer>>(
            std::move(channel));
  });

  folly::Baton<> done;
  int64_t sum = 0;
  evb->runInEventBaseThread([&] {
    if (client == nullptr) {
      ADD_FAILURE() << "client was not constructed";
      done.post();
      return;
    }
    client->semifuture_add(7, 35)
        .via(evb)
        .thenValue([&](int64_t v) {
          sum = v;
          done.post();
        })
        .thenError([&](const folly::exception_wrapper& ew) {
          ADD_FAILURE() << "RPC failed: " << folly::exceptionStr(ew);
          done.post();
        });
  });
  EXPECT_TRUE(done.try_wait_for(std::chrono::seconds{10}));
  evb->runInEventBaseThreadAndWait([&] { client.reset(); });
  return sum;
}

// Drives one `add(7, 35)` RPC against `addr` and returns true iff the RPC
// failed (errored) rather than returning a value. Used by reject tests.
bool addRoundTripFails(const folly::SocketAddress& addr) {
  folly::ScopedEventBaseThread clientThread;
  auto* evb = clientThread.getEventBase();
  std::unique_ptr<apache::thrift::Client<integration::FastThriftServer>> client;
  evb->runInEventBaseThreadAndWait([&] {
    auto socket = folly::AsyncSocket::newSocket(evb, addr);
    auto channel =
        apache::thrift::RocketClientChannel::newChannel(std::move(socket));
    client =
        std::make_unique<apache::thrift::Client<integration::FastThriftServer>>(
            std::move(channel));
  });

  folly::Baton<> done;
  bool failed = false;
  evb->runInEventBaseThread([&] {
    if (client == nullptr) {
      ADD_FAILURE() << "client was not constructed";
      done.post();
      return;
    }
    client->semifuture_add(7, 35)
        .via(evb)
        .thenValue([&](int64_t) { done.post(); })
        .thenError([&](const folly::exception_wrapper&) {
          failed = true;
          done.post();
        });
  });
  EXPECT_TRUE(done.try_wait_for(std::chrono::seconds{10}));
  evb->runInEventBaseThreadAndWait([&] { client.reset(); });
  return failed;
}

ftt::FastThriftServerConfig makeConnectionContextConfig() {
  auto config = makeLoopbackConfig();
  // Connection extensions observe the per-connection context, so the server
  // must be building one.
  config.enableRequestContext = true;
  return config;
}

// Shared sink for what a connection extension observed. A fresh extension is
// constructed per connection, so the recorder lives in the test and every
// instance points at it.
struct ConnectionRecorder {
  std::atomic<int> seq{0};
  std::atomic<int> attempted{-1};
  std::atomic<int> established{-1};
  std::atomic<int> request{-1};
  std::atomic<int> closed{0};
  std::atomic<int> negotiatedVersion{0};
  std::atomic<bool> sawClientSetup{false};

  // Stamps `slot` with the next global sequence number, keeping the first
  // stamp only, so ordering assertions read the first connection's lifecycle
  // even if the test opens more.
  void stamp(std::atomic<int>& slot) noexcept {
    int next = seq.fetch_add(1, std::memory_order_relaxed);
    int expected = -1;
    slot.compare_exchange_strong(expected, next);
  }
};

// Records every lifecycle point plus the request, so a single connection's
// callback order can be compared against the first request it carries.
struct ConnectionLifecycleExtension {
  ConnectionRecorder* rec{nullptr};

  ftt::ConnectionVerdict onConnectionAttempted(
      const ftt::ThriftSetupConnectionView& conn) noexcept {
    rec->stamp(rec->attempted);
    rec->negotiatedVersion.store(
        conn.negotiatedVersion(), std::memory_order_relaxed);
    rec->sawClientSetup.store(
        conn.clientSetup().maxVersion().has_value(), std::memory_order_relaxed);
    return ftt::ConnectionVerdict::proceed();
  }

  ftt::ConnectionVerdict onConnectionEstablished(
      const ftt::ThriftConnectionView& /*conn*/) noexcept {
    rec->stamp(rec->established);
    return ftt::ConnectionVerdict::proceed();
  }

  void onConnectionClosed(const ftt::ThriftConnectionView& /*conn*/) noexcept {
    rec->closed.fetch_add(1, std::memory_order_relaxed);
  }

  ftt::RequestVerdict onRequest(
      const ftt::ThriftRequestView& /*request*/) noexcept {
    rec->stamp(rec->request);
    return ftt::RequestVerdict::proceed();
  }
};

// Refuses at the first decision point. Also records the later points, so a
// test can assert they never run.
struct RejectAtAttemptedExtension {
  ConnectionRecorder* rec{nullptr};

  ftt::ConnectionVerdict onConnectionAttempted(
      const ftt::ThriftSetupConnectionView& /*conn*/) noexcept {
    rec->stamp(rec->attempted);
    return ftt::ConnectionVerdict::reject<std::runtime_error>(
        "denied at connection attempt");
  }

  ftt::ConnectionVerdict onConnectionEstablished(
      const ftt::ThriftConnectionView& /*conn*/) noexcept {
    rec->stamp(rec->established);
    return ftt::ConnectionVerdict::proceed();
  }

  ftt::RequestVerdict onRequest(
      const ftt::ThriftRequestView& /*request*/) noexcept {
    rec->stamp(rec->request);
    return ftt::RequestVerdict::proceed();
  }
};

// Contributes a security policy to the SETUP response, the way an
// authorization extension declares its enforcement level to the client.
struct PolicyContributingExtension {
  ConnectionRecorder* rec{nullptr};

  ftt::ConnectionVerdict onConnectionAttempted(
      ftt::ThriftSetupConnectionMutator& /*conn*/) noexcept {
    rec->stamp(rec->attempted);
    return ftt::ConnectionVerdict::proceed();
  }

  // Contributed on the way out: the response does not exist yet at
  // onConnectionAttempted.
  void onConnectionAnswering(
      ftt::ThriftSetupResponseMutator& response) noexcept {
    apache::thrift::SecurityPolicy policy;
    policy.authorization() = apache::thrift::SecurityPolicyStatus::ENFORCED;
    policy.authWall() = apache::thrift::SecurityPolicyStatus::ENABLED;
    response.setSecurityPolicy(std::move(policy));
  }
};

// Hooks only the teardown point — an extension is free to implement any subset
// of the callbacks, including no request callback at all.
struct ClosedOnlyExtension {
  ConnectionRecorder* rec{nullptr};

  void onConnectionClosed(const ftt::ThriftConnectionView& /*conn*/) noexcept {
    rec->closed.fetch_add(1, std::memory_order_relaxed);
  }
};

// Spins until `value` reaches `target` or the deadline passes. Connection
// teardown completes asynchronously after the client goes away, so tests that
// assert on it cannot read the counter straight through.
bool waitFor(const std::atomic<int>& value, int target) {
  const auto deadline =
      std::chrono::steady_clock::now() + std::chrono::seconds{10};
  while (std::chrono::steady_clock::now() < deadline) {
    if (value.load(std::memory_order_relaxed) >= target) {
      return true;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds{10});
  }
  return false;
}

} // namespace

// Embedder pool actually drives accept + dispatch end-to-end.
TEST(FastThriftServerSharedPoolTest, RoundTripWithEmbedderPool) {
  THRIFT_FLAG_SET_MOCK(rocket_client_binary_rpc_metadata_encoding, true);

  auto handler = std::make_shared<TestHandler>();
  auto pool = makeFixedSizePool(2);

  ftt::FastThriftServer server(makeLoopbackConfig());
  server.setIOThreadPool(pool);
  server.setInterface(handler);
  server.start();

  folly::ScopedEventBaseThread clientThread;
  auto* evb = clientThread.getEventBase();
  std::unique_ptr<apache::thrift::Client<integration::FastThriftServer>> client;
  evb->runInEventBaseThreadAndWait([&] {
    auto socket = folly::AsyncSocket::newSocket(evb, server.getAddress());
    auto channel =
        apache::thrift::RocketClientChannel::newChannel(std::move(socket));
    client =
        std::make_unique<apache::thrift::Client<integration::FastThriftServer>>(
            std::move(channel));
  });

  folly::Baton<> done;
  int64_t sum = 0;
  evb->runInEventBaseThread([&] {
    client->semifuture_add(7, 35)
        .via(evb)
        .thenValue([&](int64_t v) {
          sum = v;
          done.post();
        })
        .thenError([&](const folly::exception_wrapper& ew) {
          ADD_FAILURE() << "RPC failed: " << folly::exceptionStr(ew);
          done.post();
        });
  });
  ASSERT_TRUE(done.try_wait_for(std::chrono::seconds{10}));
  EXPECT_EQ(sum, 42);
  evb->runInEventBaseThreadAndWait([&] { client.reset(); });
}

// The per-connection context carries the peer address captured when the socket
// was accepted, not one re-derived from the transport at build time.
TEST(FastThriftServerConnContextTest, PeerAddressReachesConnContext) {
  THRIFT_FLAG_SET_MOCK(rocket_client_binary_rpc_metadata_encoding, true);

  auto handler = std::make_shared<TestHandler>();
  auto config = makeLoopbackConfig();
  config.enableRequestContext = true;

  ftt::FastThriftServer server(std::move(config));
  server.setInterface(handler);

  folly::Baton<> accepted;
  folly::SocketAddress observedPeer;
  server.setOnConnectionAccepted([&](ftt::ThriftConnContext* connContext) {
    if (connContext != nullptr) {
      observedPeer = connContext->getPeerAddress();
    }
    accepted.post();
  });
  server.start();

  folly::ScopedEventBaseThread clientThread;
  auto* evb = clientThread.getEventBase();
  std::shared_ptr<folly::AsyncSocket> socket;
  folly::SocketAddress clientLocalAddr;
  evb->runInEventBaseThreadAndWait([&] {
    socket = folly::AsyncSocket::newSocket(evb, server.getAddress());
    socket->getLocalAddress(&clientLocalAddr);
  });

  ASSERT_TRUE(accepted.try_wait_for(std::chrono::seconds{10}));
  EXPECT_EQ(observedPeer, clientLocalAddr);

  evb->runInEventBaseThreadAndWait([&] { socket.reset(); });
}

// An observer extension registered via addThriftExtension is spliced into every
// per-connection thrift pipeline and sees the inbound request and the outbound
// response of a real RPC without disturbing it.
TEST(FastThriftServerExtensionTest, ObserverExtensionObservesTraffic) {
  THRIFT_FLAG_SET_MOCK(rocket_client_binary_rpc_metadata_encoding, true);

  auto handler = std::make_shared<TestHandler>();
  std::atomic<int> requests{0};
  std::atomic<int> responses{0};

  ftt::FastThriftServer server(makeLoopbackConfig());
  server.setInterface(handler);
  server.addModule(
      ftt::FastServerModule("observer")
          .addThriftExtension<CountingExtension>(
              &requests, &responses, nullptr, nullptr));
  server.start();

  EXPECT_EQ(addRoundTrip(server.getAddress()), 42);
  EXPECT_GE(requests.load(), 1);
  EXPECT_GE(responses.load(), 1);
}

// Extensions are spliced in registration order (head→tail), across modules and
// within a module: module "first" (A) runs before module "second" (B, C), and
// within "second" B runs before C. First-request sequence numbers pin
// registration order == pipeline order.
TEST(FastThriftServerExtensionTest, ExtensionsSplicedInRegistrationOrder) {
  THRIFT_FLAG_SET_MOCK(rocket_client_binary_rpc_metadata_encoding, true);

  std::atomic<int> g{0};
  std::atomic<int> seqA{-1};
  std::atomic<int> seqB{-1};
  std::atomic<int> seqC{-1};

  auto handler = std::make_shared<TestHandler>();
  ftt::FastThriftServer server(makeLoopbackConfig());
  server.setInterface(handler);

  server.addModule(
      ftt::FastServerModule("first").addThriftExtension<CountingExtension>(
          nullptr, nullptr, &g, &seqA));
  server.addModule(
      ftt::FastServerModule("second")
          .addThriftExtension<CountingExtension>(nullptr, nullptr, &g, &seqB)
          .addThriftExtension<CountingExtension>(nullptr, nullptr, &g, &seqC));
  server.start();

  EXPECT_EQ(addRoundTrip(server.getAddress()), 42);
  EXPECT_GE(seqA.load(), 0);
  EXPECT_GE(seqB.load(), 0);
  EXPECT_GE(seqC.load(), 0);
  EXPECT_LT(seqA.load(), seqB.load());
  EXPECT_LT(seqB.load(), seqC.load());
}

// The native-handler allowlist denies arbitrary user types and always permits
// the framework's own extension adapter — so addThriftExtension needs no
// allowlist entry while a raw user handler would be rejected at compile time by
// makeThriftPipelineHandlerFactory's static_assert.
TEST(FastThriftServerExtensionTest, NativeHandlerAllowlistGate) {
  static_assert(
      !ftt::server::kIsAllowedNativeThriftHandler<CountingExtension>,
      "a raw user type must not be an allowlisted native handler");
  static_assert(
      ftt::server::kIsAllowedNativeThriftHandler<
          ftt::server::ThriftExtensionPipelineHandler<CountingExtension>>,
      "the framework extension adapter must always be allowlisted");
}

// ThriftRequestMutator derives from ThriftRequestView, so a mutator binds to a
// const-view parameter but not vice versa. The adapter relies on that asymmetry
// to dispatch: it tests the view form first, which is the only reason a
// read/write extension is not silently handed a read-only view.
TEST(FastThriftServerExtensionTest, ExtensionDispatchDiscriminatesAccess) {
  static_assert(
      ftt::HasRequestViewCallback<CountingExtension>,
      "a read-only extension must match the view form");
  static_assert(
      ftt::HasRequestMutatorCallback<RejectingExtension>,
      "a read/write extension must match the mutator form");
  static_assert(
      !ftt::HasRequestViewCallback<RejectingExtension>,
      "a read/write extension must not match the view form, or view-first "
      "dispatch would strip its write access");
}

// A read/write extension that rejects short-circuits the pipeline: the client
// receives an error and the service handler never runs.
TEST(FastThriftServerExtensionTest, ModifierRejectionShortCircuitsService) {
  THRIFT_FLAG_SET_MOCK(rocket_client_binary_rpc_metadata_encoding, true);

  auto handler = std::make_shared<TestHandler>();
  std::atomic<int> rejections{0};

  ftt::FastThriftServer server(makeLoopbackConfig());
  server.setInterface(handler);
  server.addModule(
      ftt::FastServerModule("reject").addThriftExtension<RejectingExtension>(
          &rejections));
  server.start();

  EXPECT_TRUE(addRoundTripFails(server.getAddress()));
  EXPECT_GE(rejections.load(), 1);
}

// The connection callbacks bracket the SETUP exchange: attempted fires once
// the client's setup metadata is parsed and version-negotiated, established
// once the SETUP response is away, and both land before the first request.
// A negotiated version and a non-null client setup prove the events are driven
// by the real SETUP frame rather than by socket accept.
TEST(FastThriftServerConnectionExtensionTest, LifecycleBracketsSetupExchange) {
  THRIFT_FLAG_SET_MOCK(rocket_client_binary_rpc_metadata_encoding, true);

  auto handler = std::make_shared<TestHandler>();
  ConnectionRecorder rec;

  ftt::FastThriftServer server(makeConnectionContextConfig());
  server.setInterface(handler);
  server.addModule(
      ftt::FastServerModule("lifecycle")
          .addThriftExtension<ConnectionLifecycleExtension>(&rec));
  server.start();

  EXPECT_EQ(addRoundTrip(server.getAddress()), 42);

  EXPECT_GE(rec.attempted.load(), 0);
  EXPECT_LT(rec.attempted.load(), rec.established.load());
  EXPECT_LT(rec.established.load(), rec.request.load());
  EXPECT_GT(rec.negotiatedVersion.load(), 0);
  EXPECT_TRUE(rec.sawClientSetup.load());
}

// Refusing at the first decision point short-circuits: no SETUP response is
// built, the established callback never runs, and the client's RPC fails.
TEST(FastThriftServerConnectionExtensionTest, AttemptedRejectionRefusesSetup) {
  THRIFT_FLAG_SET_MOCK(rocket_client_binary_rpc_metadata_encoding, true);

  auto handler = std::make_shared<TestHandler>();
  ConnectionRecorder rec;

  ftt::FastThriftServer server(makeConnectionContextConfig());
  server.setInterface(handler);
  server.addModule(
      ftt::FastServerModule("reject-attempt")
          .addThriftExtension<RejectAtAttemptedExtension>(&rec));
  server.start();

  EXPECT_TRUE(addRoundTripFails(server.getAddress()));
  EXPECT_GE(rec.attempted.load(), 0);
  EXPECT_EQ(rec.established.load(), -1);
  EXPECT_EQ(rec.request.load(), -1);
}

// An extension may hook teardown alone, with no request callback. The closed
// callback fires once the connection the client dropped has settled.
TEST(FastThriftServerConnectionExtensionTest, ClosedFiresForSettledConnection) {
  THRIFT_FLAG_SET_MOCK(rocket_client_binary_rpc_metadata_encoding, true);

  auto handler = std::make_shared<TestHandler>();
  ConnectionRecorder rec;

  ftt::FastThriftServer server(makeConnectionContextConfig());
  server.setInterface(handler);
  server.addModule(
      ftt::FastServerModule("closed-only")
          .addThriftExtension<ClosedOnlyExtension>(&rec));
  server.start();

  // addRoundTrip tears its client down before returning, so the server-side
  // close is already in flight by the time we start waiting.
  EXPECT_EQ(addRoundTrip(server.getAddress()), 42);
  EXPECT_TRUE(waitFor(rec.closed, 1));
}

// An extension can contribute to the SETUP response the client receives, not
// just accept or refuse the connection. Dispatch must hand it a mutator: the
// contribution has to land before the response is serialized.
TEST(FastThriftServerConnectionExtensionTest, ContributesToSetupResponse) {
  static_assert(
      ftt::HasConnectionAttemptedMutatorCallback<PolicyContributingExtension>,
      "a contributing extension must match the mutator form");
  static_assert(
      !ftt::HasConnectionAttemptedViewCallback<PolicyContributingExtension>,
      "a contributing extension must not match the view form, or view-first "
      "dispatch would strip its write access");
  static_assert(
      ftt::HasConnectionAttemptedViewCallback<ConnectionLifecycleExtension>,
      "a read-only extension must match the view form");

  THRIFT_FLAG_SET_MOCK(rocket_client_binary_rpc_metadata_encoding, true);

  auto handler = std::make_shared<TestHandler>();
  ConnectionRecorder rec;

  ftt::FastThriftServer server(makeConnectionContextConfig());
  server.setInterface(handler);
  server.addModule(
      ftt::FastServerModule("policy")
          .addThriftExtension<PolicyContributingExtension>(&rec));
  server.start();

  // The contribution must not disturb the exchange: the RPC still completes.
  EXPECT_EQ(addRoundTrip(server.getAddress()), 42);
  EXPECT_GE(rec.attempted.load(), 0);
}

// A connection extension observes the per-connection context, so registering
// one against a server that builds none is refused outright rather than
// silently handing the extension an empty connection.
TEST(FastThriftServerConnectionExtensionTest, RequiresRequestContext) {
  ConnectionRecorder rec;
  ftt::FastThriftServer server(makeLoopbackConfig());
  EXPECT_THROW(
      server.addModule(
          ftt::FastServerModule("needs-context")
              .addThriftExtension<ConnectionLifecycleExtension>(&rec)),
      std::logic_error);
}

// Callback families are independent: a teardown-only extension is a valid
// extension with no request callback, and an extension that hooks no
// connection event subscribes to nothing, so it is linked into no event list.
TEST(FastThriftServerConnectionExtensionTest, CallbackFamiliesAreIndependent) {
  static_assert(
      !ftt::ThriftExtensionHandler<ClosedOnlyExtension>,
      "a teardown-only extension has no request callback");
  static_assert(
      ftt::ThriftConnectionExtensionHandler<ClosedOnlyExtension>,
      "a teardown-only extension is still a connection extension");
  static_assert(
      !ftt::ThriftConnectionExtensionHandler<CountingExtension>,
      "a request/response extension hooks no connection event");
  static_assert(
      std::is_same_v<
          std::remove_const_t<
              decltype(ftt::server::ThriftExtensionPipelineHandler<
                       CountingExtension>::kSubscribedEvents)>,
          cp::Subscriptions<>>,
      "an extension with no connection callback must subscribe to no events");
}

// Registering two modules with the same name on the server is rejected.
TEST(FastThriftServerPipelineHandlerTest, DuplicateModuleNameThrows) {
  ftt::FastThriftServer server(makeLoopbackConfig());
  server.addModule(ftt::FastServerModule("dup"));
  EXPECT_THROW(
      server.addModule(ftt::FastServerModule("dup")), std::logic_error);
}

// Empty module name is rejected — the empty namespace is reserved for
// loose-handler ids.
TEST(FastThriftServerPipelineHandlerTest, EmptyModuleNameThrows) {
  ftt::FastThriftServer server(makeLoopbackConfig());
  EXPECT_THROW(server.addModule(ftt::FastServerModule("")), std::logic_error);
}

// The two-level id derivation yields a distinct id for every (namespace,
// index) pair — no cross-namespace or arithmetic-adjacency collisions.
TEST(FastServerModuleTest, DerivedHandlerIdsAreUniqueAcrossNamespaces) {
  const std::vector<std::string_view> namespaces = {
      "", "a", "b", "moduleA", "moduleB", "logging", "metrics"};
  std::set<cp::HandlerId> ids;
  std::size_t count = 0;
  for (auto ns : namespaces) {
    for (std::size_t i = 0; i < 32; ++i) {
      ids.insert(ftt::server::deriveThriftPipelineHandlerId(ns, i));
      ++count;
    }
  }
  EXPECT_EQ(ids.size(), count);
}

// Server holds a reference to the embedder pool while running and releases
// it on destruction. The dtor joins the pool, so embedders must shut down
// all consumers of a shared pool together (same contract as ThriftServer).
TEST(FastThriftServerSharedPoolTest, ServerReleasesPoolOnDestroy) {
  auto pool = makeFixedSizePool(1);
  ASSERT_EQ(pool.use_count(), 1);

  {
    auto handler = std::make_shared<TestHandler>();
    ftt::FastThriftServer server(makeLoopbackConfig());
    server.setIOThreadPool(pool);
    server.setInterface(handler);
    server.start();
    EXPECT_EQ(pool.use_count(), 2);
    server.stop();
  }

  EXPECT_EQ(pool.use_count(), 1);
}

TEST(FastThriftServerSharedPoolTest, NullPoolCrashes) {
  ftt::FastThriftServer server(makeLoopbackConfig());
  EXPECT_DEATH(
      server.setIOThreadPool(nullptr),
      "FastThriftServer::setIOThreadPool requires a non-null pool");
}

TEST(FastThriftServerSharedPoolTest, AfterStartCrashes) {
  // start() spawns IO worker threads. Default "fast" death-test style only
  // forks; any glog/folly logging mutex held by a worker at fork time stays
  // locked in the child and the CHECK message never reaches stderr, so the
  // regex match fails even though the child does abort. "threadsafe" style
  // forks + execs, which re-runs the binary clean.
  GTEST_FLAG_SET(death_test_style, "threadsafe");
  auto handler = std::make_shared<TestHandler>();
  ftt::FastThriftServer server(makeLoopbackConfig());
  server.setInterface(handler);
  server.start();
  EXPECT_DEATH(
      server.setIOThreadPool(makeFixedSizePool(1)),
      "FastThriftServer::setIOThreadPool must be called before");
  server.stop();
}

// ---------------------------------------------------------------------------
// Hot-reload — reloadTLSConfig swaps the fizz context on a running server.
// ---------------------------------------------------------------------------

TEST_F(FastThriftServerTlsTest, HotReloadServesNewCert) {
  // Establish baseline: RPC works with the cert SetUp installed.
  auto* evb = clientThread_->getEventBase();
  {
    auto transport = connectFizz(evb, server_->getAddress());
    ASSERT_NE(transport, nullptr);
    std::unique_ptr<apache::thrift::Client<integration::FastThriftServer>>
        client;
    evb->runInEventBaseThreadAndWait([&] {
      auto channel =
          apache::thrift::RocketClientChannel::newChannel(std::move(transport));
      client = std::make_unique<
          apache::thrift::Client<integration::FastThriftServer>>(
          std::move(channel));
    });
    folly::Baton<> done;
    EchoResponse echoed;
    folly::exception_wrapper rpcErr;
    evb->runInEventBaseThread([&] {
      client->semifuture_echo("pre-reload")
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
    EXPECT_EQ(*echoed.message(), "echoed:pre-reload");
    evb->runInEventBaseThreadAndWait([&] { client.reset(); });
  }

  // Hot-reload with a freshly-generated cert. The swap fans out across
  // every EVB; future accepts use the new params, in-flight ones (none
  // here) keep the old via captured shared_ptr.
  auto newCert = security::test::makeTestCert();
  security::FizzServerCertConfig newConfig;
  newConfig.certPem = newCert.certPem;
  newConfig.keyPem = newCert.keyPem;
  newConfig.clientAuth = fizz::server::ClientAuthMode::None;
  server_->reloadTLSConfig(std::move(newConfig));

  // New RPC must still succeed. The server is serving the new cert
  // (verifying that explicitly would require client-side peer cert
  // inspection; here we settle for "swap didn't break anything").
  {
    auto transport = connectFizz(evb, server_->getAddress());
    ASSERT_NE(transport, nullptr);
    std::unique_ptr<apache::thrift::Client<integration::FastThriftServer>>
        client;
    evb->runInEventBaseThreadAndWait([&] {
      auto channel =
          apache::thrift::RocketClientChannel::newChannel(std::move(transport));
      client = std::make_unique<
          apache::thrift::Client<integration::FastThriftServer>>(
          std::move(channel));
    });
    folly::Baton<> done;
    EchoResponse echoed;
    folly::exception_wrapper rpcErr;
    evb->runInEventBaseThread([&] {
      client->semifuture_echo("post-reload")
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
    EXPECT_EQ(*echoed.message(), "echoed:post-reload");
    evb->runInEventBaseThreadAndWait([&] { client.reset(); });
  }
}

TEST(FastThriftServerHotReloadTest, RequiresRunningServer) {
  // reloadTLSConfig before start() must CHECK-fail.
  GTEST_FLAG_SET(death_test_style, "threadsafe");
  auto handler = std::make_shared<TestHandler>();
  ftt::FastThriftServer server(makeLoopbackConfig());
  server.setInterface(handler);
  // No start() — server is in kNotStarted.

  auto cert = security::test::makeTestCert();
  security::FizzServerCertConfig cfg;
  cfg.certPem = cert.certPem;
  cfg.keyPem = cert.keyPem;
  cfg.clientAuth = fizz::server::ClientAuthMode::None;
  EXPECT_DEATH(
      server.reloadTLSConfig(std::move(cfg)),
      "FastThriftServer::reloadTLSConfig requires a running server");
}

} // namespace apache::thrift::fast_thrift::thrift::test::integration::test
