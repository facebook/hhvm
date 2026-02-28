/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/http/coro/transport/test/HTTPConnectTransportTest.h"
#include <proxygen/lib/http/session/test/MockHTTPSessionStats.h>

#include <folly/Portability.h>
#include <folly/coro/BlockingWait.h>
#include <folly/coro/Collect.h>
#include <folly/logging/xlog.h>

using namespace std::chrono_literals;
using namespace folly;
using namespace folly::coro;

template <size_t SIZE>
folly::coro::Task<Unit> readAll(folly::coro::TransportIf& transport,
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

namespace proxygen::coro {

class ReadCallback : public AsyncTransport::ReadCallback {
 public:
  void getReadBuffer(void** bufReturn, size_t* lenReturn) override {
    auto buf = readBuf_.preallocate(1500, 4000);
    *bufReturn = buf.first;
    *lenReturn = buf.second;
  }

  void readDataAvailable(size_t len) noexcept override {
    XLOG(DBG2) << "readDataAvailable, len " << len;
    readBuf_.postallocate(len);
    if (readBuf_.chainLength() >= expectedLen) {
      promise.setValue(folly::Unit());
    }
  }

  bool isBufferMovable() noexcept override {
    return bufferMovable;
  }

  void readBufferAvailable(
      std::unique_ptr<folly::IOBuf> buf) noexcept override {
    readBuf_.append(std::move(buf));
    if (readBuf_.chainLength() >= expectedLen) {
      promise.setValue(folly::Unit());
    }
  }

  void readErr(const AsyncSocketException& ex) noexcept override {
    XLOG(DBG2) << "readError " << ex.what();
    promise.setException(ex);
    eofPromise.setException(std::runtime_error("EOF waiting for data"));
  }

  void readEOF() noexcept override {
    XLOG(DBG2) << "readEOF";

    if (!promise.isFulfilled()) {
      promise.setException(std::runtime_error("EOF waiting for data"));
    }
    eofPromise.setValue(folly::Unit());
  }

  folly::IOBufQueue readBuf_{folly::IOBufQueue::cacheChainLength()};
  std::shared_ptr<HTTPConnectAsyncTransport> transport;
  bool bufferMovable{false};
  size_t expectedLen{std::numeric_limits<size_t>::max()};
  folly::Promise<folly::Unit> promise;
  folly::Promise<folly::Unit> eofPromise;
};

class WriteCallback : public AsyncTransport::WriteCallback {
 public:
  void writeSuccess() noexcept override {
    XLOG(DBG2) << "writeSuccess";
    promise.setValue(folly::Unit());
  }

  void writeErr(size_t nBytesWritten,
                const AsyncSocketException& ex) noexcept override {
    XLOG(DBG2) << "writeError: bytesWritten " << nBytesWritten << ", exception "
               << ex.what();
    promise.setException(ex);
  }

  std::shared_ptr<HTTPConnectAsyncTransport> transport;
  folly::Promise<folly::Unit> promise;
};

class TransportTest : public testing::Test {
 public:
  template <typename F>
  void run(F f) {
    blockingWait(co_invoke(std::move(f)), &evb);
  }

  void TearDown() override {
    evb.loop();
  }

  folly::coro::Task<> requestCancellation() {
    cancelSource.requestCancellation();
    co_return;
  }

  EventBase evb;
  CancellationSource cancelSource;
};

class HTTPConnectTransportTest : public TransportTest {
 public:
  void SetUp() override {
    HTTPServer::Config config;
    config.socketConfig.bindAddress.setFromIpPort("127.0.0.1", 0);
    // To help the write timeout test, use the default stream flow control
    config.plaintextProtocol = "h2";
    config.sessionConfig.settings = {
        {SettingsId::INITIAL_WINDOW_SIZE, 64 * 1024}};
    config.numIOThreads = 1;
    srv = ScopedHTTPServer::start(std::move(config), connectHandler);
  }

  void TearDown() override {
    TransportTest::TearDown();
    // Any connections that are still open will race with drop
    connectHandler->resetExceptionExpected();
  }

  folly::coro::Task<std::unique_ptr<HTTPConnectTransport>> connect(
      HTTPCoroSession* session = nullptr) {
    std::unique_ptr<HTTPConnectStream> connectStream;
    if (!session) {
      session = co_await HTTPCoroConnector::connect(
          &evb, *srv->address(), 0ms, getConnParams());
      auto reservation = session->reserveRequest();
      connectStream =
          co_await HTTPConnectStream::connectUnique(session,
                                                    std::move(*reservation),
                                                    kAuthority,
                                                    std::chrono::seconds(1),
                                                    {{"Foo", "Bar"}},
                                                    egressBufferSize);
    } else {
      auto reservation = session->reserveRequest();
      connectStream =
          co_await HTTPConnectStream::connect(session,
                                              std::move(*reservation),
                                              kAuthority,
                                              std::chrono::seconds(1),
                                              {{"Foo", "Bar"}},
                                              egressBufferSize);
    }
    co_return std::make_unique<HTTPConnectTransport>(std::move(connectStream));
  }

  folly::coro::Task<std::shared_ptr<HTTPConnectAsyncTransport>> connectAsync() {
    auto session = co_await HTTPCoroConnector::connect(
        &evb, *srv->address(), 0ms, getConnParams());
    auto reservation = session->reserveRequest();
    auto connectStream =
        co_await HTTPConnectStream::connectUnique(session,
                                                  std::move(*reservation),
                                                  kAuthority,
                                                  std::chrono::seconds(1),
                                                  {{"Foo", "Bar"}},
                                                  egressBufferSize);
    auto transport =
        std::make_shared<HTTPConnectAsyncTransport>(std::move(connectStream));
    transport->setReadCB(&rcb);
    co_return transport;
  }

  HTTPCoroConnector::ConnectionParams getConnParams() {
    auto connParams = HTTPCoroConnector::defaultConnectionParams();
    connParams.plaintextProtocol = "h2";
    return connParams;
  }

  size_t egressBufferSize{64 * 1024};
  std::shared_ptr<ConnectHandler> connectHandler{
      std::make_shared<ConnectHandler>()};
  std::unique_ptr<ScopedHTTPServer> srv;
  ReadCallback rcb;
  WriteCallback wcb;
};

TEST_F(HTTPConnectTransportTest, ConnectSuccess) {
  run([&]() -> Task<> {
    auto cs = co_await connect();
    // Peer address is the 'X-Connected-To' address
    EXPECT_EQ("1.2.3.4", cs->getPeerAddress().getAddressStr());
    EXPECT_EQ(connectHandler->peerAddr, cs->getLocalAddress());
    EXPECT_EQ(cs->getEventBase(), &evb);
    // TODO:  :/
    EXPECT_EQ(cs->getTransport(), nullptr);
    EXPECT_EQ(cs->getPeerCertificate(), nullptr);
  });
}

TEST_F(HTTPConnectTransportTest, ConnectNon200) {
  connectHandler->expectExceptions();
  run([&]() -> Task<> {
    auto connParams = HTTPCoroConnector::defaultConnectionParams();
    connParams.plaintextProtocol = "h2";
    auto session = co_await HTTPCoroConnector::connect(
        &evb, *srv->address(), 0ms, connParams);
    auto reservation = session->reserveRequest();
    EXPECT_THROW(
        co_await HTTPConnectStream::connectUnique(session,
                                                  std::move(*reservation),
                                                  kAuthority,
                                                  std::chrono::seconds(1),
                                                  {{"Fail", "True"}}),
        std::runtime_error);
  });
}

TEST_F(HTTPConnectTransportTest, ConnectSlow) {
  run([&]() -> Task<> {
    auto connParams = HTTPCoroConnector::defaultConnectionParams();
    connParams.plaintextProtocol = "h2";
    HTTPCoroConnector::SessionParams sessParams;
    sessParams.streamReadTimeout = std::chrono::milliseconds(100);
    // 100ms stream read timeout
    auto session = co_await HTTPCoroConnector::connect(
        &evb, *srv->address(), 0ms, connParams, sessParams);
    auto reservation = session->reserveRequest();
    // 1s connect timeout, doesn't timeout
    auto connectStream = co_await HTTPConnectStream::connectUnique(
        session,
        std::move(*reservation),
        kAuthority,
        std::chrono::seconds(1),
        {{"Slow", "True"}, {"Foo", "Bar"}});
  });
}

TEST_F(HTTPConnectTransportTest, ConnectShared) {
  run([&]() -> Task<> {
    auto connParams = HTTPCoroConnector::defaultConnectionParams();
    connParams.plaintextProtocol = "h2";
    auto session = co_await HTTPCoroConnector::connect(
        &evb, *srv->address(), 0ms, connParams);
    auto sessPtr = session->acquireKeepAlive();
    auto cs1 = co_await connect(session);
    auto cs2 = co_await connect(session);
    auto port = cs1->getLocalAddress().getPort();
    EXPECT_EQ(port, cs2->getLocalAddress().getPort());
    cs1.reset();
    cs2.reset();
    cs1 = co_await connect(session);
    EXPECT_EQ(port, cs1->getLocalAddress().getPort());
    cs1.reset();
    co_await folly::coro::co_reschedule_on_current_executor;
    session->initiateDrain();
  });
}

TEST_F(HTTPConnectTransportTest, ConnectCancelled) {
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
    EXPECT_THROW(co_await co_withCancellation(
                     cancelSource.getToken(),
                     Transport::newConnectedSocket(&evb, *srv->address(), 0ms)),
                 OperationCancelled);
  });
}

TEST_F(HTTPConnectTransportTest, SimpleReadWrite) {
  run([&]() -> Task<> {
    // Should fill window
    constexpr auto kBufSize = 65 * 1024;
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

TEST_F(HTTPConnectTransportTest, SimpleIOBufReadWrite) {
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
    // Initiate close from the peer
    connectHandler->evb->runInEventBaseThreadAndWait([this]() {
      // must run inside evb
      connectHandler->curRespSource->eom();
    });

    auto bytesRead = co_await cs->read(rcvBuf, 1000, 1000, 50ms);
    EXPECT_EQ(bytesRead, 0); // closed
    bytesRead = co_await cs->read(rcvBuf, 1000, 1000, 50ms);
    EXPECT_EQ(bytesRead, 0); // still 0

    auto data = rcvBuf.move();
    data->coalesce();
    EXPECT_EQ(0, memcmp(sndBuf.data(), data->data(), data->length()));
  });
}

TEST_F(HTTPConnectTransportTest, ReadCancelled) {
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
  });
}

TEST_F(HTTPConnectTransportTest, ReadTimeout) {
  run([&]() -> Task<> {
    auto cs = co_await connect();
    std::array<uint8_t, 1024> rcvBuf;
    EXPECT_THROW(
        co_await cs->read(
            MutableByteRange(rcvBuf.data(), (rcvBuf.data() + rcvBuf.size())),
            50ms),
        AsyncSocketException);
  });
}

TEST_F(HTTPConnectTransportTest, ReadAfterReadTimeout) {
  run([&]() -> Task<> {
    auto* sess = co_await HTTPCoroConnector::connect(
        &evb, *srv->address(), 0ms, getConnParams());
    auto connectStream =
        co_await HTTPConnectStream::connectUnique(sess,
                                                  *sess->reserveRequest(),
                                                  kAuthority,
                                                  std::chrono::seconds(1),
                                                  {{"Foo", "Bar"}},
                                                  egressBufferSize);
    auto* pConnectStream = connectStream.get();
    auto cs = std::make_unique<HTTPConnectTransport>(std::move(connectStream));

    std::array<uint8_t, 1024> buf{};
    /**
     * We only rx data on the connect stream if we tx data (it's an echo
     * handler). Since we're reading without tx'ing any data, a 50ms read should
     * timeout and yield the appropriate exception.
     */
    auto read_res = co_await folly::coro::co_awaitTry(cs->read(
        MutableByteRange(buf.data(), (buf.data() + buf.size())), 50ms));
    auto asyncSocketEx =
        CHECK_NOTNULL(read_res.tryGetExceptionObject<AsyncSocketException>());
    EXPECT_EQ(asyncSocketEx->getType(), AsyncSocketException::TIMED_OUT);

    // instruct server to send response & h2 eom together
    std::array<unsigned char, 1024> sendBuf = {"write_fin"};
    co_await cs->write(
        ByteRange(sendBuf.data(), sendBuf.data() + sendBuf.size()));

    // wait until HTTPConnectTransport consumes the body bytes enqueued
    testing::NiceMock<MockHTTPSessionStats> sessionStats;
    sess->setSessionStats(&sessionStats);
    folly::coro::Baton waitUntilRead;
    ON_CALL(sessionStats, _recordPendingBufferedReadBytes(-1024))
        .WillByDefault([&]() { waitUntilRead.post(); });
    co_await waitUntilRead;

    /**
     * read should return > 0 bytes; the ingress source is no longer readable
     * but there is a queued event
     */
    EXPECT_FALSE(pConnectStream->canRead());
    read_res = co_await folly::coro::co_awaitTry(cs->read(
        MutableByteRange(buf.data(), (buf.data() + buf.size())), 50ms));

    XCHECK(!read_res.hasException());
    EXPECT_GT(read_res.value(), 0);
    sess->setSessionStats(nullptr);

    /**
     * another read should return 0; ingress source isn't readable and no queued
     * events
     */
    read_res = co_await folly::coro::co_awaitTry(cs->read(
        MutableByteRange(buf.data(), (buf.data() + buf.size())), 50ms));
    EXPECT_EQ(read_res.value(), 0);
  });
}

TEST_F(HTTPConnectTransportTest, ReadError) {
  run([&]() -> Task<> {
    auto cs = co_await connect();

    connectHandler->evb->runInEventBaseThreadAndWait([this]() {
      auto respSource = std::exchange(connectHandler->curRespSource, nullptr);
      // must run inside evb
      respSource->abort(HTTPErrorCode::CANCEL);
    });

    std::array<uint8_t, 1024> rcvBuf;
    EXPECT_THROW(
        co_await cs->read(
            MutableByteRange(rcvBuf.data(), (rcvBuf.data() + rcvBuf.size())),
            50ms),
        folly::AsyncSocketException);
  });
}

TEST_F(HTTPConnectTransportTest, SimpleWritev) {
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
    co_await cs->write(sndBuf);

    std::array<uint8_t, kBufSize> rcvBufA;
    co_await readAll(*cs, rcvBufA, 500ms);
    EXPECT_EQ(0, memcmp(bufA.data(), rcvBufA.data(), rcvBufA.size()));
    std::array<uint8_t, kBufSize> rcvBufB;
    co_await readAll(*cs, rcvBufB, 500ms);
    EXPECT_EQ(0, memcmp(bufB.data(), rcvBufB.data(), rcvBufB.size()));
  });
}

TEST_F(HTTPConnectTransportTest, WriteCancelled) {
  run([&]() -> Task<> {
    // reduce the send buffer size so the write wouldn't complete immediately
    egressBufferSize = 4096;
    auto cs = co_await connect();
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
}

TEST_F(HTTPConnectTransportTest, ShutdownWrite) {
  run([&]() -> Task<> {
    auto cs = co_await connect();
    // Sends shutdown
    cs->shutdownWrite();
    IOBufQueue rcvBuf(IOBufQueue::cacheChainLength());
    // Wait for peer to shutdown
    auto bytesRead = co_await cs->read(rcvBuf, 1000, 1000, 500ms);
    EXPECT_EQ(bytesRead, 0); // closed
    bytesRead = co_await cs->read(rcvBuf, 1000, 1000, 500ms);
    EXPECT_EQ(bytesRead, 0); // still closed

    constexpr auto kBufSize = 65536;
    std::array<uint8_t, kBufSize> sndBuf;
    std::memset(sndBuf.data(), 'a', sndBuf.size());
    EXPECT_THROW(co_await cs->write(
                     ByteRange(sndBuf.data(), sndBuf.data() + sndBuf.size())),
                 folly::AsyncSocketException);
  });
}

TEST_F(HTTPConnectTransportTest, CloseWithReadsWrites) {
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
    // The read is cancelled
    // TODO: This exception comes from stopReading, should we normalize
    // cancellation errors from read()?
    EXPECT_THROW(co_await std::move(readFut), folly::AsyncSocketException);
  });
}

TEST_F(HTTPConnectTransportTest, CloseWithReset) {
  connectHandler->expectExceptions();
  run([&]() -> Task<> {
    auto cs = co_await connect();
    cs->closeWithReset();
  });
}

TEST_F(HTTPConnectTransportTest, WritesBlockedPendingEOF) {
  run([&]() -> Task<> {
    auto cs = co_await connect();
    constexpr auto kBufSize = 64 * 3 * 1024 + 1;
    std::array<uint8_t, kBufSize> sndBuf;
    std::memset(sndBuf.data(), 'a', sndBuf.size());
    // Fills the receiver's window (64k) and
    //       the BodyEventQueue buffer (64k) +
    //       the HTTPStreamSource buffer (64k) + 1
    // This is bigger than I thought :(
    auto writeFut =
        co_withExecutor(
            &evb,
            cs->write(ByteRange(sndBuf.data(), sndBuf.data() + sndBuf.size()),
                      100ms))
            .start()
            .via(&evb)
            .then([](auto&& res) { EXPECT_FALSE(res.hasException()); });
    co_await folly::coro::co_reschedule_on_current_executor;
    cs->shutdownWrite();
    co_await std::move(writeFut);

    IOBufQueue rcvBuf(IOBufQueue::cacheChainLength());
    // Wait for peer to shutdown
    std::array<uint8_t, kBufSize> rcvBufB;
    co_await readAll(*cs, rcvBufB, 500ms);
    EXPECT_EQ(co_await cs->read(rcvBuf, 1000, 1000, 500ms), 0); // closed
  });
}

TEST_F(HTTPConnectTransportTest, WritesBlockedTimeout) {
  run([&]() -> Task<> {
    connectHandler->timeout = std::chrono::seconds(1);
    auto cs = co_await connect();
    constexpr auto kBufSize = 64 * 3 * 1024 + 1;
    std::array<uint8_t, kBufSize> sndBuf;
    std::memset(sndBuf.data(), 'a', sndBuf.size());
    // Fills the receiver's window (64k) and
    //       the BodyEventQueue buffer (64k) +
    //       the HTTPStreamSource buffer (64k) + 1
    // This is bigger than I thought :(
    co_await cs->write(ByteRange(sndBuf.data(), sndBuf.data() + sndBuf.size()),
                       100ms);
    // Will timeout waiting to send
    EXPECT_THROW(
        co_await cs->write(
            ByteRange(sndBuf.data(), sndBuf.data() + sndBuf.size()), 100ms),
        AsyncSocketException);
  });
}

TEST_F(HTTPConnectTransportTest, AsyncConnectSuccess) {
  run([&]() -> Task<> {
    auto cs = co_await connectAsync();

    EXPECT_EQ("1.2.3.4", cs->AsyncTransport::getPeerAddress().getAddressStr());
  });
}

TEST_F(HTTPConnectTransportTest, AsyncReadWrite) {
  run([&]() -> Task<> {
    auto cs = co_await connectAsync();

    constexpr auto kBufSize = 65536;
    rcb.expectedLen = 65536;
    std::array<uint8_t, kBufSize> sndBuf;
    std::memset(sndBuf.data(), 'a', sndBuf.size());

    cs->write(&wcb, sndBuf.data(), sndBuf.size());
    co_await wcb.promise.getFuture().via(&evb);
    co_await rcb.promise.getFuture().via(&evb);
    cs->shutdownWrite();
    co_await rcb.eofPromise.getFuture().via(&evb);
  });
}

// similar to the test above, but we immediately invoke shutdownWrite after
// write
TEST_F(HTTPConnectTransportTest, AsyncReadWriteEom) {
  run([&]() -> Task<> {
    auto cs = co_await connectAsync();

    constexpr auto kBufSize = 65536;
    rcb.expectedLen = 65536;
    std::array<uint8_t, kBufSize> sndBuf;
    std::memset(sndBuf.data(), 'a', sndBuf.size());

    cs->write(&wcb, sndBuf.data(), sndBuf.size());
    cs->shutdownWrite();

    co_await wcb.promise.getFuture().via(&evb);
    co_await rcb.promise.getFuture().via(&evb);
    co_await rcb.eofPromise.getFuture().via(&evb);
  });
}

TEST_F(HTTPConnectTransportTest, AsyncReadWriteIOBuf) {
  run([&]() -> Task<> {
    auto cs = co_await connectAsync();

    constexpr auto kBufSize = 65536;
    rcb.bufferMovable = true;
    rcb.expectedLen = 65536;
    std::array<uint8_t, kBufSize> sndBuf;
    std::memset(sndBuf.data(), 'a', sndBuf.size());

    cs->write(&wcb, sndBuf.data(), sndBuf.size());
    co_await wcb.promise.getFuture().via(&evb);
    co_await rcb.promise.getFuture().via(&evb);
    cs->shutdownWrite();
    co_await rcb.eofPromise.getFuture().via(&evb);
  });
}

} // namespace proxygen::coro

// Unit tests to add

// HTTPConnectTransport
// ===
// write after egress error
// fc window cancelled

// HTTPConnectAsyncTransport
// ===
// getReadCallback
// writev
// close, good, readable, writable, connecting
// get/setSendTimeout
// getLocalAddress
// isEORTrackingEnabled
// setReadCallback no-op
// update read callback while waiting (is this a good idea?)
// deleted while read() active
// readBodyEvent exception
// non-body (can't test this since only body is legal, future proofing
// write when not writable
// write with null buffer
// shutdownRead
// shutdownWriteNow
// errorWrites with pending callbacks
// bytesProcessed with pending callbacks
