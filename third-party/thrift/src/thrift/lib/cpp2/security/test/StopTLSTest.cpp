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

#include <fizz/protocol/test/Mocks.h>
#include <thrift/lib/cpp2/security/AsyncStopTLS.h>
#include <thrift/lib/cpp2/security/FizzPeeker.h>

using namespace apache::thrift;
using namespace testing;

class MockStopTLSCallback : public AsyncStopTLS::Callback {
 public:
  MOCK_METHOD1(stopTLSSuccess, void(std::unique_ptr<folly::IOBuf>));
  MOCK_METHOD1(stopTLSError, void(const folly::exception_wrapper&));
};

class AsyncStopTLSTest : public Test {
 protected:
  fizz::test::MockAsyncFizzBase transport_;
  StrictMock<MockStopTLSCallback> callback_;
};

static const folly::AsyncSocketException socketErr(
    folly::AsyncSocketException::NETWORK_ERROR, "something bad happened");

// Simulates what would happen if the socket we attempted to start stoptls on
// was already closed by the peer. We should not even begin to invoke
// tlsShutdown
TEST_F(AsyncStopTLSTest, SynchronousReadEOF) {
  AsyncStopTLS::UniquePtr operation(new AsyncStopTLS(callback_));

  EXPECT_CALL(callback_, stopTLSError(_));
  EXPECT_CALL(transport_, tlsShutdown()).Times(0);
  EXPECT_CALL(transport_, setReadCB(_)).WillOnce(Invoke([](auto&& cb) {
    cb->readEOF();
  }));
  EXPECT_CALL(transport_, setReadCB(nullptr));
  operation->start(
      &transport_, AsyncStopTLS::Role::Server, std::chrono::milliseconds(0));
}

// Simulates what would happen if the socket we attempted to start stoptls on
// was in a bad state. We should not even begin to invoke `tlsShutdown()`
TEST_F(AsyncStopTLSTest, SynchronousReadErr) {
  AsyncStopTLS::UniquePtr operation(new AsyncStopTLS(callback_));

  EXPECT_CALL(callback_, stopTLSError(_));
  EXPECT_CALL(transport_, tlsShutdown()).Times(0);
  EXPECT_CALL(transport_, setReadCB(_)).WillOnce(Invoke([](auto&& cb) {
    cb->readErr(socketErr);
  }));
  EXPECT_CALL(transport_, setReadCB(nullptr));
  operation->start(
      &transport_, AsyncStopTLS::Role::Server, std::chrono::milliseconds(0));
}

// Simulates what would happen if peer misbehaved and closed connection instead
// of sending close_notify. We need to still be able to get the callback.
TEST_F(AsyncStopTLSTest, AsynchronousReadEOF) {
  AsyncStopTLS::UniquePtr operation(new AsyncStopTLS(callback_));

  fizz::AsyncFizzBase::ReadCallback* readCB{nullptr};
  EXPECT_CALL(transport_, setReadCB(_)).WillOnce(SaveArg<0>(&readCB));
  EXPECT_CALL(transport_, tlsShutdown());
  operation->start(
      &transport_, AsyncStopTLS::Role::Server, std::chrono::milliseconds(0));
  Mock::VerifyAndClearExpectations(&transport_);
  ASSERT_NE(readCB, nullptr);

  EXPECT_CALL(transport_, setReadCB(nullptr));
  EXPECT_CALL(callback_, stopTLSError(_));
  readCB->readEOF();
}

// Simulates what would happen if helper_->dropConnection() were called on
// in between the time we finished the TLS handshake and when we are waiting
// for a close notify
TEST_F(AsyncStopTLSTest, AsynchronousReadErr) {
  AsyncStopTLS::UniquePtr operation(new AsyncStopTLS(callback_));

  fizz::AsyncFizzBase::ReadCallback* readCB{nullptr};
  EXPECT_CALL(transport_, setReadCB(_)).WillOnce(SaveArg<0>(&readCB));
  EXPECT_CALL(transport_, tlsShutdown());
  operation->start(
      &transport_, AsyncStopTLS::Role::Server, std::chrono::milliseconds(0));
  Mock::VerifyAndClearExpectations(&transport_);
  ASSERT_NE(readCB, nullptr);

  EXPECT_CALL(transport_, setReadCB(nullptr));
  EXPECT_CALL(callback_, stopTLSError(_));
  readCB->readErr(socketErr);
}

static const std::unique_ptr<folly::IOBuf> kUnexpectedData =
    folly::IOBuf::copyBuffer("hello");

// Simulates what would happen if the peer did not implement StopTLS properly
// and instead of sending a close_notify, started sending application data.
TEST_F(AsyncStopTLSTest, SynchronousUnexpectedApplicationData) {
  AsyncStopTLS::UniquePtr operation(new AsyncStopTLS(callback_));

  EXPECT_CALL(callback_, stopTLSError(_));
  EXPECT_CALL(transport_, tlsShutdown()).Times(0);
  EXPECT_CALL(transport_, setReadCB(_)).WillOnce(Invoke([](auto&& cb) {
    cb->readBufferAvailable(kUnexpectedData->clone());
  }));
  EXPECT_CALL(transport_, setReadCB(nullptr));
  operation->start(
      &transport_, AsyncStopTLS::Role::Server, std::chrono::milliseconds(0));
}

TEST_F(AsyncStopTLSTest, AsynchronousUnexpectedApplicationData) {
  AsyncStopTLS::UniquePtr operation(new AsyncStopTLS(callback_));

  fizz::AsyncFizzBase::ReadCallback* readCB{nullptr};
  EXPECT_CALL(transport_, setReadCB(_)).WillOnce(SaveArg<0>(&readCB));
  EXPECT_CALL(transport_, tlsShutdown());
  operation->start(
      &transport_, AsyncStopTLS::Role::Server, std::chrono::milliseconds(0));
  Mock::VerifyAndClearExpectations(&transport_);
  ASSERT_NE(readCB, nullptr);

  EXPECT_CALL(transport_, setReadCB(nullptr));
  EXPECT_CALL(callback_, stopTLSError(_));
  readCB->readBufferAvailable(kUnexpectedData->clone());
}

// Simulates a successful stoptls connection that fires synchronously w.r.t
// tlsShutdown()
TEST_F(AsyncStopTLSTest, SynchronousStopTLSSuccess) {
  AsyncStopTLS::UniquePtr operation(new AsyncStopTLS(callback_));
  fizz::AsyncFizzBase::EndOfTLSCallback* stoptlsCB{nullptr};

  EXPECT_CALL(callback_, stopTLSSuccess(_))
      .WillOnce(Invoke([](auto&& postData) {
        EXPECT_EQ(
            postData->computeChainDataLength(),
            kUnexpectedData->computeChainDataLength());
      }));

  EXPECT_CALL(transport_, setReadCB(Ne(nullptr)));
  EXPECT_CALL(transport_, setReadCB(nullptr));
  EXPECT_CALL(transport_, setEndOfTLSCallback(Ne(nullptr)))
      .WillOnce(SaveArg<0>(&stoptlsCB));
  EXPECT_CALL(transport_, setEndOfTLSCallback(Eq(nullptr)));
  EXPECT_CALL(transport_, tlsShutdown()).WillOnce(Invoke([&]() {
    ASSERT_NE(stoptlsCB, nullptr);
    stoptlsCB->endOfTLS(&transport_, kUnexpectedData->clone());
  }));
  operation->start(
      &transport_, AsyncStopTLS::Role::Server, std::chrono::milliseconds(0));
}
//
// Simulates a successful stoptls connection that fires asynchronously --
// this is the case we expect to be hit the most.
TEST_F(AsyncStopTLSTest, AsynchronousStopTLSSuccess) {
  AsyncStopTLS::UniquePtr operation(new AsyncStopTLS(callback_));
  fizz::AsyncFizzBase::EndOfTLSCallback* stoptlsCB{nullptr};

  EXPECT_CALL(transport_, setReadCB(_));
  EXPECT_CALL(transport_, setEndOfTLSCallback(_))
      .WillOnce(SaveArg<0>(&stoptlsCB));
  EXPECT_CALL(transport_, tlsShutdown());
  operation->start(
      &transport_, AsyncStopTLS::Role::Server, std::chrono::milliseconds(0));
  Mock::VerifyAndClearExpectations(&transport_);

  ASSERT_NE(stoptlsCB, nullptr);

  EXPECT_CALL(callback_, stopTLSSuccess(_))
      .WillOnce(Invoke([](auto&& postData) {
        EXPECT_EQ(
            postData->computeChainDataLength(),
            kUnexpectedData->computeChainDataLength());
      }));
  EXPECT_CALL(transport_, setReadCB(nullptr));
  stoptlsCB->endOfTLS(&transport_, kUnexpectedData->clone());
}

// Simulates a client initiating StopTLS, but never receiving the server sent
// close_notify within a specified timeout.
TEST_F(AsyncStopTLSTest, ClientStopTLSTimeoutError) {
  folly::EventBase evb;
  AsyncStopTLS::UniquePtr operation(new AsyncStopTLS(callback_));

  fizz::AsyncFizzBase::EndOfTLSCallback* stoptlsCB{nullptr};
  EXPECT_CALL(transport_, setReadCB(Ne(nullptr)));
  EXPECT_CALL(transport_, setReadCB(Eq(nullptr)));
  EXPECT_CALL(transport_, setEndOfTLSCallback(Ne(nullptr)))
      .WillOnce(SaveArg<0>(&stoptlsCB));
  EXPECT_CALL(transport_, setEndOfTLSCallback(Eq(nullptr)));

  // We are the client, we should not initiate `tlsShutdown`
  EXPECT_CALL(transport_, tlsShutdown()).Times(0);

  auto mockUnderlyingTransport =
      transport_.getUnderlyingTransport<folly::test::MockAsyncTransport>();
  ASSERT_NE(mockUnderlyingTransport, nullptr);
  EXPECT_CALL(*mockUnderlyingTransport, getEventBase())
      .WillRepeatedly(Return(&evb));

  EXPECT_CALL(callback_, stopTLSError(_));
  operation->start(
      &transport_, AsyncStopTLS::Role::Client, std::chrono::milliseconds(10));
  evb.loop();
}

// Simulates a client initiating StopTLS with a timeout, and successfully
// negotiating it.
TEST_F(AsyncStopTLSTest, ClientStopTLS) {
  folly::EventBase evb;
  AsyncStopTLS::UniquePtr operation(new AsyncStopTLS(callback_));

  fizz::AsyncFizzBase::EndOfTLSCallback* stoptlsCB{nullptr};
  EXPECT_CALL(transport_, setEndOfTLSCallback(Ne(nullptr)))
      .WillOnce(SaveArg<0>(&stoptlsCB));
  EXPECT_CALL(transport_, setEndOfTLSCallback(Eq(nullptr)));

  // We are the client, we should not initiate `tlsShutdown`
  EXPECT_CALL(transport_, tlsShutdown()).Times(0);

  auto mockUnderlyingTransport =
      transport_.getUnderlyingTransport<folly::test::MockAsyncTransport>();
  ASSERT_NE(mockUnderlyingTransport, nullptr);
  EXPECT_CALL(*mockUnderlyingTransport, getEventBase())
      .WillRepeatedly(Return(&evb));

  EXPECT_CALL(callback_, stopTLSSuccess(_));
  operation->start(
      &transport_, AsyncStopTLS::Role::Client, std::chrono::milliseconds(10));
  ASSERT_NE(stoptlsCB, nullptr);

  evb.runInLoop([=, this] { stoptlsCB->endOfTLS(&transport_, nullptr); });
  evb.loop();
}
