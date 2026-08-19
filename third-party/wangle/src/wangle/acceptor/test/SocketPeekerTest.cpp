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

#include <wangle/acceptor/SocketPeeker.h>

#include <string>

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>

#include <folly/io/async/test/MockAsyncSocket.h>

using namespace folly;
using namespace folly::test;
using namespace wangle;
using namespace testing;

class MockSocketPeekerCallback : public SocketPeeker::Callback {
 public:
  ~MockSocketPeekerCallback() override = default;

  MOCK_METHOD1(peekSuccess_, void(typename std::vector<uint8_t>));
  void peekSuccess(std::vector<uint8_t> peekBytes) noexcept override {
    peekSuccess_(peekBytes);
  }

  MOCK_METHOD1(peekError_, void(const folly::AsyncSocketException&));
  void peekError(const folly::AsyncSocketException& ex) noexcept override {
    peekError_(ex);
  }
};

class SocketPeekerTest : public Test {
 public:
  void SetUp() override {
    sock = new MockAsyncSocket(&base);
  }

  void TearDown() override {
    sock->destroy();
  }

  MockAsyncSocket* sock;
  MockSocketPeekerCallback callback;
  EventBase base;
};

MATCHER_P2(BufMatches, buf, len, "") {
  if (arg.size() != size_t(len)) {
    return false;
  } else if (len == 0) {
    return true;
  }
  return memcmp(buf, arg.data(), len) == 0;
}

MATCHER_P2(IOBufMatches, buf, len, "") {
  return folly::IOBufEqualTo()(arg, folly::IOBuf::copyBuffer(buf, len));
}

TEST_F(SocketPeekerTest, TestPeekSuccess) {
  EXPECT_CALL(*sock, setReadCB(_));
  SocketPeeker::UniquePtr peeker(new SocketPeeker(*sock, &callback, 2));
  peeker->start();

  uint8_t* buf = nullptr;
  size_t len = 0;
  peeker->getReadBuffer(reinterpret_cast<void**>(&buf), &len);
  EXPECT_EQ(2, len);
  // first 2 bytes of SSL3+.
  buf[0] = 0x16;
  buf[1] = 0x03;
  EXPECT_CALL(*sock, _setPreReceivedData(IOBufMatches(buf, 2)));
  EXPECT_CALL(callback, peekSuccess_(BufMatches(buf, 2)));
  // once after peeking, and once during destruction.
  EXPECT_CALL(*sock, setReadCB(nullptr));
  peeker->readDataAvailable(2);
}

TEST_F(SocketPeekerTest, TestEOFDuringPeek) {
  EXPECT_CALL(*sock, setReadCB(_));
  SocketPeeker::UniquePtr peeker(new SocketPeeker(*sock, &callback, 2));
  peeker->start();

  EXPECT_CALL(callback, peekError_(_));
  EXPECT_CALL(*sock, setReadCB(nullptr));
  peeker->readEOF();
}

TEST_F(SocketPeekerTest, TestNotEnoughDataError) {
  EXPECT_CALL(*sock, setReadCB(_));
  SocketPeeker::UniquePtr peeker(new SocketPeeker(*sock, &callback, 2));
  peeker->start();

  uint8_t* buf = nullptr;
  size_t len = 0;
  peeker->getReadBuffer(reinterpret_cast<void**>(&buf), &len);
  EXPECT_EQ(2, len);
  buf[0] = 0x16;
  peeker->readDataAvailable(1);

  EXPECT_CALL(callback, peekError_(_));
  EXPECT_CALL(*sock, setReadCB(nullptr));
  peeker->readEOF();
}

TEST_F(SocketPeekerTest, TestMultiplePeeks) {
  EXPECT_CALL(*sock, setReadCB(_));
  SocketPeeker::UniquePtr peeker(new SocketPeeker(*sock, &callback, 2));
  peeker->start();

  uint8_t* buf = nullptr;
  size_t len = 0;
  peeker->getReadBuffer(reinterpret_cast<void**>(&buf), &len);
  EXPECT_EQ(2, len);
  buf[0] = 0x16;
  peeker->readDataAvailable(1);

  peeker->getReadBuffer(reinterpret_cast<void**>(&buf), &len);
  EXPECT_EQ(1, len);
  buf[0] = 0x03;

  EXPECT_CALL(*sock, _setPreReceivedData(IOBufMatches("\x16\x03", 2)));
  EXPECT_CALL(callback, peekSuccess_(BufMatches("\x16\x03", 2)));
  EXPECT_CALL(*sock, setReadCB(nullptr));
  peeker->readDataAvailable(1);
}

TEST_F(SocketPeekerTest, TestDestoryWhilePeeking) {
  EXPECT_CALL(*sock, setReadCB(_));
  SocketPeeker::UniquePtr peeker(new SocketPeeker(*sock, &callback, 2));
  peeker->start();
  peeker = nullptr;
}

TEST_F(SocketPeekerTest, TestNoPeekSuccess) {
  SocketPeeker::UniquePtr peeker(new SocketPeeker(*sock, &callback, 0));

  char buf = '\0';
  EXPECT_CALL(callback, peekSuccess_(BufMatches(&buf, 0)));
  peeker->start();
}

TEST_F(SocketPeekerTest, TestReadBufferAvailableOversized) {
  constexpr size_t kPeekSize = 13;
  const std::string data = std::string(kPeekSize, '\xAB') + "suffix";

  EXPECT_CALL(*sock, setReadCB(_));
  SocketPeeker::UniquePtr peeker(new SocketPeeker(*sock, &callback, kPeekSize));
  peeker->start();

  EXPECT_CALL(*sock, setReadCB(nullptr));
  EXPECT_CALL(
      *sock, _setPreReceivedData(IOBufMatches(data.data(), data.size())));
  EXPECT_CALL(callback, peekSuccess_(BufMatches(data.data(), kPeekSize)));
  peeker->readBufferAvailable(folly::IOBuf::copyBuffer(data));
}

TEST(TransportPeekerTest, TestReadBufferAvailableExactSize) {
  folly::EventBase evb;
  auto* sock = new MockAsyncSocket(&evb);
  MockSocketPeekerCallback callback;
  TransportPeeker::UniquePtr peeker(new TransportPeeker(sock, &callback, 13));

  auto buf = folly::IOBuf::create(13);
  buf->append(13);
  memset(buf->writableData(), 0xAB, 13);

  std::vector<uint8_t> peekedBytes;
  EXPECT_CALL(callback, peekSuccess_(_)).WillOnce(SaveArg<0>(&peekedBytes));
  EXPECT_CALL(*sock, setReadCB(nullptr));

  peeker->readBufferAvailable(std::move(buf));

  ASSERT_EQ(peekedBytes.size(), 13);
  for (int i = 0; i < 13; i++) {
    EXPECT_EQ(peekedBytes[i], 0xAB);
  }

  auto readData = peeker->moveReadData();
  ASSERT_NE(readData, nullptr);
  EXPECT_EQ(readData->computeChainDataLength(), 13);

  sock->destroy();
}

TEST(TransportPeekerTest, TestReadBufferAvailableOversized) {
  folly::EventBase evb;
  auto* sock = new MockAsyncSocket(&evb);
  MockSocketPeekerCallback callback;
  TransportPeeker::UniquePtr peeker(new TransportPeeker(sock, &callback, 13));

  auto buf = folly::IOBuf::create(2048);
  buf->append(2048);
  memset(buf->writableData(), 0xAB, 13);
  memset(buf->writableData() + 13, 0xCD, 2035);

  std::vector<uint8_t> peekedBytes;
  EXPECT_CALL(callback, peekSuccess_(_)).WillOnce(SaveArg<0>(&peekedBytes));
  EXPECT_CALL(*sock, setReadCB(nullptr));

  peeker->readBufferAvailable(std::move(buf));

  ASSERT_EQ(peekedBytes.size(), 13);
  for (int i = 0; i < 13; i++) {
    EXPECT_EQ(peekedBytes[i], 0xAB);
  }

  auto readData = peeker->moveReadData();
  ASSERT_NE(readData, nullptr);
  EXPECT_EQ(readData->computeChainDataLength(), 2048);
  EXPECT_EQ(readData->data()[0], 0xAB);
  EXPECT_EQ(readData->data()[13], 0xCD);

  sock->destroy();
}

TEST(TransportPeekerTest, TestReadBufferAvailableMultiple) {
  folly::EventBase evb;
  auto* sock = new MockAsyncSocket(&evb);
  MockSocketPeekerCallback callback;
  TransportPeeker::UniquePtr peeker(new TransportPeeker(sock, &callback, 13));

  auto buf1 = folly::IOBuf::create(5);
  buf1->append(5);
  memset(buf1->writableData(), 0xAA, 5);
  EXPECT_CALL(callback, peekSuccess_(_)).Times(0);
  peeker->readBufferAvailable(std::move(buf1));

  Mock::VerifyAndClearExpectations(&callback);

  auto buf2 = folly::IOBuf::create(8);
  buf2->append(8);
  memset(buf2->writableData(), 0xBB, 8);

  std::vector<uint8_t> peekedBytes;
  EXPECT_CALL(callback, peekSuccess_(_)).WillOnce(SaveArg<0>(&peekedBytes));
  EXPECT_CALL(*sock, setReadCB(nullptr));

  peeker->readBufferAvailable(std::move(buf2));

  ASSERT_EQ(peekedBytes.size(), 13);
  for (int i = 0; i < 5; i++) {
    EXPECT_EQ(peekedBytes[i], 0xAA);
  }
  for (int i = 5; i < 13; i++) {
    EXPECT_EQ(peekedBytes[i], 0xBB);
  }

  auto readData = peeker->moveReadData();
  ASSERT_NE(readData, nullptr);
  EXPECT_EQ(readData->computeChainDataLength(), 13);

  sock->destroy();
}

TEST(TransportPeekerTest, TestReadBufferAvailableMultipleOversized) {
  folly::EventBase evb;
  auto* sock = new MockAsyncSocket(&evb);
  MockSocketPeekerCallback callback;
  TransportPeeker::UniquePtr peeker(new TransportPeeker(sock, &callback, 13));

  auto buf1 = folly::IOBuf::create(5);
  buf1->append(5);
  memset(buf1->writableData(), 0xAA, 5);
  EXPECT_CALL(callback, peekSuccess_(_)).Times(0);
  peeker->readBufferAvailable(std::move(buf1));

  Mock::VerifyAndClearExpectations(&callback);

  auto buf2 = folly::IOBuf::create(2043);
  buf2->append(2043);
  memset(buf2->writableData(), 0xBB, 8);
  memset(buf2->writableData() + 8, 0xCC, 2035);

  std::vector<uint8_t> peekedBytes;
  EXPECT_CALL(callback, peekSuccess_(_)).WillOnce(SaveArg<0>(&peekedBytes));
  EXPECT_CALL(*sock, setReadCB(nullptr));

  peeker->readBufferAvailable(std::move(buf2));

  ASSERT_EQ(peekedBytes.size(), 13);
  for (int i = 0; i < 5; i++) {
    EXPECT_EQ(peekedBytes[i], 0xAA);
  }
  for (int i = 5; i < 13; i++) {
    EXPECT_EQ(peekedBytes[i], 0xBB);
  }

  auto readData = peeker->moveReadData();
  ASSERT_NE(readData, nullptr);
  EXPECT_EQ(readData->computeChainDataLength(), 2048);

  sock->destroy();
}
