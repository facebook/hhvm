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

#include <wangle/acceptor/PeekingAcceptorHandshakeHelper.h>

#include <thread>

#include <folly/io/async/test/MockAsyncSSLSocket.h>
#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>
#include <wangle/acceptor/test/AcceptorHelperMocks.h>

using namespace folly;
using namespace folly::test;
using namespace wangle;
using namespace testing;

class MockPeekingCallback
    : public PeekingAcceptorHandshakeHelper::PeekCallback {
 public:
  using PeekingAcceptorHandshakeHelper::PeekCallback::PeekCallback;

  MOCK_METHOD4_T(
      getHelperInternal,
      AcceptorHandshakeHelper*(
          const std::vector<uint8_t>&,
          const folly::SocketAddress&,
          std::chrono::steady_clock::time_point,
          TransportInfo&));

  wangle::AcceptorHandshakeHelper::UniquePtr getHelper(
      const std::vector<uint8_t>& peekedBytes,
      const folly::SocketAddress& clientAddr,
      std::chrono::steady_clock::time_point acceptTime,
      TransportInfo& tinfo) override {
    return wangle::AcceptorHandshakeHelper::UniquePtr(
        getHelperInternal(peekedBytes, clientAddr, acceptTime, tinfo));
  }
};

class PeekingAcceptorHandshakeHelperTest : public Test {
 protected:
  void SetUp() override {
    sslSock_ = new MockAsyncSSLSocket(
        SSLContextPtr(new SSLContext()),
        &base_,
        true /* defer security negotiation */);
    sockPtr_ = AsyncSSLSocket::UniquePtr(sslSock_);

    peekCallbacks_.push_back(&mockPeekCallback1_);
    peekCallbacks_.push_back(&mockPeekCallback2_);

    helper_ = new PeekingAcceptorHandshakeHelper(
        sa_, std::chrono::steady_clock::now(), tinfo_, peekCallbacks_, 2);

    innerHelper_ = new MockHandshakeHelper<>();
    helperPtr_ = AcceptorHandshakeHelper::UniquePtr(innerHelper_);
  }

  void TearDown() override {
    // Normally this would be destoryed by the AcceptorHandshakeManager.
    helper_->destroy();
  }

  PeekingAcceptorHandshakeHelper* helper_;
  MockAsyncSSLSocket* sslSock_;
  AsyncSSLSocket::UniquePtr sockPtr_;
  EventBase base_;
  MockPeekingCallback mockPeekCallback1_{2};
  MockPeekingCallback mockPeekCallback2_{1};
  std::vector<PeekingCallbackPtr> peekCallbacks_;
  MockHandshakeHelper<UseSharedPtrPolicy>* innerHelper_;
  AcceptorHandshakeHelper::UniquePtr helperPtr_;
  StrictMock<MockHandshakeHelperCallback<>> callback_;
  TransportInfo tinfo_;
  SocketAddress sa_;
};

TEST_F(PeekingAcceptorHandshakeHelperTest, TestPeekSuccess) {
  helper_->start(std::move(sockPtr_), &callback_);
  // first 2 bytes of SSL3+.
  std::vector<uint8_t> buf(2);
  buf[0] = 0x16;
  buf[1] = 0x03;
  EXPECT_CALL(mockPeekCallback1_, getHelperInternal(_, _, _, _))
      .WillOnce(Return(helperPtr_.release()));
  EXPECT_CALL(*innerHelper_, startInternal(_, _));
  helper_->peekSuccess(buf);
}

TEST_F(PeekingAcceptorHandshakeHelperTest, TestPeekNonSuccess) {
  helper_->start(std::move(sockPtr_), &callback_);
  // first 2 bytes of SSL3+.
  std::vector<uint8_t> buf(2);
  buf[0] = 0x16;
  buf[1] = 0x03;
  EXPECT_CALL(mockPeekCallback1_, getHelperInternal(_, _, _, _))
      .WillOnce(Return(nullptr));
  EXPECT_CALL(mockPeekCallback2_, getHelperInternal(_, _, _, _))
      .WillOnce(Return(nullptr));
  EXPECT_CALL(callback_, connectionError_(_, _, _));
  helper_->peekSuccess(buf);
}

TEST_F(PeekingAcceptorHandshakeHelperTest, TestPeek2ndSuccess) {
  helper_->start(std::move(sockPtr_), &callback_);
  // first 2 bytes of SSL3+.
  std::vector<uint8_t> buf(2);
  buf[0] = 0x16;
  buf[1] = 0x03;
  EXPECT_CALL(mockPeekCallback1_, getHelperInternal(_, _, _, _))
      .WillOnce(Return(nullptr));
  EXPECT_CALL(mockPeekCallback2_, getHelperInternal(_, _, _, _))
      .WillOnce(Return(helperPtr_.release()));
  EXPECT_CALL(*innerHelper_, startInternal(_, _));
  helper_->peekSuccess(buf);
}

TEST_F(PeekingAcceptorHandshakeHelperTest, TestEOFDuringPeek) {
  AsyncTransport::ReadCallback* rcb;
  EXPECT_CALL(*sslSock_, setReadCB(_)).WillOnce(SaveArg<0>(&rcb));
  EXPECT_CALL(*sslSock_, setReadCB(nullptr));
  EXPECT_CALL(callback_, connectionError_(_, _, _));
  helper_->start(std::move(sockPtr_), &callback_);
  ASSERT_TRUE(rcb);
  rcb->readEOF();
}

TEST_F(PeekingAcceptorHandshakeHelperTest, TestPeekErr) {
  helper_->start(std::move(sockPtr_), &callback_);
  EXPECT_CALL(callback_, connectionError_(_, _, _));
  helper_->peekError(AsyncSocketException(
      AsyncSocketException::AsyncSocketExceptionType::END_OF_FILE,
      "Unit test"));
}

TEST_F(PeekingAcceptorHandshakeHelperTest, TestDropDuringPeek) {
  helper_->start(std::move(sockPtr_), &callback_);

  EXPECT_CALL(*sslSock_, closeNow()).Times(AtLeast(1)).WillOnce(Invoke([&] {
    helper_->peekError(AsyncSocketException(
        AsyncSocketException::AsyncSocketExceptionType::UNKNOWN, "unit test"));
  }));
  EXPECT_CALL(callback_, connectionError_(_, _, _));
  helper_->dropConnection();
}

TEST_F(PeekingAcceptorHandshakeHelperTest, TestDropAfterPeek) {
  helper_->start(std::move(sockPtr_), &callback_);
  std::vector<uint8_t> buf(2);
  // first 2 bytes of SSL3+.
  buf[0] = 0x16;
  buf[1] = 0x03;

  EXPECT_CALL(mockPeekCallback1_, getHelperInternal(_, _, _, _))
      .WillOnce(Return(helperPtr_.release()));
  EXPECT_CALL(*innerHelper_, startInternal(_, _));
  helper_->peekSuccess(buf);

  EXPECT_CALL(*innerHelper_, dropConnection(_));
  helper_->dropConnection();
}
