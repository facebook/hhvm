/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/http/coro/transport/CoroSSLTransport.h"
#include <folly/logging/xlog.h>

#include <folly/Portability.h>

#include <folly/coro/BlockingWait.h>
#include <folly/coro/Collect.h>
#include <folly/coro/Sleep.h>
#include <folly/io/async/ssl/BasicTransportCertificate.h>
#include <folly/io/async/test/AsyncSSLSocketTest.h>
#include <folly/io/async/test/TestSSLServer.h>
#include <folly/io/coro/ServerSocket.h>
#include <folly/io/coro/Transport.h>
#include <folly/portability/GTest.h>
#include <folly/testing/TestUtil.h>

#include "proxygen/lib/http/coro/transport/test/TestCoroTransport.h"

using namespace std::chrono_literals;
using namespace folly;
using namespace folly::coro;
using namespace proxygen::coro::test;
using folly::ssl::OpenSSLUtils;
using folly::test::find_resource;

template <size_t SIZE>
folly::coro::Task<Unit> readAll(proxygen::coro::CoroSSLTransport& transport,
                                std::array<uint8_t, SIZE>& rcvBuf,
                                std::chrono::milliseconds timeout) {
  size_t totalBytes{0};
  while (totalBytes < SIZE) {
    auto bytesRead = co_await transport.read(
        MutableByteRange(rcvBuf.data() + totalBytes,
                         (rcvBuf.data() + rcvBuf.size())),
        timeout);
    totalBytes += bytesRead;
  }
  co_return unit;
};

class FlexibleWriteCallback : public folly::test::WriteCallbackBase {
 public:
  ~FlexibleWriteCallback() override {
    if (errorOk) {
      state = folly::test::STATE_SUCCEEDED;
    }
  }

  bool errorOk{false};
};

class FlexibleReadCallback : public folly::test::ReadCallback {
 public:
  explicit FlexibleReadCallback(folly::test::WriteCallbackBase* wcb)
      : ReadCallback(wcb) {
  }

  ~FlexibleReadCallback() override {
    if (errorOk) {
      state = folly::test::STATE_SUCCEEDED;
    }
  }

  bool errorOk{false};
};

class FlexibleHandshakeCallback : public folly::test::HandshakeCallback {
 public:
  explicit FlexibleHandshakeCallback(folly::test::ReadCallback* wcb)
      : HandshakeCallback(wcb) {
  }

  ~FlexibleHandshakeCallback() override {
    if (errorOk) {
      state = folly::test::STATE_SUCCEEDED;
    }
  }

  bool errorOk{false};
};

namespace proxygen::coro {

class TransportTest : public testing::Test {
 public:
  template <typename F>
  void run(F f) {
    blockingWait(co_invoke(std::move(f)), &evb);
  }

  folly::coro::Task<> requestCancellation() {
    cancelSource.requestCancellation();
    co_return;
  }

  EventBase evb;
  CancellationSource cancelSource;
};

class CoroSSLTransportTest : public TransportTest {
 public:
  CoroSSLTransportTest()
      : srv{&acceptCallback,
            folly::test::TestSSLServer::getDefaultSSLContext()} {
  }

  folly::coro::Task<std::unique_ptr<CoroSSLTransport>> connect(
      folly::Optional<folly::SocketAddress> addr = folly::none) {
    auto socket = co_await Transport::newConnectedSocket(
        &evb, addr ? *addr : srv.getAddress(), 0ms);

    auto transport = std::make_unique<CoroSSLTransport>(
        std::make_unique<folly::coro::Transport>(std::move(socket)),
        sslCtx,
        transportOptions);
    transport->setVerificationOption(verifyPeer);
    if (session) {
      transport->setSSLSession(session);
    }
    co_await transport->connect(sni, std::chrono::seconds(2));
    co_return transport;
  }
  FlexibleWriteCallback writeCallback;
  folly::Optional<folly::test::ReadErrorCallback> readErrorCallback;
  FlexibleReadCallback readCallback{&writeCallback};
  FlexibleHandshakeCallback handshakeCallback{&readCallback};
  folly::test::SSLServerAcceptCallback acceptCallback{&handshakeCallback};
  folly::test::TestSSLServer srv;

  std::shared_ptr<folly::SSLContext> sslCtx{
      std::make_shared<folly::SSLContext>()};
  folly::SSLContext::SSLVerifyPeerEnum verifyPeer{folly::SSLContext::USE_CTX};
  CoroSSLTransport::TransportOptions transportOptions;
  std::shared_ptr<ssl::SSLSession> session;
  folly::Optional<std::string> sni;
};

TEST_F(CoroSSLTransportTest, ConnectFailure) {
  folly::test::HandshakeErrorCallback handshakeErrorCallback(
      &handshakeCallback);
  folly::test::TestSSLServer badSrv(&handshakeErrorCallback);
  run([&]() -> Task<> {
    EXPECT_THROW(co_await connect(badSrv.getAddress()), AsyncSocketException);
  });
  handshakeErrorCallback.state = folly::test::STATE_SUCCEEDED;
  acceptCallback.state = folly::test::STATE_SUCCEEDED;
}

TEST_F(CoroSSLTransportTest, ConnectSuccess) {
  run([&]() -> Task<> {
    sni = "example.com";
    auto cs = co_await connect();
    EXPECT_EQ(srv.getAddress(), cs->getPeerAddress());
    readCallback.setState(folly::test::STATE_SUCCEEDED);
    auto socket = handshakeCallback.getSocket();
    EXPECT_EQ(socket->getSSLServerName(), *sni);
    folly::SocketAddress serverPeerAddr;
    socket->getPeerAddress(&serverPeerAddr);
    EXPECT_EQ(serverPeerAddr, cs->getLocalAddress());
    EXPECT_EQ(cs->getApplicationProtocol(), "");
    EXPECT_EQ(cs->getSSLVersion(), TLS1_3_VERSION);
    EXPECT_EQ(cs->getNegotiatedCipherName(),
              std::string("TLS_AES_256_GCM_SHA384"));
    co_await folly::coro::co_reschedule_on_current_executor;
  });
}

TEST_F(CoroSSLTransportTest, ConnectVerifySuccess) {
  readCallback.errorOk = true;
  writeCallback.errorOk = true;

  sslCtx->loadTrustedCertificates(find_resource(folly::test::kTestCA).c_str());
  auto verifier =
      std::make_shared<folly::test::MockCertificateIdentityVerifier>();
  transportOptions.verifier = verifier;
  verifyPeer = folly::SSLContext::VERIFY;
  EXPECT_CALL(*verifier, verifyLeaf(testing::_))
      .WillOnce(testing::Return(testing::ByMove(
          std::make_unique<folly::ssl::BasicTransportCertificate>(
              "Asox Company", nullptr))));
  run([&]() -> Task<> {
    auto cs = co_await connect();
    EXPECT_EQ(cs->getPeerCertificate()->getIdentity(), "Asox Company");
  });
}

TEST_F(CoroSSLTransportTest, ConnectResume) {
  readCallback.errorOk = true;
  writeCallback.errorOk = true;
  run([&]() -> Task<> {
    auto cs = co_await connect();
    readCallback.setState(folly::test::STATE_SUCCEEDED);
    EXPECT_FALSE(cs->getSSLSessionReused());

    constexpr auto kBufSize = 65536;

    // The following is copied from the CoroSSLTransportTest.SimpleReadWrite
    // test. Perform a round of write/read. This will ensure session tickets are
    // read when using TLS 1.3.
    std::array<uint8_t, kBufSize> rcvBuf;
    std::array<uint8_t, kBufSize> sndBuf;
    std::memset(sndBuf.data(), 'a', sndBuf.size());
    folly::coro::TransportIf::WriteInfo info;
    co_await cs->write(ByteRange(sndBuf.data(), sndBuf.data() + sndBuf.size()),
                       100ms,
                       folly::WriteFlags::NONE,
                       &info);
    EXPECT_EQ(info.bytesWritten, sndBuf.size());
    co_await readAll(*cs, rcvBuf, 0ms);
    EXPECT_EQ(0, memcmp(sndBuf.data(), rcvBuf.data(), rcvBuf.size()));
    EXPECT_EQ(co_await cs->read(folly::MutableByteRange(nullptr, nullptr)), 0);
    cs->close();

    // the session can then be reused
    session = cs->getSSLSession();
    EXPECT_TRUE(session != nullptr);

    auto cs2 = co_await connect();
    EXPECT_TRUE(cs2->getSSLSessionReused());
    cs2->close();
  });
}

TEST_F(CoroSSLTransportTest, ConnectVerifyFailure) {
  handshakeCallback.expect_ = folly::test::HandshakeCallback::EXPECT_ERROR;
  readCallback.errorOk = true;
  writeCallback.errorOk = true;

  // Socket uses context set to verify
  verifyPeer = folly::SSLContext::USE_CTX;
  sslCtx->setVerificationOption(folly::SSLContext::VERIFY);
  run([&]() -> Task<> {
    EXPECT_THROW(co_await connect(), AsyncSocketException);
  });

  // CTX wants to checkPeerName but the name mismatches
  sslCtx->authenticate(true, true, "nope");
  sslCtx->loadTrustedCertificates(find_resource(folly::test::kTestCA).c_str());
  run([&]() -> Task<> {
    EXPECT_THROW(co_await connect(), AsyncSocketException);
  });

  // Socket overrides context and verifier fails
  sslCtx->loadTrustedCertificates(find_resource(folly::test::kTestCA).c_str());
  auto verifier =
      std::make_shared<folly::test::MockCertificateIdentityVerifier>();
  transportOptions.verifier = verifier;
  verifyPeer = folly::SSLContext::VERIFY;
  EXPECT_CALL(*verifier, verifyLeaf(testing::_))
      .WillOnce(
          testing::Throw(folly::CertificateIdentityVerifierException("bad")));
  run([&]() -> Task<> {
    EXPECT_THROW(co_await connect(), AsyncSocketException);
  });
}

TEST_F(CoroSSLTransportTest, ConnectCancelled) {
  handshakeCallback.errorOk = true;
  readCallback.errorOk = true;
  writeCallback.errorOk = true;
  acceptCallback.state = folly::test::STATE_SUCCEEDED;
  run([&]() -> Task<> {
    co_await folly::coro::collectAll(
        // token would be cancelled while waiting on connect
        [&]() -> Task<> {
          EXPECT_THROW(
              co_await co_withCancellation(cancelSource.getToken(), connect()),
              OperationCancelled);
        }(),
        requestCancellation());
    // token was cancelled before read was called
    EXPECT_THROW(co_await co_withCancellation(cancelSource.getToken(),
                                              Transport::newConnectedSocket(
                                                  &evb, srv.getAddress(), 0ms)),
                 OperationCancelled);
  });
}

TEST_F(CoroSSLTransportTest, ConnectEOF) {
  class HandshakeEOFCallback : public folly::test::SSLServerAcceptCallbackBase {
   public:
    explicit HandshakeEOFCallback(folly::test::HandshakeCallback* hcb)
        : SSLServerAcceptCallbackBase(hcb) {
    }

    void connAccepted(
        const std::shared_ptr<folly::AsyncSSLSocket>& s) noexcept override {
      s->close();
      state = folly::test::STATE_SUCCEEDED;
    }
  };

  HandshakeEOFCallback handshakeEOFCallback(&handshakeCallback);
  folly::test::TestSSLServer badSrv(&handshakeEOFCallback);
  run([&]() -> Task<> {
    EXPECT_THROW(co_await connect(badSrv.getAddress()), AsyncSocketException);
  });
  acceptCallback.state = folly::test::STATE_SUCCEEDED;
  readCallback.errorOk = true;
  writeCallback.errorOk = true;
  handshakeCallback.errorOk = true;
}

TEST_F(CoroSSLTransportTest, SimpleReadWrite) {
  run([&]() -> Task<> {
    constexpr auto kBufSize = 65536;
    auto cs = co_await connect();

    // read using coroutines
    std::array<uint8_t, kBufSize> rcvBuf;
    std::array<uint8_t, kBufSize> sndBuf;
    std::memset(sndBuf.data(), 'a', sndBuf.size());

    // write use co-routine
    folly::coro::TransportIf::WriteInfo info;
    co_await cs->write(ByteRange(sndBuf.data(), sndBuf.data() + sndBuf.size()),
                       100ms,
                       folly::WriteFlags::NONE,
                       &info);
    EXPECT_EQ(info.bytesWritten, sndBuf.size());
    co_await readAll(*cs, rcvBuf, 0ms);
    EXPECT_EQ(0, memcmp(sndBuf.data(), rcvBuf.data(), rcvBuf.size()));

    // Read with 0 length returns immediately
    EXPECT_EQ(co_await cs->read(folly::MutableByteRange(nullptr, nullptr)), 0);
  });
}

TEST_F(CoroSSLTransportTest, SimpleIOBufReadWrite) {
  run([&]() -> Task<> {
    // Exactly fills a buffer mid-loop and triggers deferredReadEOF handling
    constexpr auto kBufSize = 55 * 1184;

    auto cs = co_await connect();

    std::array<uint8_t, kBufSize> sndBuf;
    std::memset(sndBuf.data(), 'a', sndBuf.size());

    // write use co-routine
    folly::coro::TransportIf::WriteInfo info;
    co_await cs->write(ByteRange(sndBuf.data(), sndBuf.data() + sndBuf.size()),
                       100ms,
                       folly::WriteFlags::NONE,
                       &info);
    EXPECT_EQ(info.bytesWritten, sndBuf.size());

    // read using coroutines
    IOBufQueue rcvBuf(IOBufQueue::cacheChainLength());
    int totalBytes{0};
    while (totalBytes < kBufSize) {
      auto bytesRead = co_await cs->read(rcvBuf, 1000, 1000, 0ms);
      totalBytes += bytesRead;
    }
    auto socket = handshakeCallback.getSocket();
    socket->getEventBase()->runInEventBaseThread([socket] { socket->close(); });
    auto bytesRead = co_await cs->read(rcvBuf, 1000, 1000, 50ms);
    EXPECT_EQ(bytesRead, 0); // closed

    auto data = rcvBuf.move();
    data->coalesce();
    EXPECT_EQ(0, memcmp(sndBuf.data(), data->data(), data->length()));
  });
}

TEST_F(CoroSSLTransportTest, ReadCancelled) {
  run([&]() -> Task<> {
    auto cs = co_await connect();
    auto reader = [&cs]() -> Task<Unit> {
      std::array<uint8_t, 1024> rcvBuf;
      EXPECT_THROW(
          co_await cs->read(
              MutableByteRange(rcvBuf.data(), (rcvBuf.data() + rcvBuf.size())),
              0ms),
          OperationCancelled);
      co_return unit;
    };

    co_await co_withCancellation(
        cancelSource.getToken(),
        folly::coro::collectAll(requestCancellation(), reader()));
    // token was cancelled before read was called
    co_await co_withCancellation(cancelSource.getToken(), reader());
    // This allows pending writes to get "flushed", so that when that when the
    // CoroSSLTransport is destroyed at the end of this scope, the close_notify
    // can be written (rather than be added to the pending writes that will
    // performed).
    co_await folly::coro::co_reschedule_on_current_executor;
  });
  readCallback.setState(folly::test::STATE_SUCCEEDED);
  writeCallback.state = folly::test::STATE_SUCCEEDED;
}

TEST_F(CoroSSLTransportTest, ReadTimeout) {
  run([&]() -> Task<> {
    auto cs = co_await connect();
    std::array<uint8_t, 1024> rcvBuf;
    EXPECT_THROW(
        co_await cs->read(
            MutableByteRange(rcvBuf.data(), (rcvBuf.data() + rcvBuf.size())),
            50ms),
        AsyncSocketException);
  });
  readCallback.setState(folly::test::STATE_SUCCEEDED);
  writeCallback.state = folly::test::STATE_SUCCEEDED;
}

TEST_F(CoroSSLTransportTest, ReadError) {
  run([&]() -> Task<> {
    auto cs = co_await connect();
    auto socket = handshakeCallback.getSocket();
    socket->getEventBase()->runInEventBaseThread([socket] {
      // closeWithReset was still giving 0 return like FIN?
      struct linger optLinger = {1, 0};
      socket->setSockOpt(SOL_SOCKET, SO_LINGER, &optLinger);
      socket->AsyncSocket::closeNow();
    });

    std::array<uint8_t, 1024> rcvBuf;
    EXPECT_THROW(
        co_await cs->read(
            MutableByteRange(rcvBuf.data(), (rcvBuf.data() + rcvBuf.size())),
            50ms),
        AsyncSocketException);
  });
  readCallback.setState(folly::test::STATE_SUCCEEDED);
  writeCallback.state = folly::test::STATE_SUCCEEDED;
}

TEST_F(CoroSSLTransportTest, SimpleWritev) {
  run([&]() -> Task<> {
    auto cs = co_await connect();

    IOBufQueue sndBuf{folly::IOBufQueue::cacheChainLength()};
    constexpr auto kBufSize = 65536;
    std::array<uint8_t, kBufSize> bufA;
    std::memset(bufA.data(), 'a', bufA.size());
    std::array<uint8_t, kBufSize> bufB;
    std::memset(bufB.data(), 'b', bufB.size());
    sndBuf.append(bufA.data(), bufA.size());
    sndBuf.append(bufB.data(), bufB.size());

    // write use co-routine
    Transport::WriteInfo info;
    co_await cs->write(sndBuf, 500ms, folly::WriteFlags::NONE, &info);
    // read
    std::array<uint8_t, kBufSize> rcvBufA;
    co_await readAll(*cs, rcvBufA, 500ms);
    EXPECT_EQ(0, memcmp(bufA.data(), rcvBufA.data(), rcvBufA.size()));
    std::array<uint8_t, kBufSize> rcvBufB;
    co_await readAll(*cs, rcvBufB, 500ms);
    EXPECT_EQ(0, memcmp(bufB.data(), rcvBufB.data(), rcvBufB.size()));
  });
}

TEST_F(CoroSSLTransportTest, WriteCancelled) {
  run([&]() -> Task<> {
    auto cs = co_await connect();
    // reduce the send buffer size so the write wouldn't complete immediately
    auto asyncSocket = dynamic_cast<folly::AsyncSocket*>(cs->getTransport());
    XCHECK(asyncSocket);
    EXPECT_EQ(asyncSocket->setSendBufSize(4096), 0);

    constexpr auto kBufSize = 65536;
    std::array<uint8_t, kBufSize> sndBuf;
    std::memset(sndBuf.data(), 'a', sndBuf.size());

    // write use co-routine
    auto writer = [&]() -> Task<> {
      EXPECT_THROW(co_await co_withCancellation(
                       cancelSource.getToken(),
                       cs->write(ByteRange(sndBuf.data(),
                                           sndBuf.data() + sndBuf.size()))),
                   OperationCancelled);
    };

    co_await folly::coro::collectAll(requestCancellation(), writer());
    co_await co_withCancellation(cancelSource.getToken(), writer());
  });
  writeCallback.errorOk = true;
  readCallback.errorOk = true;
}

TEST_F(CoroSSLTransportTest, ShutdownWrite) {
  run([&]() -> Task<> {
    auto cs = co_await connect();
    readCallback.setState(folly::test::STATE_SUCCEEDED);
    // Sends shutdown
    cs->shutdownWrite();
    IOBufQueue rcvBuf(IOBufQueue::cacheChainLength());
    // Wait for peer to shutdown
    auto bytesRead = co_await cs->read(rcvBuf, 1000, 1000, 500ms);
    EXPECT_EQ(bytesRead, 0); // closed
  });
}

TEST_F(CoroSSLTransportTest, CloseWithReadsWrites) {
  writeCallback.errorOk = true;
  readCallback.errorOk = true;
  run([&]() -> Task<> {
    auto cs = co_await connect();

    constexpr auto kBufSize = 256 * 1024;
    std::array<uint8_t, 1024> rcvBuf;
    auto readFut =
        co_withExecutor(
            &evb,
            cs->read(MutableByteRange(rcvBuf.data(),
                                      (rcvBuf.data() + rcvBuf.size())),
                     0ms))
            .start();

    std::array<uint8_t, kBufSize> sndBuf;
    std::memset(sndBuf.data(), 'a', sndBuf.size());
    auto writeFut =
        co_withExecutor(
            &evb,
            cs->write(ByteRange(sndBuf.data(), sndBuf.data() + sndBuf.size()),
                      0ms))
            .start();
    // Get the write started, but blocking
    co_await folly::coro::co_reschedule_on_current_executor;

    cs->close();
    // Writes complete normally
    co_await std::move(writeFut);
    // The read is either cancelled, or completes normally
    try {
      co_await std::move(readFut);
    } catch (OperationCancelled&) {
    }
  });
}

TEST_F(CoroSSLTransportTest, Close) {
  writeCallback.errorOk = true;
  readCallback.errorOk = true;
  run([&]() -> Task<> {
    auto cs = co_await connect();
    cs->close();
  });
}

TEST_F(CoroSSLTransportTest, CloseWithReset) {
  readCallback.state = folly::test::STATE_SUCCEEDED;
  readErrorCallback.emplace(&writeCallback);
  handshakeCallback.rcb_ = readErrorCallback.get_pointer();
  run([&]() -> Task<> {
    auto cs = co_await connect();
    cs->closeWithReset();
  });
}

TEST_F(CoroSSLTransportTest, AttemptWriteWithPendingShutdown) {
  run([&]() -> Task<> {
    auto cs = co_await connect();

    // Just enough so that we get WANT_WRITE
    constexpr auto kBufSize = 132 * 1024;
    std::array<uint8_t, kBufSize> sndBuf;
    std::memset(sndBuf.data(), 'a', sndBuf.size());
    auto writeFut =
        co_withExecutor(
            &evb,
            cs->write(ByteRange(sndBuf.data(), sndBuf.data() + sndBuf.size()),
                      0ms))
            .start();
    // Get the write started, but blocking
    co_await folly::coro::co_reschedule_on_current_executor;

    // Initiate shutdown, which is pending
    cs->shutdownWrite();

    // Attempting to write now will fail
    EXPECT_THROW(
        co_await cs->write(ByteRange(sndBuf.data(), sndBuf.data() + 1), 0ms),
        AsyncSocketException);
    IOBufQueue writeBuf{folly::IOBufQueue::cacheChainLength()};
    writeBuf.append(sndBuf.data(), 1);
    EXPECT_THROW(co_await cs->write(writeBuf, 0ms), AsyncSocketException);

    // Writes complete normally
    co_await std::move(writeFut);
    // Read everything the server echoes back
    std::array<uint8_t, kBufSize> rcvBuf;
    size_t nRead = 0;
    do {
      nRead = co_await cs->read(MutableByteRange(rcvBuf.data(), rcvBuf.size()));
    } while (nRead != 0);
  });
}

TEST_F(CoroSSLTransportTest, ResetWithPendingWrites) {
  writeCallback.errorOk = true;
  readCallback.errorOk = true;
  run([&]() -> Task<> {
    auto cs = co_await connect();

    // Just enough so that we get WANT_WRITE
    constexpr auto kBufSize = 132 * 1024;
    std::array<uint8_t, kBufSize> sndBuf;
    std::memset(sndBuf.data(), 'a', sndBuf.size());
    auto writeFut =
        co_withExecutor(
            &evb,
            cs->write(ByteRange(sndBuf.data(), sndBuf.data() + sndBuf.size()),
                      0ms))
            .start();
    // Get the write started, but blocking
    co_await folly::coro::co_reschedule_on_current_executor;
    cs->closeWithReset();
    EXPECT_THROW(co_await std::move(writeFut), AsyncSocketException);
  });
}
} // namespace proxygen::coro

namespace {

/* === BIO wrapper for TestCoroTransport === */

TestCoroTransport* transportFromBio(BIO* bio) {
  auto appData = OpenSSLUtils::getBioAppData(bio);
  XCHECK(appData);
  auto* transport = reinterpret_cast<TestCoroTransport*>(appData);
  XCHECK(transport);
  return transport;
}

int testSSLTransportBioWrite(BIO* bio, const char* buf, int sz) {
  BIO_clear_retry_flags(bio);
  transportFromBio(bio)->addReadEvent(folly::IOBuf::copyBuffer(buf, sz), false);
  return sz;
}
int testSSLTransportBioWriteEx(BIO* bio,
                               const char* buf,
                               size_t sz,
                               size_t* nw) {
  BIO_clear_retry_flags(bio);
  auto rc = testSSLTransportBioWrite(bio, buf, sz);
  if (rc >= 0) {
    *nw = rc;
    return 1;
  }
  return 0;
}
int testSSLTransportBioRead(BIO* bio, char* buf, int sz) {
  BIO_clear_retry_flags(bio);
  auto transport = transportFromBio(bio);
  int nRead = 0;
  while (sz > 0 && !transport->state_->writeEvents.empty()) {
    if (transport->state_->writeEvents.front().empty()) {
      transport->state_->writeEvents.pop_front();
      continue;
    }
    folly::io::Cursor c(transport->state_->writeEvents.front().front());
    auto readFromFront = c.pullAtMost(buf + nRead, sz);
    sz -= readFromFront;
    nRead += readFromFront;
    transport->state_->writeEvents.front().trimStart(readFromFront);
  }
  if (nRead == 0) {
    if (!transport->state_->writesClosed) {
      // need more
      BIO_set_retry_read(bio);
    }
  }
  return nRead;
}
int testSSLTransportBioReadEx(BIO* bio, char* buf, size_t sz, size_t* nr) {
  auto rc = testSSLTransportBioRead(bio, buf, sz);
  if (rc >= 0) {
    *nr = rc;
    return 1;
  }
  return 0;
}
long testSSLTransportBioCtrl(BIO*, int, long, void*) {
  return 1;
}

folly::ssl::BioMethodUniquePtr initBioMethod() {
  BIO_METHOD* newmeth = nullptr;
  newmeth = BIO_meth_new(BIO_get_new_index(), "test_ssl_transport_bio_method");
  if (!newmeth) {
    return nullptr;
  }
  BIO_meth_set_ctrl(newmeth, testSSLTransportBioCtrl);
  BIO_meth_set_read(newmeth, testSSLTransportBioRead);
  BIO_meth_set_read_ex(newmeth, testSSLTransportBioReadEx);
  BIO_meth_set_write(newmeth, testSSLTransportBioWrite);
  BIO_meth_set_write_ex(newmeth, testSSLTransportBioWriteEx);

  return folly::ssl::BioMethodUniquePtr(newmeth);
}

BIO_METHOD* getTestSSLBioMethod() {
  static auto const instance = initBioMethod().release();
  return instance;
}

} // namespace

namespace proxygen::coro {

class CoroSSLTransportFakeTest : public TransportTest {
 public:
  folly::coro::Task<std::unique_ptr<CoroSSLTransport>> connect() {
    auto testTransport =
        std::make_unique<TestCoroTransport>(&evb, &transportState_);
    testTransport_ = testTransport.get();
    auto transport = std::make_unique<CoroSSLTransport>(
        std::move(testTransport), std::make_shared<SSLContext>());

    co_withExecutor(&evb, accept()).start();
    co_await transport->connect(folly::none, std::chrono::seconds(2));
    co_return transport;
  }

  // Accept an SSL connection via the TestCoroTransport
  folly::coro::Task<void> accept() {
    ctx_ = std::make_shared<folly::SSLContext>();
    ctx_->loadCertificate(find_resource(folly::test::kTestCert).c_str());
    ctx_->loadPrivateKey(find_resource(folly::test::kTestKey).c_str());
    ctx_->ciphers("ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH");
    try {
      ssl_.reset(ctx_->createSSL());
    } catch (std::exception& ex) {
      XLOG(ERR) << "TestSSLTransport::accept(this=" << this
                << "): " << ex.what();
      throw;
    }

    if (!setupSSLBio()) {
      XLOG(ERR) << "TestSSLTransport::accept(this=" << this << "): ";
      co_return;
    }

    while (true) {
      auto rc = SSL_accept(ssl_.get());
      if (rc <= 0) {
        int sslError = SSL_get_error(ssl_.get(), rc);
        if (sslError == SSL_ERROR_WANT_READ) {
          // A bit lame, but meh
          co_await folly::coro::sleep(100ms);
        } else {
          XLOG(ERR) << "SSL_accept error=" << sslError;
          sslAccept_.post();
          break;
        }
      } else {
        XLOG(INFO) << "accepted";
        sslAccept_.post();
        break;
      }
    }
  }

  bool setupSSLBio() {
    auto sslBio = BIO_new(getTestSSLBioMethod());

    if (!sslBio) {
      return false;
    }

    OpenSSLUtils::setBioAppData(sslBio, testTransport_);
    SSL_set_bio(ssl_.get(), sslBio, sslBio);
    return true;
  }

  TestCoroTransport* testTransport_{nullptr};
  TestCoroTransport::State transportState_;
  std::shared_ptr<folly::SSLContext> ctx_;
  folly::ssl::SSLUniquePtr ssl_;
  // baton gating the successful or failed SSL_accept
  folly::coro::Baton sslAccept_;
};

TEST_F(CoroSSLTransportFakeTest, BackgroundWriteTimeout) {
  run([&]() -> Task<> {
    auto cs = co_await connect();
    testTransport_->pauseWrites();
    constexpr auto kBufSize = 4 * 1024;
    std::array<uint8_t, kBufSize> sndBuf;
    std::memset(sndBuf.data(), 'a', sndBuf.size());
    // Write appears to succeed but is running in the background, will timeout
    // and closeWithReset, cancelling the read
    co_await cs->write(ByteRange(sndBuf.data(), sndBuf.data() + sndBuf.size()),
                       100ms);

    std::array<uint8_t, 1024> rcvBuf;
    EXPECT_THROW(
        co_await cs->read(
            MutableByteRange(rcvBuf.data(), (rcvBuf.data() + rcvBuf.size())),
            0ms),
        AsyncSocketException);

    // Can't delete testTransport_ while blocked
    testTransport_->resumeWrites();
    // let the accept loop complete before destroying test objects
    co_await sslAccept_;
  });
}

TEST_F(CoroSSLTransportFakeTest, WritesBlockedTimeout) {
  run([&]() -> Task<> {
    auto cs = co_await connect();
    testTransport_->pauseWrites();
    constexpr auto kBufSize = 132 * 1024;
    std::array<uint8_t, kBufSize> sndBuf;
    std::memset(sndBuf.data(), 'a', sndBuf.size());
    EXPECT_THROW(
        co_await cs->write(
            ByteRange(sndBuf.data(), sndBuf.data() + sndBuf.size()), 500ms),
        AsyncSocketException);
    // Can't delete testTransport_ while blocked
    testTransport_->resumeWrites();
    // And have to give it 1 loop to wake up before deleting safely
    co_await folly::coro::co_reschedule_on_current_executor;
  });
}

TEST_F(CoroSSLTransportFakeTest, ProtocolError) {
  run([&]() -> Task<> {
    auto cs = co_await connect();
    // waiting for server accept to complete ensures corresponding objects
    // aren't destroyed too early
    co_await sslAccept_;
    std::array<uint8_t, 1024> rcvBuf;
    // Add garbage to the transport read buf
    testTransport_->addReadEvent(folly::IOBuf::copyBuffer("\0\0\0\0\0", 5),
                                 false);
    EXPECT_THROW(
        co_await cs->read(
            MutableByteRange(rcvBuf.data(), (rcvBuf.data() + rcvBuf.size())),
            0ms),
        AsyncSocketException);
  });
}

// This tests how CoroSSLTransport handles getting SSL_ERROR_WANT_WRITE as a
// result for SSL_read, and SSL_ERROR_WANT_READ from SSL_write by having the
// server request a rehandshake.  It also tests how CoroSSLTransport handles
// having more than one simultaneous read and write.  It's somewhat dependent on
// current openssl behavior, and could break if OpenSSL changes.
TEST_F(CoroSSLTransportFakeTest, WriteFromSSLRead) {
  run([&]() -> Task<> {
    auto cs = co_await connect();

    // Pause writes and schedule a big write to exactly fill the transport buf
    testTransport_->pauseWrites();
    constexpr auto kBufSize = 128 * 1024;
    std::array<uint8_t, kBufSize> sndBuf;
    std::memset(sndBuf.data(), 'a', sndBuf.size());
    // write returns but the next write will block
    co_await cs->write(ByteRange(sndBuf.data(), sndBuf.data() + sndBuf.size()),
                       0ms);

    // Schedule a read
    std::array<uint8_t, 1024> rcvBuf;
    auto readFut =
        co_withExecutor(
            &evb,
            cs->read(MutableByteRange(rcvBuf.data(),
                                      (rcvBuf.data() + rcvBuf.size())),
                     300ms))
            .start();

    // Wait a loop for read to start/block
    co_await folly::coro::co_reschedule_on_current_executor;

    // Initiate a renegotiation from the server side

    // SSL_read inside cs->read will return WANT_WRITE, and block on
    // writes with the read deadline (300ms)
    SSL_renegotiate(ssl_.get());
    std::array<uint8_t, kBufSize> serverReadBuf;
    // Have to call read/write to trigger actual renegotiation
    auto rc = SSL_read(ssl_.get(), serverReadBuf.data(), serverReadBuf.size());
    EXPECT_GT(rc, 0);
    size_t serverReadBytes = rc;
    co_await folly::coro::co_reschedule_on_current_executor;

    // Start another write, which will also block.  It does not increase the
    // timeout.  It also triggers a WANT_READ from inside SSL_write
    auto writeFut =
        co_withExecutor(
            &evb, cs->write(ByteRange(sndBuf.data(), sndBuf.data() + 1), 500ms))
            .start();

    // Now resume writes
    testTransport_->resumeWrites();

    // read the data and process the renegotiation
    while (serverReadBytes < kBufSize + 1 ||
           SSL_renegotiate_pending(ssl_.get())) {
      rc = SSL_read(ssl_.get(), serverReadBuf.data(), serverReadBuf.size());
      if (rc <= 0) {
        int sslError = SSL_get_error(ssl_.get(), rc);
        if (sslError == SSL_ERROR_WANT_READ) {
          // A bit lame, but meh
          co_await folly::coro::sleep(100ms);
        } else if (sslError == SSL_ERROR_ZERO_RETURN) {
          XLOG(ERR) << "SSL_read zer return";
          break;
        } else {
          XLOG(ERR) << "SSL_read error=" << sslError;
          break;
        }
      } else {
        serverReadBytes += rc;
        XLOG(DBG6) << "Server read returned rc=" << rc
                   << " serverReadBytes=" << serverReadBytes;
      }
    }
    EXPECT_EQ(serverReadBytes, kBufSize + 1);
    // The 1 byte write completed, shutdown.
    co_await std::move(writeFut);
    cs->shutdownWrite();
    SSL_shutdown(ssl_.get());
    EXPECT_EQ(co_await std::move(readFut), 0);
  });
}

} // namespace proxygen::coro

// timeout when computing timeout
// applyVerificationOptions fails
// callback ctrl
// read EOF from SSL_write
