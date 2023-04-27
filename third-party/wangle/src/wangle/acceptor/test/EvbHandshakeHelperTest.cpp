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

#include <wangle/acceptor/EvbHandshakeHelper.h>

#include <chrono>
#include <thread>

#include <folly/futures/Barrier.h>
#include <folly/io/async/ScopedEventBaseThread.h>
#include <folly/io/async/test/MockAsyncSSLSocket.h>
#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>
#include <folly/synchronization/Baton.h>

#include <wangle/acceptor/test/AcceptorHelperMocks.h>

using namespace std::chrono_literals;
using namespace folly;
using namespace folly::test;
using namespace wangle;
using namespace testing;

class EvbHandshakeHelperTest : public Test {
 protected:
  void SetUp() override {
    original_.getEventBase()->runInEventBaseThreadAndWait([=] {
      originalThreadId_ = std::this_thread::get_id();
      auto evb = original_.getEventBase();
      auto sslSock =
          new MockAsyncSSLSocket(std::make_shared<SSLContext>(), evb, true);

      sslSock_ = sslSock;
      sockPtr_.reset(sslSock);
    });

    alternate_.getEventBase()->runInEventBaseThreadAndWait(
        [=] { alternateThreadId_ = std::this_thread::get_id(); });

    mockHelper_ = new MockHandshakeHelper<UseOwnedRawPtrPolicy>();

    evbHelper_ = new EvbHandshakeHelper(
        AcceptorHandshakeHelper::UniquePtr(mockHelper_),
        alternate_.getEventBase());
  }

  void TearDown() override {
    if (evbHelper_) {
      evbHelper_->destroy();
    }
  }

  ScopedEventBaseThread original_;
  ScopedEventBaseThread alternate_;

  std::atomic<std::thread::id> originalThreadId_;
  std::atomic<std::thread::id> alternateThreadId_;

  EvbHandshakeHelper* evbHelper_{nullptr};
  MockHandshakeHelper<UseOwnedRawPtrPolicy>* mockHelper_{nullptr};

  MockHandshakeHelperCallback<UseOwnedRawPtrPolicy> mockCb_;

  MockAsyncSSLSocket* sslSock_{nullptr};
  AsyncSSLSocket::UniquePtr sockPtr_{nullptr};
};

TEST_F(EvbHandshakeHelperTest, TestSuccessPath) {
  folly::Baton<> barrier;

  EXPECT_CALL(mockCb_, connectionReadyInternalRaw(_, _, _, _))
      .WillOnce(Invoke([&](auto sock, auto nextProtocol, auto&&, auto&&) {
        EXPECT_EQ(original_.getEventBase(), sock->getEventBase());
        EXPECT_EQ(originalThreadId_, std::this_thread::get_id());
        EXPECT_EQ("h2", nextProtocol);
        sock->destroy();
        barrier.post();
      }));

  EXPECT_CALL(*mockHelper_, startInternal(_, _))
      .WillOnce(Invoke([&](auto sock, auto cb) {
        EXPECT_EQ(alternate_.getEventBase(), sock->getEventBase());
        EXPECT_EQ(alternateThreadId_, std::this_thread::get_id());

        sock->getEventBase()->runInLoop([sock, cb] {
          cb->connectionReady(
              AsyncTransport::UniquePtr(sock),
              "h2",
              SecureTransportType::TLS,
              folly::none);
        });
      }));

  original_.getEventBase()->runInEventBaseThreadAndWait(
      [=] { evbHelper_->start(std::move(sockPtr_), &mockCb_); });

  if (!barrier.try_wait_for(2s)) {
    FAIL() << "Timeout waiting for connectionReady callback to be called";
  }
}

TEST_F(EvbHandshakeHelperTest, TestFailPath) {
  folly::Baton<> barrier;

  EXPECT_NE(nullptr, sockPtr_->getEventBase());
  EXPECT_CALL(mockCb_, connectionError_(_, _, _))
      .WillOnce(Invoke([&](auto sock, auto&&, auto&&) {
        EXPECT_EQ(sock, nullptr);
        EXPECT_EQ(originalThreadId_, std::this_thread::get_id());
        barrier.post();
      }));

  EXPECT_CALL(*mockHelper_, startInternal(_, _))
      .WillOnce(Invoke([&](auto sock, auto cb) {
        EXPECT_EQ(alternate_.getEventBase(), sock->getEventBase());
        EXPECT_EQ(alternateThreadId_, std::this_thread::get_id());

        sock->getEventBase()->runInLoop([sock, cb, this] {
          folly::DelayedDestruction::DestructorGuard dg(mockHelper_);
          EXPECT_FALSE(mockHelper_->getDestroyPending());
          cb->connectionError(sock, {}, folly::none);
          EXPECT_TRUE(mockHelper_->getDestroyPending());
          EXPECT_EQ(alternate_.getEventBase(), sock->getEventBase());
          sock->destroy();
        });
      }));

  original_.getEventBase()->runInEventBaseThreadAndWait(
      [=] { evbHelper_->start(std::move(sockPtr_), &mockCb_); });

  if (!barrier.try_wait_for(2s)) {
    FAIL() << "Timeout while waiting for connectionError callback to be called";
  }
}

TEST_F(EvbHandshakeHelperTest, TestDropConnection) {
  folly::Baton<> barrier;

  EXPECT_CALL(*mockHelper_, dropConnection(_)).WillOnce(Invoke([&](auto) {
    EXPECT_EQ(alternateThreadId_, std::this_thread::get_id());
    CHECK(alternate_.getEventBase()->isInEventBaseThread());
    evbHelper_->connectionError(sslSock_, {}, {});
    barrier.post();
  }));

  AsyncTransport* transport;
  EXPECT_CALL(mockCb_, connectionError_(_, _, _))
      .WillOnce(SaveArg<0>(&transport));

  EXPECT_CALL(*mockHelper_, startInternal(_, _))
      .WillOnce(Invoke([&](auto sock, auto&&) {
        EXPECT_EQ(alternate_.getEventBase(), sock->getEventBase());
        EXPECT_EQ(alternateThreadId_, std::this_thread::get_id());
        sslSock_ = dynamic_cast<MockAsyncSSLSocket*>(sock);
        sockPtr_.reset(sslSock_);
        barrier.post();
      }));

  original_.getEventBase()->runInEventBaseThreadAndWait(
      [=] { evbHelper_->start(std::move(sockPtr_), &mockCb_); });

  if (!barrier.try_wait_for(2s)) {
    FAIL() << "Timeout while waiting for startInternal to be called";
  }

  barrier.reset();

  original_.getEventBase()->runInEventBaseThreadAndWait(
      [=] { evbHelper_->dropConnection(SSLErrorEnum::DROPPED); });

  if (!barrier.try_wait_for(2s)) {
    FAIL() << "Timeout while waiting for dropConnection to be called";
  }

  EXPECT_EQ(nullptr, transport);
}

TEST_F(EvbHandshakeHelperTest, TestDropConnectionTricky) {
  folly::Baton<> barrier;
  folly::Baton<> connectionReadyCalled;
  folly::futures::Barrier raceBarrier(3);

  // One of these two methods will be called depending on the race, but
  // not both of them.
  bool called = false;
  EXPECT_CALL(mockCb_, connectionError_(_, _, _))
      .Times(AtMost(1))
      .WillOnce(Invoke([&](auto, auto, auto) {
        EXPECT_FALSE(called);
        called = true;
        barrier.post();
      }));

  EXPECT_CALL(mockCb_, connectionReadyInternalRaw(_, _, _, _))
      .Times(AtMost(1))
      .WillOnce(Invoke([&](auto, auto, auto, auto) {
        EXPECT_FALSE(called);
        called = true;
        barrier.post();
      }));

  EXPECT_CALL(*mockHelper_, startInternal(_, _))
      .WillOnce(Invoke([&](auto sock, auto&&) {
        EXPECT_EQ(alternate_.getEventBase(), sock->getEventBase());
        EXPECT_EQ(alternateThreadId_, std::this_thread::get_id());
        sslSock_ = dynamic_cast<MockAsyncSSLSocket*>(sock);
        sockPtr_.reset(sslSock_);
        barrier.post();
      }));

  original_.getEventBase()->runInEventBaseThreadAndWait(
      [=] { evbHelper_->start(std::move(sockPtr_), &mockCb_); });

  if (!barrier.try_wait_for(2s)) {
    FAIL() << "Timeout while waiting for startInternal to be called";
  }

  barrier.reset();

  // Race the dropConnection() and handshakeSuccess() calls
  original_.getEventBase()->runInEventBaseThread([=, &raceBarrier] {
    raceBarrier.wait().get();
    evbHelper_->dropConnection(SSLErrorEnum::DROPPED);
  });

  alternate_.getEventBase()->runInEventBaseThread(
      [=, &raceBarrier, &connectionReadyCalled]() mutable {
        raceBarrier.wait().get();
        evbHelper_->connectionReady(std::move(sockPtr_), "test", {}, {});
        connectionReadyCalled.post();
      });

  raceBarrier.wait();

  if (!barrier.try_wait_for(2s)) {
    FAIL() << "Timeout while waiting for connectionError to be called";
  }

  if (!connectionReadyCalled.try_wait_for(2s)) {
    FAIL() << "Timeout while waiting for connectionReady to call";
  }
}
