/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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
#include <fizz/client/AsyncFizzClient.h>
#include <fizz/client/SynchronizedLruPskCache.h>
#include <fizz/crypto/test/TestUtil.h>
#include <fizz/experimental/ktls/AsyncFizzBaseKTLS.h>
#include <fizz/protocol/Certificate.h>
#include <fizz/protocol/test/Mocks.h>
#include <fizz/server/AeadTicketCipher.h>
#include <fizz/server/AsyncFizzServer.h>
#include <fizz/server/CertManager.h>
#include <fizz/server/FizzServerContext.h>
#include <fizz/server/TicketCodec.h>

#include <folly/Function.h>
#include <folly/futures/Future.h>
#include <folly/io/async/AsyncServerSocket.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/test/TestUtils.h>

using namespace ::testing;
using namespace fizz;

class ServerAcceptor : public folly::AsyncServerSocket::AcceptCallback {
 public:
  using SuccessFn = void(folly::AsyncSocket::UniquePtr) noexcept;
  using FailFn = void(const std::exception&) noexcept;

  explicit ServerAcceptor(
      folly::EventBase* evb,
      folly::Function<SuccessFn> successFn,
      folly::Function<FailFn> failFn)
      : success_(std::move(successFn)), fail_(std::move(failFn)) {
    serverSocket_.reset(new folly::AsyncServerSocket(evb));
    serverSocket_->bind(0);
    serverSocket_->listen(10);
    serverSocket_->addAcceptCallback(this, nullptr);
    serverSocket_->startAccepting();
  }
  void connectionAccepted(
      folly::NetworkSocket fd,
      const folly::SocketAddress&,
      AcceptInfo /* info */) noexcept override {
    auto sock =
        folly::AsyncSocket::newSocket(serverSocket_->getEventBase(), fd);
    if (success_) {
      success_(std::move(sock));
    }
  }

  void stopAccepting() {
    serverSocket_->stopAccepting();
  }

  folly::SocketAddress getLocalAddress() const {
    return serverSocket_->getAddress();
  }

  void acceptError(const std::exception& ex) noexcept override {
    if (fail_) {
      fail_(ex);
    }
  }

 private:
  folly::AsyncServerSocket::UniquePtr serverSocket_;
  folly::Function<SuccessFn> success_;
  folly::Function<FailFn> fail_;
};

template <class T>
static fizz::AsyncKTLSSocket::UniquePtr mustConvertKTLS(T& sock) {
  auto result = fizz::tryConvertKTLS(sock);
  if (!result) {
    throw std::runtime_error("tryConvertKTLS failed");
  }
  return std::move(result).value();
}

using namespace fizz::server;
using namespace fizz::client;

folly::SemiFuture<AsyncFizzServer::UniquePtr> fizzAccept(
    AsyncFizzServer::UniquePtr socket) {
  struct HandshakeAwaiter : public AsyncFizzServer::HandshakeCallback {
    explicit HandshakeAwaiter(AsyncFizzServer::UniquePtr sock)
        : fizzSocket_(std::move(sock)) {}

    folly::SemiFuture<AsyncFizzServer::UniquePtr> await() {
      auto [p, f] = folly::makePromiseContract<AsyncFizzServer::UniquePtr>();
      promise_ = std::move(p);

      fizzSocket_->accept(this);
      return std::move(f);
    }

    void fizzHandshakeSuccess(AsyncFizzServer*) noexcept override {
      promise_.setValue(std::move(fizzSocket_));
      delete this;
    }

    void fizzHandshakeError(
        AsyncFizzServer*,
        folly::exception_wrapper ex) noexcept override {
      promise_.setException(std::move(ex));
      delete this;
    }

    void fizzHandshakeAttemptFallback(
        AttemptVersionFallback) noexcept override {
      fizzHandshakeError(
          fizzSocket_.get(),
          folly::make_exception_wrapper<std::runtime_error>(
              "can only handle TLS 1.3"));
    }

    AsyncFizzServer::UniquePtr fizzSocket_;
    folly::Promise<AsyncFizzServer::UniquePtr> promise_;
  };

  HandshakeAwaiter* awaiter = new HandshakeAwaiter(std::move(socket));
  return awaiter->await();
}

folly::SemiFuture<fizz::client::AsyncFizzClient::UniquePtr> fizzConnect(
    AsyncFizzClient::UniquePtr socket,
    const folly::SocketAddress& addr,
    std::string pskIdentity) {
  struct ConnectAwaiter : public folly::AsyncSocket::ConnectCallback {
    explicit ConnectAwaiter(
        AsyncFizzClient::UniquePtr sock,
        const folly::SocketAddress& addr,
        std::string&& pskIdentity)
        : fizzSocket_(std::move(sock)),
          addr_(addr),
          pskIdentity_(std::move(pskIdentity)) {}

    folly::SemiFuture<AsyncFizzClient::UniquePtr> await() {
      auto [p, f] = folly::makePromiseContract<AsyncFizzClient::UniquePtr>();
      promise_ = std::move(p);

      fizzSocket_->connect(addr_, this, nullptr, folly::none, pskIdentity_);
      return std::move(f);
    }

    void connectSuccess() noexcept override {
      promise_.setValue(std::move(fizzSocket_));
      delete this;
    }

    void connectErr(const folly::AsyncSocketException& ex) noexcept override {
      promise_.setException(ex);
      delete this;
    }

    AsyncFizzClient::UniquePtr fizzSocket_;
    folly::SocketAddress addr_;
    folly::Promise<AsyncFizzClient::UniquePtr> promise_;
    std::string pskIdentity_;
  };

  ConnectAwaiter* awaiter =
      new ConnectAwaiter(std::move(socket), addr, std::move(pskIdentity));
  return awaiter->await();
}

template <class T>
class OneshotRead : public folly::AsyncTransport::ReadCallback {
 public:
  using UniquePtr = typename T::UniquePtr;
  using FutValueType = std::pair<UniquePtr, std::unique_ptr<folly::IOBuf>>;

  OneshotRead(UniquePtr t, size_t siz)
      : transport_(std::move(t)), buf_(folly::IOBuf::create(siz)) {}

  void getReadBuffer(void** bufReturn, size_t* lenReturn) override {
    *bufReturn = buf_->writableData();
    *lenReturn = buf_->tailroom();
  }

  void readDataAvailable(size_t len) noexcept override {
    transport_->setReadCB(nullptr);
    buf_->append(len);
    promise_.setValue(std::make_pair(std::move(transport_), std::move(buf_)));
    delete this;
  }

  void readEOF() noexcept override {
    transport_->setReadCB(nullptr);
    promise_.setValue(std::make_pair(std::move(transport_), nullptr));
    delete this;
  }

  void readErr(const folly::AsyncSocketException& ex) noexcept override {
    transport_->setReadCB(nullptr);
    promise_.setException(ex);
    delete this;
  }

  folly::SemiFuture<FutValueType> await() {
    transport_->setReadCB(this);
    return promise_.getSemiFuture();
  }

 private:
  UniquePtr transport_;
  std::unique_ptr<folly::IOBuf> buf_;
  folly::Promise<FutValueType> promise_;
};

static std::shared_ptr<fizz::server::FizzServerContext>
makeTestServerContext() {
  auto certmanager = std::make_shared<fizz::server::CertManager>();
  certmanager->addCert(
      CertUtils::makeSelfCert(
          fizz::test::kP256Certificate.str(), fizz::test::kP256Key.str()),
      true);

  auto factory = std::make_shared<fizz::test::MockFactory>();
  auto certManager = std::make_shared<CertManager>();
  auto ticketCipher = std::make_shared<
      Aead128GCMTicketCipher<TicketCodec<CertificateStorage::X509>>>(
      std::move(factory), std::move(certManager));
  static constexpr folly::StringPiece ticketSecret =
      "this is the incredibly secure ticket cipher secret";
  ticketCipher->setTicketSecrets({folly::range(ticketSecret)});

  auto ctx = std::make_shared<fizz::server::FizzServerContext>();
  ctx->setVersionFallbackEnabled(false);
  ctx->setCertManager(certmanager);
  ctx->setTicketCipher(ticketCipher);
  return ctx;
}

static std::shared_ptr<fizz::client::FizzClientContext>
makeTestClientContext() {
  auto pskCache = std::make_shared<fizz::client::SynchronizedLruPskCache>(5);
  auto ctx = std::make_shared<fizz::client::FizzClientContext>();
  ctx->setPskCache(pskCache);
  ctx->setSupportedCiphers({CipherSuite::TLS_AES_128_GCM_SHA256});
  return ctx;
}

// Test a Fizz server upgrading to a KTLS socket, communicating with a Fizz
// client.
TEST(AsyncFizzBaseKTLSTest, TestFizzClientKTLSServer) {
  folly::EventBase evb;

  auto ctx = makeTestServerContext();

  std::function<void()> stopServer;

  auto handleClient =
      [&](folly::AsyncSocket::UniquePtr sock) mutable noexcept -> void {
    AsyncFizzServer::UniquePtr fizzSock;
    fizzSock.reset(new AsyncFizzServer(std::move(sock), ctx));
    fizzSock->setHandshakeRecordAlignedReads(true);
    fizzAccept(std::move(fizzSock))
        .via(&evb)
        .thenValue([](AsyncFizzServer::UniquePtr fizzSock) {
          VLOG(1) << "finished handshaking";
          auto ktlsSocket = mustConvertKTLS(*fizzSock);

          EXPECT_NE(ktlsSocket->getSelfCertificate(), nullptr);
          EXPECT_EQ(ktlsSocket->getSelfCertificate()->getIdentity(), "Fizz");
          EXPECT_EQ(ktlsSocket->getPeerCertificate(), nullptr);
          ktlsSocket->dropSelfCertificate();
          EXPECT_EQ(ktlsSocket->getSelfCertificate(), nullptr);

          ktlsSocket->write(
              nullptr, "hello from ktls", sizeof("hello from ktls") - 1);
          auto read =
              new OneshotRead<AsyncKTLSSocket>(std::move(ktlsSocket), 512);
          return read->await();
        })
        .thenValue([](auto&& res) {
          auto& [t, data] = res;
          ASSERT_TRUE(data.get());
          auto value = data->moveToFbString().toStdString();
          VLOG(1) << "ktls read: " << value;
          EXPECT_EQ(value, "hello from fizz");
        })
        .thenError(
            [](auto&& ew) { VLOG(1) << "fizz handshake error: " << ew.what(); })
        .ensure([&] { stopServer(); });
  };

  ServerAcceptor acceptor(
      &evb, handleClient, [](auto&&) noexcept { ASSERT_FALSE(true); });
  stopServer = [&] { acceptor.stopAccepting(); };

  VLOG(1) << "Server listening on " << acceptor.getLocalAddress().describe();
  auto clientCtx = makeTestClientContext();
  AsyncFizzClient::UniquePtr fizzClient;
  fizzClient.reset(new AsyncFizzClient(&evb, clientCtx));
  fizzConnect(std::move(fizzClient), acceptor.getLocalAddress(), "ktls_server")
      .via(&evb)
      .thenValue([](AsyncFizzClient::UniquePtr fizzSock) {
        auto read = new OneshotRead<AsyncFizzClient>(std::move(fizzSock), 512);
        return read->await();
      })
      .via(&evb)
      .thenValue([](auto&& res) {
        auto& [t, data] = res;
        ASSERT_TRUE(data.get());
        auto value = data->moveToFbString().toStdString();
        VLOG(1) << "client received: " << value;
        EXPECT_EQ(value, "hello from ktls");

        t->write(nullptr, "hello from fizz", sizeof("hello from fizz") - 1);
      });
  ;
  evb.loop();
}

// Test a Fizz client upgrading to a KTLS socket, communicating with a "server-
// writes-first" Fizz server.
TEST(AsyncFizzBaseKTLSTest, TestKTLSClientFizzServer) {
  folly::EventBase evb;

  auto serverCtx = makeTestServerContext();

  std::function<void()> stopServer;

  auto handleClient =
      [&](folly::AsyncSocket::UniquePtr sock) mutable noexcept -> void {
    AsyncFizzServer::UniquePtr fizzSock;
    fizzSock.reset(new AsyncFizzServer(std::move(sock), serverCtx));
    fizzSock->setHandshakeRecordAlignedReads(false);

    fizzAccept(std::move(fizzSock))
        .via(&evb)
        .thenValue([](AsyncFizzServer::UniquePtr fizzSock) {
          VLOG(1) << "finished handshaking";
          fizzSock->write(
              nullptr, "hello from fizz", sizeof("hello from fizz") - 1);
          auto read =
              new OneshotRead<AsyncFizzServer>(std::move(fizzSock), 512);
          return read->await();
        })
        .thenValue([](auto&& res) {
          auto& [t, data] = res;
          ASSERT_TRUE(data.get());
          auto value = data->moveToFbString().toStdString();
          VLOG(1) << "fizz server read: " << value;
          EXPECT_EQ(value, "hello from ktls");
        })
        .thenError([](auto&& ew) { VLOG(1) << "server error: " << ew.what(); })
        .ensure([&] { stopServer(); });
  };

  ServerAcceptor acceptor(
      &evb, handleClient, [](auto&&) noexcept { ASSERT_FALSE(true); });
  stopServer = [&] { acceptor.stopAccepting(); };

  auto clientCtx = makeTestClientContext();

  {
    AsyncFizzClient::UniquePtr fizzClient;
    fizzClient.reset(new AsyncFizzClient(&evb, clientCtx));
    fizzClient->setHandshakeRecordAlignedReads(true);
    fizzConnect(
        std::move(fizzClient), acceptor.getLocalAddress(), "ktls_client")
        .via(&evb)
        .thenValue([](AsyncFizzClient::UniquePtr fizzSock) {
          VLOG(1) << "fizz client connected";
          auto ktlsSocket = mustConvertKTLS(*fizzSock);

          EXPECT_EQ(ktlsSocket->getSelfCertificate(), nullptr);
          EXPECT_NE(ktlsSocket->getPeerCertificate(), nullptr);
          EXPECT_EQ(ktlsSocket->getPeerCertificate()->getIdentity(), "Fizz");
          ktlsSocket->dropPeerCertificate();
          EXPECT_EQ(ktlsSocket->getPeerCertificate(), nullptr);
          auto read =
              new OneshotRead<AsyncKTLSSocket>(std::move(ktlsSocket), 512);
          return read->await();
        })
        .via(&evb)
        .thenValue([](auto&& res) {
          auto& [t, data] = res;
          ASSERT_TRUE(data.get());
          auto value = data->moveToFbString().toStdString();
          VLOG(1) << "client received: " << value;
          EXPECT_EQ(value, "hello from fizz");
          t->write(nullptr, "hello from ktls", sizeof("hello from ktls") - 1);
        });
  }

  evb.loop();

  auto cachedPsk = clientCtx->getPsk("ktls_client");
  ASSERT_TRUE(cachedPsk.has_value());

  auto handshakeAndAssertResumption =
      [&](folly::AsyncSocket::UniquePtr sock) mutable noexcept -> void {
    AsyncFizzServer::UniquePtr fizzSock;
    fizzSock.reset(new AsyncFizzServer(std::move(sock), serverCtx));
    fizzAccept(std::move(fizzSock))
        .via(&evb)
        .thenValue([](auto&& sock) {
          auto pskType = sock->getState().pskType();
          ASSERT_TRUE(pskType.has_value());
          EXPECT_EQ(pskType.value(), PskType::Resumption);
        })
        .ensure([&] { stopServer(); });
  };

  ServerAcceptor acceptor2(
      &evb, handshakeAndAssertResumption, [](auto&&) noexcept {
        ASSERT_FALSE(true);
      });
  stopServer = [&] { acceptor2.stopAccepting(); };

  {
    AsyncFizzClient::UniquePtr fizzClient;
    fizzClient.reset(new AsyncFizzClient(&evb, clientCtx));
    fizzClient->setHandshakeRecordAlignedReads(true);
    fizzConnect(
        std::move(fizzClient), acceptor2.getLocalAddress(), "ktls_client")
        .via(&evb)
        .thenValue([](auto&& sock) {
          auto pskType = sock->getState().pskType();
          ASSERT_TRUE(pskType.has_value());
          EXPECT_EQ(pskType.value(), PskType::Resumption);
        });
  }
}
