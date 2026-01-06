/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>

#include <fizz/protocol/AsyncFizzBase.h>

#include <folly/io/async/test/MockAsyncTransport.h>
#include <folly/io/async/test/MockTimeoutManager.h>

namespace fizz {
namespace test {

using namespace folly;
using namespace folly::test;
using namespace testing;

static const uint32_t kPartialWriteThreshold = 128 * 1024;

/**
 * The test class itself implements AsyncFizzBase so that it has access to the
 * app data interfaces.
 */
class AsyncFizzBaseTest : public testing::Test, public AsyncFizzBase {
 public:
  AsyncFizzBaseTest()
      : testing::Test(),
        AsyncFizzBase(
            AsyncTransportWrapper::UniquePtr(new MockAsyncTransport()),
            TransportOptions{}) {
    socket_ = getUnderlyingTransport<MockAsyncTransport>();
    ON_CALL(*this, good()).WillByDefault(Return(true));
    ON_CALL(*this, isReplaySafe()).WillByDefault(Return(true));
    ON_CALL(*this, connecting()).WillByDefault(Return(false));
    ON_CALL(*this, transportError(_))
        .WillByDefault(Invoke([this](const AsyncSocketException& ex) {
          this->deliverError(ex);
        }));
  }

  void TearDown() override {
    EXPECT_CALL(*socket_, setReadCB(nullptr));
  }

  MOCK_METHOD(bool, good, (), (const));
  MOCK_METHOD(bool, readable, (), (const));
  MOCK_METHOD(bool, connecting, (), (const));
  MOCK_METHOD(bool, error, (), (const));
  MOCK_METHOD(folly::ssl::X509UniquePtr, getPeerCert, (), (const));
  MOCK_METHOD(const X509*, getSelfCert, (), (const));
  MOCK_METHOD(bool, isReplaySafe, (), (const));
  MOCK_METHOD(
      void,
      setReplaySafetyCallback,
      (folly::AsyncTransport::ReplaySafetyCallback * callback));
  MOCK_METHOD(const Cert*, getSelfCertificate, (), (const));
  MOCK_METHOD(const Cert*, getPeerCertificate, (), (const));
  MOCK_METHOD(std::string, getApplicationProtocol_, (), (const));

  std::string getApplicationProtocol() const noexcept override {
    return getApplicationProtocol_();
  }

  MOCK_METHOD(folly::Optional<CipherSuite>, getCipher, (), (const));
  MOCK_METHOD(folly::Optional<NamedGroup>, getGroup, (), (const));
  MOCK_METHOD(
      std::vector<SignatureScheme>,
      getSupportedSigSchemes,
      (),
      (const));
  MOCK_METHOD(
      Buf,
      getExportedKeyingMaterial,
      (folly::StringPiece, Buf, uint16_t),
      (const));

  MOCK_METHOD(
      void,
      writeAppDataInternal,
      (folly::AsyncTransportWrapper::WriteCallback*,
       std::shared_ptr<folly::IOBuf>,
       folly::WriteFlags));

  void writeAppData(
      folly::AsyncTransportWrapper::WriteCallback* callback,
      std::unique_ptr<folly::IOBuf>&& buf,
      folly::WriteFlags flags = folly::WriteFlags::NONE) override {
    writeAppDataInternal(
        callback, std::shared_ptr<folly::IOBuf>(buf.release()), flags);
  }

  MOCK_METHOD(void, transportError, (const folly::AsyncSocketException&));
  MOCK_METHOD(void, transportDataAvailable, ());
  MOCK_METHOD(void, pauseEvents, ());
  MOCK_METHOD(void, resumeEvents, ());

  MOCK_METHOD(folly::Optional<Random>, getClientRandom, (), (const));
  MOCK_METHOD(void, tlsShutdown, ());
  MOCK_METHOD(void, shutdownWrite, ());
  MOCK_METHOD(void, shutdownWriteNow, ());
  MOCK_METHOD(void, initiateKeyUpdate, (KeyUpdateRequest), (override));

 protected:
  void expectReadBufRequest(size_t sizeToGive) {
    readBuf_.resize(sizeToGive);
    EXPECT_CALL(readCallback_, getReadBuffer(_, _))
        .InSequence(readBufSeq_)
        .WillOnce(DoAll(
            SetArgPointee<0>(readBuf_.data()),
            SetArgPointee<1>(readBuf_.size())));
  }

  void expectReadData(const std::string& data) {
    EXPECT_CALL(readCallback_, readDataAvailable_(data.size()))
        .InSequence(readBufSeq_)
        .WillOnce(Invoke([this, data](size_t len) {
          EXPECT_TRUE(std::memcmp(readBuf_.data(), data.data(), len) == 0);
        }));
  }

  void expectTransportReadCallback() {
    EXPECT_CALL(*socket_, setReadCB(_))
        .WillOnce(SaveArg<0>(&transportReadCallback_));
  }

  void expectWrite(
      char repeatedData,
      size_t len,
      folly::AsyncTransportWrapper::WriteCallback** callbackToSave = nullptr) {
    EXPECT_CALL(*this, writeAppDataInternal(_, _, _))
        .InSequence(this->writeSeq_)
        .WillOnce(
            Invoke([repeatedData, len, callbackToSave](
                       folly::AsyncTransportWrapper::WriteCallback* callback,
                       std::shared_ptr<folly::IOBuf> buf,
                       folly::WriteFlags) {
              EXPECT_EQ(buf->computeChainDataLength(), len);
              folly::io::Cursor c(buf.get());
              while (!c.isAtEnd()) {
                EXPECT_EQ(c.read<char>(), repeatedData);
              }

              if (callbackToSave) {
                *callbackToSave = callback;
              } else if (callback) {
                callback->writeSuccess();
              }
            }));
  }

  MockAsyncTransport* socket_;
  StrictMock<folly::test::MockReadCallback> readCallback_;
  ReadCallback* transportReadCallback_{nullptr};
  EventRecvmsgCallback* transportRecvCallback_{nullptr};
  AsyncSocketException ase_{AsyncSocketException::UNKNOWN, "unit test"};
  AsyncSocketException eof_{AsyncSocketException::END_OF_FILE, "unit test eof"};
  Sequence readBufSeq_;
  Sequence writeSeq_;
  std::vector<uint8_t> readBuf_;
};

namespace {

std::unique_ptr<folly::IOBuf> getBuf(char repeatedData, size_t len) {
  auto buf = folly::IOBuf::create(len);
  memset(buf->writableData(), repeatedData, len);
  buf->append(len);
  return buf;
}

class MockSecretCallback : public AsyncFizzBase::SecretCallback {
 public:
  MOCK_METHOD(void, externalPskBinderAvailable_, (const std::vector<uint8_t>&));
  void externalPskBinderAvailable(
      const std::vector<uint8_t>& secret) noexcept override {
    externalPskBinderAvailable_(secret);
  }
  MOCK_METHOD(
      void,
      resumptionPskBinderAvailable_,
      (const std::vector<uint8_t>&));
  void resumptionPskBinderAvailable(
      const std::vector<uint8_t>& secret) noexcept override {
    resumptionPskBinderAvailable_(secret);
  }
  MOCK_METHOD(
      void,
      earlyExporterSecretAvailable_,
      (const std::vector<uint8_t>&));
  void earlyExporterSecretAvailable(
      const std::vector<uint8_t>& secret) noexcept override {
    earlyExporterSecretAvailable_(secret);
  }
  MOCK_METHOD(
      void,
      clientEarlyTrafficSecretAvailable_,
      (const std::vector<uint8_t>&));
  void clientEarlyTrafficSecretAvailable(
      const std::vector<uint8_t>& secret) noexcept override {
    clientEarlyTrafficSecretAvailable_(secret);
  }
  MOCK_METHOD(
      void,
      clientHandshakeTrafficSecretAvailable_,
      (const std::vector<uint8_t>&));
  void clientHandshakeTrafficSecretAvailable(
      const std::vector<uint8_t>& secret) noexcept override {
    clientHandshakeTrafficSecretAvailable_(secret);
  }
  MOCK_METHOD(
      void,
      serverHandshakeTrafficSecretAvailable_,
      (const std::vector<uint8_t>&));
  void serverHandshakeTrafficSecretAvailable(
      const std::vector<uint8_t>& secret) noexcept override {
    serverHandshakeTrafficSecretAvailable_(secret);
  }
  MOCK_METHOD(
      void,
      exporterMasterSecretAvailable_,
      (const std::vector<uint8_t>&));
  void exporterMasterSecretAvailable(
      const std::vector<uint8_t>& secret) noexcept override {
    exporterMasterSecretAvailable_(secret);
  }
  MOCK_METHOD(
      void,
      resumptionMasterSecretAvailable_,
      (const std::vector<uint8_t>&));
  void resumptionMasterSecretAvailable(
      const std::vector<uint8_t>& secret) noexcept override {
    resumptionMasterSecretAvailable_(secret);
  }
  MOCK_METHOD(
      void,
      clientAppTrafficSecretAvailable_,
      (const std::vector<uint8_t>&));
  void clientAppTrafficSecretAvailable(
      const std::vector<uint8_t>& secret) noexcept override {
    clientAppTrafficSecretAvailable_(secret);
  }
  MOCK_METHOD(
      void,
      serverAppTrafficSecretAvailable_,
      (const std::vector<uint8_t>&));
  void serverAppTrafficSecretAvailable(
      const std::vector<uint8_t>& secret) noexcept override {
    serverAppTrafficSecretAvailable_(secret);
  }
};
} // namespace

MATCHER_P(BufMatches, expected, "") {
  folly::IOBufEqualTo eq;
  return eq(*arg, *expected);
}

TEST_F(AsyncFizzBaseTest, TestIsFizz) {
  EXPECT_EQ(this->getSecurityProtocol(), "Fizz");
}

TEST_F(AsyncFizzBaseTest, TestAppBytesWritten) {
  EXPECT_EQ(this->getAppBytesWritten(), 0);

  auto four = IOBuf::copyBuffer("4444");
  this->writeChain(nullptr, std::move(four));
  EXPECT_EQ(this->getAppBytesWritten(), 4);

  auto eight = IOBuf::copyBuffer("88888888");
  auto two = IOBuf::copyBuffer("22");
  eight->prependChain(std::move(two));
  this->writeChain(nullptr, std::move(eight));
  EXPECT_EQ(this->getAppBytesWritten(), 14);
}

TEST_F(AsyncFizzBaseTest, TestAppBytesReceived) {
  EXPECT_EQ(this->getAppBytesReceived(), 0);

  auto four = IOBuf::copyBuffer("4444");
  this->deliverAppData(std::move(four));
  EXPECT_EQ(this->getAppBytesReceived(), 4);

  auto eight = IOBuf::copyBuffer("88888888");
  auto two = IOBuf::copyBuffer("22");
  eight->prependChain(std::move(two));
  this->deliverAppData(std::move(eight));
  EXPECT_EQ(this->getAppBytesReceived(), 14);
}

TEST_F(AsyncFizzBaseTest, TestAppBytesBuffered) {
  AsyncTransportWrapper::WriteCallback* wcb;
  this->expectWrite('a', kPartialWriteThreshold, &wcb);
  this->expectWrite('a', 25);
  this->expectWrite('a', kPartialWriteThreshold, &wcb);
  this->expectWrite('a', 25);

  EXPECT_EQ(this->getAppBytesBuffered(), 0);

  auto buf = getBuf('a', kPartialWriteThreshold + 25);

  // Send kPartialWriteThreshold bytes and cache the rest 25 bytes
  this->writeChain(nullptr, buf->clone());

  EXPECT_EQ(this->getAppBytesBuffered(), 25);

  // Cache all kPartialWriteThreshold + 25 bytes
  this->writeChain(nullptr, buf->clone());

  EXPECT_EQ(this->getAppBytesBuffered(), kPartialWriteThreshold + 50);

  // Send the oldest cached 25 bytes and the first kPartialWriteThreshold
  // bytes of the next node in the chain
  wcb->writeSuccess();

  EXPECT_EQ(this->getAppBytesBuffered(), 25);

  // Send the rest 25 bytes
  wcb->writeSuccess();

  EXPECT_EQ(this->getAppBytesBuffered(), 0);

  EXPECT_EQ(this->getAppBytesWritten(), 2 * kPartialWriteThreshold + 50);
}

TEST_F(AsyncFizzBaseTest, TestWrite) {
  auto buf = IOBuf::copyBuffer("buf");

  EXPECT_CALL(*this, writeAppDataInternal(_, _, _));
  this->writeChain(nullptr, std::move(buf));
}

TEST_F(AsyncFizzBaseTest, TestReadErr) {
  this->setReadCB(&this->readCallback_);

  EXPECT_CALL(this->readCallback_, readErr_(_));
  EXPECT_CALL(*this->socket_, close());
  this->deliverError(this->ase_);
  EXPECT_EQ(this->getReadCallback(), nullptr);
}

TEST_F(AsyncFizzBaseTest, TestReadErrNoCallback) {
  EXPECT_CALL(*this->socket_, close());
  this->deliverError(this->ase_);
}

TEST_F(AsyncFizzBaseTest, TestReadErrAsync) {
  ON_CALL(*this, good()).WillByDefault(Return(false));
  this->deliverError(this->ase_);

  EXPECT_CALL(this->readCallback_, readErr_(_));
  this->setReadCB(&this->readCallback_);
  EXPECT_EQ(this->getReadCallback(), nullptr);
}

TEST_F(AsyncFizzBaseTest, TestReadEOF) {
  this->setReadCB(&this->readCallback_);

  EXPECT_CALL(this->readCallback_, readEOF_());
  this->deliverError(this->eof_);
  EXPECT_EQ(this->getReadCallback(), nullptr);
}

TEST_F(AsyncFizzBaseTest, TestReadEOFNoCallback) {
  this->deliverError(this->eof_);
}

TEST_F(AsyncFizzBaseTest, TestReadEOFDelayedCallback) {
  this->deliverError(this->eof_);

  EXPECT_CALL(this->readCallback_, readEOF_());
  this->setReadCB(&this->readCallback_);
  EXPECT_EQ(this->getReadCallback(), nullptr);
}

TEST_F(AsyncFizzBaseTest, TestReadEOFSetCallbackAgain) {
  this->expectTransportReadCallback();
  this->setReadCB(&this->readCallback_);

  EXPECT_CALL(this->readCallback_, readEOF_());
  EXPECT_CALL(*this->socket_, close())
      .WillOnce(Invoke([this]() {
        ON_CALL(*this, good()).WillByDefault(Return(false));
        this->transportReadCallback_->readEOF();
      }))
      .WillRepeatedly([] {});
  this->endOfTLS(IOBuf::create(0));

  EXPECT_CALL(this->readCallback_, readErr_(_));
  this->setReadCB(&this->readCallback_);
}

TEST_F(AsyncFizzBaseTest, TestMovableBuffer) {
  EXPECT_CALL(this->readCallback_, isBufferMovable_())
      .WillRepeatedly(Return(true));

  this->setReadCB(&this->readCallback_);

  auto buf = IOBuf::copyBuffer("buf");
  EXPECT_CALL(this->readCallback_, readBufferAvailable_(BufMatches(buf.get())));
  this->deliverAppData(buf->clone());

  auto buf2 = IOBuf::copyBuffer("buf2");
  EXPECT_CALL(
      this->readCallback_, readBufferAvailable_(BufMatches(buf2.get())));
  this->deliverAppData(buf2->clone());
}

TEST_F(AsyncFizzBaseTest, TestMovableBufferAsyncCallback) {
  EXPECT_CALL(this->readCallback_, isBufferMovable_())
      .WillRepeatedly(Return(true));

  auto buf = IOBuf::copyBuffer("buf");
  this->deliverAppData(std::move(buf));

  auto buf2 = IOBuf::copyBuffer("buf2");
  this->deliverAppData(std::move(buf2));

  auto expected = IOBuf::copyBuffer("bufbuf2");
  EXPECT_CALL(
      this->readCallback_, readBufferAvailable_(BufMatches(expected.get())));
  this->setReadCB(&this->readCallback_);

  auto buf3 = IOBuf::copyBuffer("buf3");
  EXPECT_CALL(
      this->readCallback_, readBufferAvailable_(BufMatches(buf3.get())));
  this->deliverAppData(buf3->clone());
}

TEST_F(AsyncFizzBaseTest, TestReadBufferLarger) {
  EXPECT_CALL(this->readCallback_, isBufferMovable_())
      .WillRepeatedly(Return(false));

  this->setReadCB(&this->readCallback_);

  auto buf = IOBuf::copyBuffer("sup");
  this->expectReadBufRequest(20);
  this->expectReadData("sup");
  this->deliverAppData(std::move(buf));
}

TEST_F(AsyncFizzBaseTest, TestReadBufferExact) {
  EXPECT_CALL(this->readCallback_, isBufferMovable_())
      .WillRepeatedly(Return(false));

  this->setReadCB(&this->readCallback_);

  auto buf = IOBuf::copyBuffer("sup");
  this->expectReadBufRequest(3);
  this->expectReadData("sup");
  this->deliverAppData(std::move(buf));
}

TEST_F(AsyncFizzBaseTest, TestReadBufferSmaller) {
  EXPECT_CALL(this->readCallback_, isBufferMovable_())
      .WillRepeatedly(Return(false));

  this->setReadCB(&this->readCallback_);

  auto buf = IOBuf::copyBuffer("hello");
  this->expectReadBufRequest(3);
  this->expectReadData("hel");
  this->expectReadBufRequest(3);
  this->expectReadData("lo");
  this->deliverAppData(std::move(buf));
}

TEST_F(AsyncFizzBaseTest, TestReadBufferAsync) {
  EXPECT_CALL(this->readCallback_, isBufferMovable_())
      .WillRepeatedly(Return(false));

  auto buf1 = IOBuf::copyBuffer("buf1");
  this->deliverAppData(std::move(buf1));
  auto buf2 = IOBuf::copyBuffer("buf2");
  this->deliverAppData(std::move(buf2));

  this->expectReadBufRequest(20);
  this->expectReadData("buf1buf2");
  this->setReadCB(&this->readCallback_);

  auto buf3 = IOBuf::copyBuffer("buf3");
  this->expectReadBufRequest(20);
  this->expectReadData("buf3");
  this->deliverAppData(std::move(buf3));
}

TEST_F(AsyncFizzBaseTest, TestReadBufferZero) {
  EXPECT_CALL(this->readCallback_, isBufferMovable_())
      .WillRepeatedly(Return(false));

  this->setReadCB(&this->readCallback_);

  auto buf = IOBuf::copyBuffer("hello");
  this->expectReadBufRequest(0);
  EXPECT_CALL(this->readCallback_, readErr_(_));
  EXPECT_CALL(*this->socket_, close());
  this->deliverAppData(std::move(buf));
}

TEST_F(AsyncFizzBaseTest, TestReadBufferPause) {
  EXPECT_CALL(this->readCallback_, isBufferMovable_())
      .WillRepeatedly(Return(false));

  this->setReadCB(&this->readCallback_);

  auto buf = IOBuf::copyBuffer("hello");
  this->expectReadBufRequest(3);
  EXPECT_CALL(this->readCallback_, readDataAvailable_(3))
      .InSequence(this->readBufSeq_)
      .WillOnce(Invoke([this](size_t len) {
        EXPECT_TRUE(std::memcmp(this->readBuf_.data(), "hel", len) == 0);
        this->setReadCB(nullptr);
      }));
  this->deliverAppData(std::move(buf));

  this->expectReadBufRequest(20);
  this->expectReadData("lo");
  this->setReadCB(&this->readCallback_);
}

TEST_F(AsyncFizzBaseTest, TestReadBufferSwitchToNewMovable) {
  StrictMock<folly::test::MockReadCallback> movableCallback;
  EXPECT_CALL(this->readCallback_, isBufferMovable_())
      .WillRepeatedly(Return(false));
  EXPECT_CALL(movableCallback, isBufferMovable_()).WillRepeatedly(Return(true));

  this->setReadCB(&this->readCallback_);

  auto buf = IOBuf::copyBuffer("hello, world!");
  auto secondBuf = IOBuf::copyBuffer("lo, world!");
  this->expectReadBufRequest(3);
  EXPECT_CALL(this->readCallback_, readDataAvailable_(3))
      .InSequence(this->readBufSeq_)
      .WillOnce(Invoke([this, &movableCallback](size_t len) {
        EXPECT_TRUE(std::memcmp(this->readBuf_.data(), "hel", len) == 0);
        this->setReadCB(&movableCallback);
      }));
  EXPECT_CALL(
      movableCallback, readBufferAvailable_(BufMatches(secondBuf.get())))
      .InSequence(this->readBufSeq_);
  this->deliverAppData(std::move(buf));
}

TEST_F(AsyncFizzBaseTest, TestReadBufferSwitchToMovableBehavior) {
  this->setReadCB(&this->readCallback_);

  auto buf = IOBuf::copyBuffer("hello, world!");
  auto secondBuf = IOBuf::copyBuffer("lo, world!");
  bool movable = false;

  EXPECT_CALL(this->readCallback_, isBufferMovable_())
      .WillRepeatedly(Invoke([&movable]() { return movable; }));
  this->expectReadBufRequest(3);
  EXPECT_CALL(this->readCallback_, readDataAvailable_(3))
      .InSequence(this->readBufSeq_)
      .WillOnce(Invoke([this, &movable](size_t len) {
        EXPECT_TRUE(std::memcmp(this->readBuf_.data(), "hel", len) == 0);
        movable = true;
      }));
  EXPECT_CALL(
      this->readCallback_, readBufferAvailable_(BufMatches(secondBuf.get())))
      .InSequence(this->readBufSeq_);
  this->deliverAppData(std::move(buf));
}

TEST_F(AsyncFizzBaseTest, TestTransportReadBufMovable) {
  this->expectTransportReadCallback();
  this->startTransportReads();
  EXPECT_TRUE(this->transportReadCallback_->isBufferMovable());
}

TEST_F(AsyncFizzBaseTest, TestTransportReadBufMove) {
  IOBufEqualTo eq;
  this->expectTransportReadCallback();
  this->startTransportReads();

  auto buf = IOBuf::copyBuffer("hello");
  EXPECT_CALL(*this, transportDataAvailable());
  this->transportReadCallback_->readBufferAvailable(buf->clone());
  EXPECT_TRUE(eq(*buf, *this->transportReadBuf_.front()));

  EXPECT_CALL(*this, transportDataAvailable());
  this->transportReadCallback_->readBufferAvailable(IOBuf::copyBuffer("world"));
  EXPECT_TRUE(
      eq(*IOBuf::copyBuffer("helloworld"), *this->transportReadBuf_.front()));
}

TEST_F(AsyncFizzBaseTest, TestTransportReadBufAvail) {
  void* buf;
  size_t len;
  IOBufEqualTo eq;
  this->expectTransportReadCallback();
  this->startTransportReads();

  EXPECT_CALL(*this, transportDataAvailable());
  this->transportReadCallback_->getReadBuffer(&buf, &len);
  // Make sure the buffer is a reasonable size.
  EXPECT_GE(len, 128);
  EXPECT_LE(len, 1024 * 64);
  std::memcpy(buf, "hello", 5);
  this->transportReadCallback_->readDataAvailable(5);
  EXPECT_TRUE(
      eq(*IOBuf::copyBuffer("hello"), *this->transportReadBuf_.front()));

  EXPECT_CALL(*this, transportDataAvailable());
  this->transportReadCallback_->getReadBuffer(&buf, &len);
  std::memcpy(buf, "goodbye", 7);
  this->transportReadCallback_->readDataAvailable(7);
  EXPECT_TRUE(
      eq(*IOBuf::copyBuffer("hellogoodbye"), *this->transportReadBuf_.front()));
}

TEST_F(AsyncFizzBaseTest, TestTransportReadError) {
  this->expectTransportReadCallback();
  this->startTransportReads();

  EXPECT_CALL(*this, transportError(_));
  this->transportReadCallback_->readErr(this->ase_);
}

TEST_F(AsyncFizzBaseTest, TestTransportReadEOF) {
  this->expectTransportReadCallback();
  this->startTransportReads();

  EXPECT_CALL(*this, transportError(_))
      .WillOnce(Invoke([](const AsyncSocketException& ex) {
        EXPECT_EQ(ex.getType(), AsyncSocketException::END_OF_FILE);
      }));
  this->transportReadCallback_->readEOF();
}

TEST_F(AsyncFizzBaseTest, TestTransportReadBufPause) {
  this->expectTransportReadCallback();
  this->startTransportReads();

  auto bigBuf = IOBuf::create(1024 * 1024);
  bigBuf->append(1024 * 1024);
  this->expectTransportReadCallback();
  EXPECT_CALL(*this, transportDataAvailable());
  this->transportReadCallback_->readBufferAvailable(std::move(bigBuf));
  EXPECT_EQ(this->transportReadCallback_, nullptr);

  this->expectTransportReadCallback();
  this->setReadCB(&this->readCallback_);
  EXPECT_NE(this->transportReadCallback_, nullptr);
}

TEST_F(AsyncFizzBaseTest, TestAppReadBufPause) {
  EXPECT_CALL(this->readCallback_, isBufferMovable_())
      .WillRepeatedly(Return(true));
  this->expectTransportReadCallback();
  this->startTransportReads();

  auto bigBuf = IOBuf::create(1024 * 1024);
  bigBuf->append(1024 * 1024);
  this->expectTransportReadCallback();
  this->deliverAppData(std::move(bigBuf));
  EXPECT_EQ(this->transportReadCallback_, nullptr);

  this->expectTransportReadCallback();
  EXPECT_CALL(this->readCallback_, readBufferAvailable_(_));
  this->setReadCB(&this->readCallback_);
  EXPECT_NE(this->transportReadCallback_, nullptr);
}

TEST_F(AsyncFizzBaseTest, TestWriteSuccess) {
  AsyncTransportWrapper::WriteCallback* writeCallback = this;
  writeCallback->writeSuccess();
}

TEST_F(AsyncFizzBaseTest, TestWriteError) {
  AsyncTransportWrapper::WriteCallback* writeCallback = this;
  EXPECT_CALL(*this, transportError(_));
  writeCallback->writeErr(0, this->ase_);
}

TEST_F(AsyncFizzBaseTest, TestHandshakeTimeout) {
  MockTimeoutManager manager;
  ON_CALL(manager, isInTimeoutManagerThread()).WillByDefault(Return(true));
  this->attachTimeoutManager(&manager);
  AsyncTimeout* timeout;

  EXPECT_CALL(manager, scheduleTimeout(_, std::chrono::milliseconds(2)))
      .WillOnce(DoAll(SaveArg<0>(&timeout), Return(true)));
  this->startHandshakeTimeout(std::chrono::milliseconds(2));

  EXPECT_CALL(*this, transportError(_))
      .WillOnce(Invoke([](const AsyncSocketException& ex) {
        EXPECT_EQ(ex.getType(), AsyncSocketException::TIMED_OUT);
      }));
  timeout->timeoutExpired();
}

TEST_F(AsyncFizzBaseTest, TestAttachEventBase) {
  EventBase evb;
  this->expectTransportReadCallback();
  this->startTransportReads();
  ON_CALL(*this->socket_, good()).WillByDefault(Return(true));
  Sequence s;

  EXPECT_CALL(*this->socket_, setReadCB(nullptr)).InSequence(s);
  EXPECT_CALL(*this->socket_, detachEventBase()).InSequence(s);
  EXPECT_CALL(*this, pauseEvents()).InSequence(s);
  this->detachEventBase();

  EXPECT_CALL(*this->socket_, attachEventBase(&evb)).InSequence(s);
  EXPECT_CALL(*this, resumeEvents()).InSequence(s);
  EXPECT_CALL(*this->socket_, setReadCB(this->transportReadCallback_))
      .InSequence(s);
  this->attachEventBase(&evb);
}

TEST_F(AsyncFizzBaseTest, TestAttachEventBaseWithReadCb) {
  EventBase evb;
  this->expectTransportReadCallback();
  this->startTransportReads();
  ON_CALL(*this->socket_, good()).WillByDefault(Return(false));
  Sequence s;

  EXPECT_CALL(*this->socket_, setReadCB(nullptr)).InSequence(s);
  EXPECT_CALL(*this->socket_, detachEventBase()).InSequence(s);
  EXPECT_CALL(*this, pauseEvents()).InSequence(s);
  this->detachEventBase();

  this->expectTransportReadCallback();
  this->setReadCB(&this->readCallback_);
  EXPECT_CALL(*this->socket_, attachEventBase(&evb)).InSequence(s);
  EXPECT_CALL(*this, resumeEvents()).InSequence(s);
  EXPECT_CALL(*this->socket_, setReadCB(this->transportReadCallback_))
      .InSequence(s);
  this->attachEventBase(&evb);
}

TEST_F(AsyncFizzBaseTest, TestSecretAvailable) {
  MockSecretCallback cb;
  this->setSecretCallback(&cb);
  auto makeSecret = [](std::string secret, SecretType type) {
    std::vector<uint8_t> secretBuf(secret.begin(), secret.end());
    return DerivedSecret(std::move(secretBuf), type);
  };

  auto checkSecret = [](const DerivedSecret& expected) {
    return [&expected](const std::vector<uint8_t>& secret) {
      EXPECT_EQ(secret, expected.secret);
    };
  };

  auto exPskBinder =
      makeSecret("exPskBindSecret", EarlySecrets::ExternalPskBinder);
  EXPECT_CALL(cb, externalPskBinderAvailable_(_))
      .WillOnce(Invoke(checkSecret(exPskBinder)));
  this->secretAvailable(exPskBinder);

  auto resPskBinder =
      makeSecret("resPskBindSecret", EarlySecrets::ResumptionPskBinder);
  EXPECT_CALL(cb, resumptionPskBinderAvailable_(_))
      .WillOnce(Invoke(checkSecret(resPskBinder)));
  this->secretAvailable(resPskBinder);

  auto earlyExpSecret =
      makeSecret("earlyExpSecret", EarlySecrets::EarlyExporter);
  EXPECT_CALL(cb, earlyExporterSecretAvailable_(_))
      .WillOnce(Invoke(checkSecret(earlyExpSecret)));
  this->secretAvailable(earlyExpSecret);

  auto clientEarlySecret =
      makeSecret("clientEarlySecret", EarlySecrets::ClientEarlyTraffic);
  EXPECT_CALL(cb, clientEarlyTrafficSecretAvailable_(_))
      .WillOnce(Invoke(checkSecret(clientEarlySecret)));
  this->secretAvailable(clientEarlySecret);

  auto clientHandSecret =
      makeSecret("clientHandSecret", HandshakeSecrets::ClientHandshakeTraffic);
  EXPECT_CALL(cb, clientHandshakeTrafficSecretAvailable_(_))
      .WillOnce(Invoke(checkSecret(clientHandSecret)));
  this->secretAvailable(clientHandSecret);

  auto serverHandSecret =
      makeSecret("serverHandSecret", HandshakeSecrets::ServerHandshakeTraffic);
  EXPECT_CALL(cb, serverHandshakeTrafficSecretAvailable_(_))
      .WillOnce(Invoke(checkSecret(serverHandSecret)));
  this->secretAvailable(serverHandSecret);

  auto exporterMaster =
      makeSecret("exporterMaster", MasterSecrets::ExporterMaster);
  EXPECT_CALL(cb, exporterMasterSecretAvailable_(_))
      .WillOnce(Invoke(checkSecret(exporterMaster)));
  this->secretAvailable(exporterMaster);

  auto resumptionMaster =
      makeSecret("resumptionMaster", MasterSecrets::ResumptionMaster);
  EXPECT_CALL(cb, resumptionMasterSecretAvailable_(_))
      .WillOnce(Invoke(checkSecret(resumptionMaster)));
  this->secretAvailable(resumptionMaster);

  auto clientAppSecret =
      makeSecret("clientAppSecret", AppTrafficSecrets::ClientAppTraffic);
  EXPECT_CALL(cb, clientAppTrafficSecretAvailable_(_))
      .WillOnce(Invoke(checkSecret(clientAppSecret)));
  this->secretAvailable(clientAppSecret);

  auto serverAppSecret =
      makeSecret("serverAppSecret", AppTrafficSecrets::ServerAppTraffic);
  EXPECT_CALL(cb, serverAppTrafficSecretAvailable_(_))
      .WillOnce(Invoke(checkSecret(serverAppSecret)));
  this->secretAvailable(serverAppSecret);
}

TEST_F(AsyncFizzBaseTest, TestWriteBuffering) {
  AsyncTransportWrapper::WriteCallback* wcb;

  this->expectWrite('a', kPartialWriteThreshold, &wcb);
  auto buf = getBuf('a', kPartialWriteThreshold + 1);
  this->writeChain(nullptr, buf->clone());

  this->expectWrite('a', 1);
  wcb->writeSuccess();
}

TEST_F(AsyncFizzBaseTest, TestWriteBufferingTransportBuffer) {
  AsyncTransportWrapper::WriteCallback* wcb;

  ON_CALL(*this->socket_, getRawBytesBuffered()).WillByDefault(Return(25));

  this->expectWrite('a', 10, &wcb);
  auto buf = getBuf('a', 10);
  this->writeChain(nullptr, buf->clone());

  this->writeChain(nullptr, getBuf('b', 10));
  this->expectWrite('b', 10);
  wcb->writeSuccess();
}

TEST_F(AsyncFizzBaseTest, TestNoWriteBufferingUnshared) {
  this->expectWrite('a', kPartialWriteThreshold * 10);
  auto buf = getBuf('a', kPartialWriteThreshold * 10);
  this->writeChain(nullptr, std::move(buf));
}

TEST_F(AsyncFizzBaseTest, TestNoWriteBufferingConnecting) {
  EXPECT_CALL(*this, connecting()).WillRepeatedly(Return(true));

  this->expectWrite('a', kPartialWriteThreshold * 10);
  auto buf = getBuf('a', kPartialWriteThreshold * 10);
  this->writeChain(nullptr, buf->clone());
}

TEST_F(AsyncFizzBaseTest, TestNoWriteBufferingReplayUnsafe) {
  EXPECT_CALL(*this, isReplaySafe()).WillRepeatedly(Return(false));

  this->expectWrite('a', kPartialWriteThreshold * 10);
  auto buf = getBuf('a', kPartialWriteThreshold * 10);
  this->writeChain(nullptr, buf->clone());
}

TEST_F(AsyncFizzBaseTest, TestWriteBufferingUnbufferedAfter) {
  AsyncTransportWrapper::WriteCallback* wcb;

  this->expectWrite('a', kPartialWriteThreshold, &wcb);
  auto buf = getBuf('a', kPartialWriteThreshold + 1);
  this->writeChain(nullptr, buf->clone());

  this->expectWrite('a', 1);
  wcb->writeSuccess();

  this->expectWrite('b', 100);
  this->writeChain(nullptr, getBuf('b', 100));
}

TEST_F(AsyncFizzBaseTest, TestWriteBufferingSmallWritesFollowing) {
  AsyncTransportWrapper::WriteCallback* wcb;

  this->expectWrite('a', kPartialWriteThreshold, &wcb);
  auto buf = getBuf('a', kPartialWriteThreshold * 3);
  this->writeChain(nullptr, buf->clone());

  this->expectWrite('a', kPartialWriteThreshold);
  this->expectWrite('a', kPartialWriteThreshold);
  this->expectWrite('b', 200);
  this->expectWrite('c', 75);

  this->writeChain(nullptr, getBuf('b', 200));
  this->writeChain(nullptr, getBuf('c', 75));

  wcb->writeSuccess();
}

TEST_F(AsyncFizzBaseTest, TestWriteBufferingSuccessCallbacks) {
  AsyncTransportWrapper::WriteCallback* wcb;
  StrictMock<folly::test::MockWriteCallback> cb1;
  StrictMock<folly::test::MockWriteCallback> cb2;

  this->expectWrite('a', kPartialWriteThreshold, &wcb);
  auto buf = getBuf('a', kPartialWriteThreshold * 3);
  this->writeChain(&cb1, buf->clone());

  this->expectWrite('a', kPartialWriteThreshold);
  this->expectWrite('a', kPartialWriteThreshold);
  EXPECT_CALL(cb1, writeSuccess_()).InSequence(this->writeSeq_);
  this->expectWrite('b', 200);
  EXPECT_CALL(cb2, writeSuccess_()).InSequence(this->writeSeq_);

  this->writeChain(&cb2, getBuf('b', 200));

  wcb->writeSuccess();
}

TEST_F(AsyncFizzBaseTest, TestWriteBufferingError) {
  AsyncTransportWrapper::WriteCallback* wcb;
  StrictMock<folly::test::MockWriteCallback> cb1;
  StrictMock<folly::test::MockWriteCallback> cb2;

  this->expectWrite('a', kPartialWriteThreshold, &wcb);
  auto buf = getBuf('a', kPartialWriteThreshold * 3);
  this->writeChain(&cb1, buf->clone());

  this->writeChain(&cb2, getBuf('b', 100));

  EXPECT_CALL(cb1, writeErr_(kPartialWriteThreshold, _))
      .InSequence(this->writeSeq_);
  EXPECT_CALL(cb2, writeErr_(0, _)).InSequence(this->writeSeq_);

  wcb->writeErr(kPartialWriteThreshold, this->ase_);
}

TEST_F(AsyncFizzBaseTest, TestWriteBufferingMixedSuccessError) {
  AsyncTransportWrapper::WriteCallback* wcb;
  AsyncTransportWrapper::WriteCallback* wcb2;
  StrictMock<folly::test::MockWriteCallback> cb1;
  StrictMock<folly::test::MockWriteCallback> cb2;

  this->expectWrite('a', kPartialWriteThreshold, &wcb);
  auto buf = getBuf('a', kPartialWriteThreshold * 3);
  this->writeChain(&cb1, buf->clone());

  this->expectWrite('a', kPartialWriteThreshold);
  this->expectWrite('a', kPartialWriteThreshold);
  EXPECT_CALL(cb1, writeSuccess_()).InSequence(this->writeSeq_);

  auto buf2 = getBuf('b', kPartialWriteThreshold * 3);
  this->writeChain(&cb2, buf2->clone());

  this->expectWrite('b', kPartialWriteThreshold);
  this->expectWrite('b', kPartialWriteThreshold, &wcb2);

  wcb->writeSuccess();

  EXPECT_CALL(cb2, writeErr_(kPartialWriteThreshold * 2, _))
      .InSequence(this->writeSeq_);
  wcb2->writeErr(kPartialWriteThreshold, this->ase_);
}

TEST_F(AsyncFizzBaseTest, TestWriteBufferingCork) {
  EXPECT_CALL(*this, writeAppDataInternal(_, _, _))
      .InSequence(this->writeSeq_)
      .WillOnce(Invoke([](folly::AsyncTransportWrapper::WriteCallback* callback,
                          std::shared_ptr<folly::IOBuf> buf,
                          folly::WriteFlags flags) {
        EXPECT_EQ(buf->computeChainDataLength(), kPartialWriteThreshold);
        EXPECT_EQ(flags, folly::WriteFlags::CORK);
        callback->writeSuccess();
      }));

  EXPECT_CALL(*this, writeAppDataInternal(_, _, _))
      .InSequence(this->writeSeq_)
      .WillOnce(Invoke([](folly::AsyncTransportWrapper::WriteCallback* callback,
                          std::shared_ptr<folly::IOBuf> buf,
                          folly::WriteFlags flags) {
        EXPECT_EQ(buf->computeChainDataLength(), 1);
        EXPECT_EQ(flags, folly::WriteFlags::NONE);
        callback->writeSuccess();
      }));

  auto buf = getBuf('a', kPartialWriteThreshold + 1);
  this->writeChain(nullptr, buf->clone());
}

TEST_F(AsyncFizzBaseTest, TestWriteBufferingWriteInCallback) {
  AsyncTransportWrapper::WriteCallback* wcb;
  StrictMock<folly::test::MockWriteCallback> cb;

  this->expectWrite('a', kPartialWriteThreshold, &wcb);
  auto buf = getBuf('a', kPartialWriteThreshold + 1);
  this->writeChain(&cb, buf->clone());

  this->expectWrite('a', 1);
  EXPECT_CALL(cb, writeSuccess_())
      .InSequence(this->writeSeq_)
      .WillOnce(Invoke([this]() {
        this->expectWrite('b', 200);
        this->writeChain(nullptr, getBuf('b', 200));
      }));

  wcb->writeSuccess();
}

TEST_F(AsyncFizzBaseTest, TestWriteBufferingCloseInCallback) {
  AsyncTransportWrapper::WriteCallback* wcb;
  StrictMock<folly::test::MockWriteCallback> cb1, cb2;

  this->expectWrite('a', kPartialWriteThreshold, &wcb);
  auto buf = getBuf('a', kPartialWriteThreshold + 1);
  this->writeChain(&cb1, buf->clone());
  this->writeChain(&cb2, getBuf('b', 200));

  this->expectWrite('a', 1);
  EXPECT_CALL(cb1, writeSuccess_())
      .InSequence(this->writeSeq_)
      .WillOnce(Invoke([this, &cb2]() {
        bool deliverErrorReturned = false;
        EXPECT_CALL(cb2, writeErr_(0, _))
            .InSequence(this->writeSeq_)
            .WillOnce(InvokeWithoutArgs([&deliverErrorReturned]() {
              EXPECT_FALSE(deliverErrorReturned);
            }));
        this->deliverError(this->ase_);
        deliverErrorReturned = true;
      }));

  wcb->writeSuccess();
}

TEST_F(AsyncFizzBaseTest, TestAlignedRecordReads) {
  this->setHandshakeRecordAlignedReads(true);

  this->expectTransportReadCallback();
  this->startTransportReads();
  IOBufEqualTo eq;

  void* buf;
  size_t len;
  this->transportReadCallback_->getReadBuffer(&buf, &len);

  // Under record aligned reads mode, we should always start with a read
  // of the record header (5 bytes)
  EXPECT_EQ(len, 5);
  std::memcpy(buf, "12345", 5);

  EXPECT_CALL(*this, transportDataAvailable());
  this->transportReadCallback_->readDataAvailable(5);
  EXPECT_TRUE(
      eq(*IOBuf::copyBuffer("12345"), *(this->transportReadBuf_.front())));
  {
    auto _ = this->transportReadBuf_.move();
  }

  // Subclasses would normally make this call whenever it receives a
  // WaitForData action from the state machine
  this->updateReadHint(100);

  this->transportReadCallback_->getReadBuffer(&buf, &len);
  std::memset(buf, 'A', 100);
  EXPECT_EQ(len, 100);

  EXPECT_CALL(*this, transportDataAvailable());
  this->transportReadCallback_->readDataAvailable(100);
  {
    auto _ = this->transportReadBuf_.move();
  }

  // When the handshake completes, read hint will be 0, subsequent allocations
  // should be equal to the default kMaxReadSize allocation.
  this->updateReadHint(0);

  // This should be ignored, since updateReadHint(0) was already called.
  this->updateReadHint(5);

  this->transportReadCallback_->getReadBuffer(&buf, &len);

  // Not caring about record alignment, we should be able to get a buffer of
  // at least AsyncFizzBase::kMinReadSize
  EXPECT_GE(len, 1460);
}

TEST_F(AsyncFizzBaseTest, TestKeyUpdate) {
  size_t threshold = 20;
  size_t small_write = 15;
  size_t big_write = threshold;
  this->setRekeyAfterWriting(threshold);

  // If we perform an appWrite that exceeds or equals to the threshold,
  // we initiate a key update
  EXPECT_CALL(*this, initiateKeyUpdate(KeyUpdateRequest::update_not_requested));
  this->wroteApplicationBytes(big_write);

  // If the write does not exceed the threshold, we don't initiate a key update
  EXPECT_CALL(*this, initiateKeyUpdate(KeyUpdateRequest::update_not_requested))
      .Times(0);
  this->wroteApplicationBytes(small_write);
  EXPECT_EQ(this->appByteProcessedUnderKey_, small_write);

  // If multiple appWrites' total bytes exceed the threshold,
  // we initiate a key update
  EXPECT_CALL(*this, initiateKeyUpdate(KeyUpdateRequest::update_not_requested));
  this->wroteApplicationBytes(small_write);
  EXPECT_EQ(this->appByteProcessedUnderKey_, 0);

  // Make sure the rekey threshold stays the same
  EXPECT_EQ(this->getRekeyAfterWriting(), threshold);
}

} // namespace test
} // namespace fizz
