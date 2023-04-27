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

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>
#include <thread>

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
