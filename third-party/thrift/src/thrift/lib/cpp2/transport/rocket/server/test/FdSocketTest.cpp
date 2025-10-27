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

#include <chrono>

#include <gmock/gmock.h>
#include <folly/coro/Baton.h>
#include <folly/testing/TestUtil.h>

#include <thrift/lib/cpp2/async/AsyncProcessor.h>
#include <thrift/lib/cpp2/async/PooledRequestChannel.h>
#include <thrift/lib/cpp2/server/Cpp2Worker.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>
#include <thrift/lib/cpp2/test/gen-cpp2/TestServiceAsyncClient.h>
#include <thrift/lib/cpp2/test/util/TestHandler.h>
#include <thrift/lib/cpp2/transport/rocket/test/util/FdUtils.h>
#include <thrift/lib/cpp2/util/ScopedServerInterfaceThread.h>

using namespace apache::thrift;
using namespace apache::thrift::rocket::test;
using namespace std::literals;

struct ServerResponseEnqueuedInterface : public TestHandler {
  explicit ServerResponseEnqueuedInterface(
      MessageQueue* sendQueue, folly::Baton<>& responseEnqueuedBaton)
      : sendQueue_(sendQueue), responseEnqueuedBaton_(responseEnqueuedBaton) {}

  void async_eb_eventBaseAsync(
      apache::thrift::HandlerCallbackPtr<std::unique_ptr<std::string>> callback)
      override {
    MessagePair resPair;
    CHECK(sendQueue_->read(resPair));
    auto& [outData, outFds] = resPair;
    LOG(INFO) << "Sending " << renderData(*outData) << " / "
              << renderFds(outFds);

    callback->getRequestContext()->getHeader()->fds.dcheckEmpty() =
        folly::SocketFds{std::move(outFds)};

    // Since `eventBaseAsync` is a `thread = 'eb'` method, this runs on
    // the IO thread, and we can guarantee that the baton is posted
    // no earlier than the write was enqueued to the WriteBatcher.
    //
    // In contrast, if we posted the baton from a regular request handler
    // thread pool, there would be a chance that it would fire BEFORE the IO
    // thread could enqueue the write.
    callback->getEventBase()->dcheckIsInEventBaseThread();
    callback->getEventBase()->runInEventBaseThread(
        [&]() mutable { responseEnqueuedBaton_.post(); });

    callback->result(std::move(outData));
  }

  MessageQueue* sendQueue_;
  folly::Baton<>& responseEnqueuedBaton_;
};

struct InterceptedCpp2Worker : public Cpp2Worker {
  InterceptedCpp2Worker(MessageQueue* q, ThriftServer* s)
      : Cpp2Worker(s, {}), checkQueue_(q) {}
  static std::shared_ptr<InterceptedCpp2Worker> create(
      MessageQueue* checkQueue, ThriftServer* server, folly::EventBase* evb) {
    auto worker = std::make_shared<InterceptedCpp2Worker>(checkQueue, server);
    worker->construct(server, evb, nullptr);
    return worker;
  }
  folly::AsyncSocket::UniquePtr makeNewAsyncSocket(
      folly::EventBase* base,
      int fd,
      const folly::SocketAddress* peerAddress) override {
    LOG(INFO) << "InterceptedCpp2Worker making new InterceptedAsyncFdSocket";
    return folly::AsyncSocket::UniquePtr(new InterceptedAsyncFdSocket(
        checkQueue_, base, folly::NetworkSocket::fromFd(fd), peerAddress));
  }
  MessageQueue* checkQueue_;
};

struct InterceptedAcceptorFactory : wangle::AcceptorFactory {
  InterceptedAcceptorFactory(MessageQueue* q, ThriftServer* s)
      : checkQueue_(q), server_(s) {}
  std::shared_ptr<wangle::Acceptor> newAcceptor(
      folly::EventBase* evb) override {
    return InterceptedCpp2Worker::create(checkQueue_, server_, evb);
  }
  MessageQueue* checkQueue_;
  ThriftServer* server_;
};

// An FD-centric variant of `WriteBatchingTest` in `ThriftServerTest.cpp`
class FdWriteBatchingTest : public testing::TestWithParam<bool> {
 protected:
  folly::test::TemporaryDirectory tempDir_;

  // Configure these BEFORE calling `init()`.
  //
  // `init()` immediately writes `serverSends_` into the `MessageQueue`s for
  // the server to send out as the requests come in.  We keep around these
  // FD references here, so that we can verify if they're still in any kind
  // of Thrift internals by peeking at `use_count()`.
  std::vector<std::tuple<folly::SocketFds::ToSend, std::string, std::string>>
      serverSends_;
  size_t batchingSize_{std::numeric_limits<size_t>::max()};

  MessageQueue sendQueue_{1000}; // never fills up
  MessageQueue checkQueue_{1000};

  std::unique_ptr<ScopedServerInterfaceThread> runner_;
  std::unique_ptr<Client<test::TestService>> client_;
  folly::Baton<> baton_;

  // We have both intercepted and non-intercepted test variants because
  // always intercepting the acceptor would deny coverage to
  // Cpp2Worker::makeNewAsyncSocket.
  std::optional<bool> isIntercepted_; // invalid until after `init()`

  void init() {
    CHECK_NE(std::numeric_limits<size_t>::max(), batchingSize_)
        << "Must set batchingSize_";
    CHECK_GT(serverSends_.size(), 0) << "Must set serverSends_";

    isIntercepted_ = GetParam();

    // Pre-fill the queues with responses for the expected number of requests.
    for (const auto& [fds, data, dataRe] : serverSends_) {
      sendQueue_.blockingWrite(
          std::make_pair(std::make_unique<std::string>(data), fds));
      // In `BatchWithoutFDs`, e.g., we have fewer "check" events than
      // requests due to batching.
      if (!dataRe.empty()) {
        checkQueue_.blockingWrite(
            std::make_pair(std::make_unique<std::string>(dataRe), fds));
      }
    }
    // Pad the queues so they don't run dry during TearDown
    for (size_t i = 0; i < batchingSize_ + 1; ++i) {
      sendQueue_.blockingWrite(
          std::make_pair(
              std::make_unique<std::string>("FdWriteBatchingTest::TearDown"),
              folly::SocketFds::ToSend{}));
      checkQueue_.blockingWrite(
          std::make_pair(
              std::make_unique<std::string>(
                  "(FdWriteBatchingTest::TearDown" // batched test cases
                  "|TTransportException: Channel got EOF)" // unbatched
                  ),
              folly::SocketFds::ToSend{}));
    }

    folly::SocketAddress sockAddr;
    sockAddr.setFromPath((tempDir_.path() / "sock").string());

    runner_ = std::make_unique<ScopedServerInterfaceThread>(
        std::make_shared<ServerResponseEnqueuedInterface>(&sendQueue_, baton_),
        sockAddr,
        [this](ThriftServer& server) {
          server.setWriteBatchingSize(batchingSize_);
          // For this test, the batching strategy does not matter.  For
          // simplicity, prevent flushing the batch due to time or byte size.
          server.setWriteBatchingInterval(batchingSize_ ? 600s : 0s);
          server.setWriteBatchingByteSize(0);
          if (*isIntercepted_) {
            server.setAcceptorFactory(
                std::make_shared<InterceptedAcceptorFactory>(
                    &checkQueue_, &server));
          }
        });

    client_ = std::make_unique<Client<test::TestService>>(
        PooledRequestChannel::newChannel(
            nullptr,
            [sockAddr](folly::EventBase& evb) {
              return RocketClientChannel::newChannel(
                  folly::AsyncSocket::UniquePtr(
                      new folly::AsyncFdSocket(&evb, sockAddr)));
            },
            /*numThreads*/ 1));
  }

  void waitUntilServerWriteScheduled() {
    baton_.wait();
    baton_.reset();
  }

  void TearDown() override {
    // If we have configured the Thrift Server to use write batching, the
    // last ErrorFrame that is sent by the server during destruction will be
    // buffered and will cause the test to wait until the buffer is flushed. We
    // can avoid this by sending dummy requests before the ErrorFrame is queued
    // so that the buffer is flushed immediately after the ErrorFrame is queued.
    if (batchingSize_) {
      for (size_t i = 0; i < batchingSize_ - 1; ++i) { // -1 for the ErrorFrame
        client_->semifuture_eventBaseAsync();
        waitUntilServerWriteScheduled();
      }
    }
  }

  auto newFd() {
    return std::make_shared<folly::File>(
        folly::File{2, /*ownsFd*/ false}.dupCloseOnExec());
  }

  bool issueRequestAndAwaitFulfillment(std::chrono::milliseconds timeout) {
    auto f = client_->semifuture_eventBaseAsync();
    waitUntilServerWriteScheduled();
    return std::move(f).wait(timeout);
  }

  // Although the server handler already ran for `numRequests`, neither the
  // data, nor the FDs were yet written to the socket.
  void expectNumRequestsInBatch(int numRequests) {
    EXPECT_EQ(numRequests, sendQueue_.readCount());
    EXPECT_EQ(0, checkQueue_.readCount());
    for (const auto& [fds, _d, _re] : serverSends_) {
      expectUseCount(3, fds); // `serverSends_` + Rocket queue + `checkQueue_`
    }
  }

  // The server handler ran for each expected request, and everything was
  // flushed to the socket.
  void expectAllRequestsCompleted(int numSocketCalls = 0) {
    EXPECT_EQ(serverSends_.size(), sendQueue_.readCount());
    if (*isIntercepted_) {
      EXPECT_EQ(
          numSocketCalls ? numSocketCalls : serverSends_.size(),
          checkQueue_.readCount());
    }
    for (const auto& [fds, _d, _re] : serverSends_) { // All FDs have been sent
      // `serverSends_` + `checkQueue_` if not intercepted
      expectUseCount(*isIntercepted_ ? 1 : 2, fds);
    }
  }
};

constexpr auto kShort = "DoneShort";
constexpr auto kShortSendTimeout = 100ms;

// Check the unbatched processing of a single short request.
// We reuse the same short request & timeout in a later, batched test.
TEST_P(FdWriteBatchingTest, Unbatched) {
  serverSends_.emplace_back(
      folly::SocketFds::ToSend{newFd(), newFd()}, kShort, kShort);
  batchingSize_ = 0;
  init();

  // Without batching, the request is fulfilled promptly.
  EXPECT_TRUE(issueRequestAndAwaitFulfillment(kShortSendTimeout));
  expectAllRequestsCompleted();
}
// This covers several aspects of the interaction of FD passing with:
//  - (Un)batching: each set of FDs gets a separate socket call, even when
//    `WriteBatcher` is active.
//  - Fragment frame handling: if a request is split across multiple frames,
//    the socket gets the FDs together with the last frame -- actually, in
//    the current implementation, it gets all the frames at once.
//  - The right data matches with the right FDs.
TEST_P(FdWriteBatchingTest, FlushBatchDueToWriteCount) {
  CHECK_GE(apache::thrift::rocket::kMaxFragmentedPayloadSize, 10000);
  // Server uses one element of `serverSends_` per `eventBaseAsync()` call
  serverSends_.emplace_back(
      folly::SocketFds::ToSend{newFd(), newFd()}, kShort, kShort);
  serverSends_.emplace_back(
      folly::SocketFds::ToSend{newFd()},
      // Make this server response large enough that it HAS to be split into
      // two fragments.
      "StartLong" +
          std::string(apache::thrift::rocket::kMaxFragmentedPayloadSize, '%') +
          "EndLong",
      // The regex needs to match some protocol bytes between fragments.
      folly::to<std::string>(
          "StartLong"
          // Almost all of the payload (% chars) should be in the first frame.
          // Add `+` to make the quantifier greedy for faster matching.
          "%{",
          apache::thrift::rocket::kMaxFragmentedPayloadSize - 1000,
          ",}+"
          // Match some non-% protocol bytes between fragments
          ".{1,100}[^%].{1,100}"
          // Second frame with the tail end of the message
          "%{1,1000}+EndLong"));
  batchingSize_ = serverSends_.size() + 1; // +1 for SetupFrame
  init();

  // Send first request. This will cause 2 writes to be buffered on the server
  // (1 SetupFrame and 1 response). Ensure we don't get a response.
  EXPECT_FALSE(issueRequestAndAwaitFulfillment(kShortSendTimeout));
  expectNumRequestsInBatch(1);

  // Send second request. This will cause batching size limit to be reached and
  // buffered writes will be flushed. Ensure we get a response.
  //
  // Use a long timeout since the intercepted socket runs an 400ms regex
  // match.  In practice, we'll never wait this long.
  EXPECT_TRUE(issueRequestAndAwaitFulfillment(10s));
  expectAllRequestsCompleted();
}

// Covers two behaviors:
//  - Requests without FDs are batched into the next request that does
//    have FDs.
//  - The unbatching code has a separate branch for when the last
//    request in a batch does not carry FDs. Exercise it.
TEST_P(FdWriteBatchingTest, BatchWithoutFDs) {
  serverSends_.emplace_back(folly::SocketFds::ToSend{}, "ReqOne", "");
  serverSends_.emplace_back(folly::SocketFds::ToSend{}, "ReqTwo", "");
  serverSends_.emplace_back(
      folly::SocketFds::ToSend{newFd()},
      "ReqThree",
      "ReqOne.+ReqTwo.+ReqThree");
  serverSends_.emplace_back(folly::SocketFds::ToSend{}, "ReqFour", "");
  serverSends_.emplace_back(
      folly::SocketFds::ToSend{}, "ReqFive", "ReqFour.+ReqFive");
  batchingSize_ = serverSends_.size() + 1; // +1 for SetupFrame
  init();

  for (int i = 1; i <= 4; ++i) {
    EXPECT_FALSE(issueRequestAndAwaitFulfillment(kShortSendTimeout));
    expectNumRequestsInBatch(i);
  }

  EXPECT_TRUE(issueRequestAndAwaitFulfillment(kShortSendTimeout));
  expectAllRequestsCompleted(2); // Only had 2 socket writes
}

INSTANTIATE_TEST_SUITE_P(
    BothInterceptedAndNot, FdWriteBatchingTest, testing::Values(false, true));
