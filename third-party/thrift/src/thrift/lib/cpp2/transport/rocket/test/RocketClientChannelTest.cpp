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

#include <algorithm>
#include <deque>
#include <memory>
#include <string>

#include <folly/portability/GTest.h>

#include <folly/Conv.h>
#include <folly/Try.h>
#include <folly/fibers/Baton.h>
#include <folly/fibers/Fiber.h>
#include <folly/fibers/FiberManager.h>
#include <folly/fibers/FiberManagerMap.h>
#include <folly/futures/Future.h>
#include <folly/io/Cursor.h>
#include <folly/io/IOBufQueue.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/EventBase.h>

#include <thrift/lib/cpp2/Flags.h>
#include <thrift/lib/cpp2/async/ClientBufferedStream.h>
#include <thrift/lib/cpp2/async/RocketClientChannel.h>
#include <thrift/lib/cpp2/async/ServerStream.h>
#include <thrift/lib/cpp2/test/gen-cpp2/TestService.h>
#include <thrift/lib/cpp2/transport/core/RpcMetadataPlugins.h>
#include <thrift/lib/cpp2/transport/core/RpcMetadataUtil.h>
#include <thrift/lib/cpp2/transport/rocket/test/util/TestUtil.h>
#include <thrift/lib/cpp2/util/ScopedServerInterfaceThread.h>

THRIFT_FLAG_DECLARE_bool(rocket_client_rocket_skip_protocol_key);

using namespace apache::thrift;

namespace apache::thrift::detail {

std::unique_ptr<folly::IOBuf> makeFrameworkMetadataHook(
    const RpcOptions&,
    folly::dynamic&,
    folly::F14NodeMap<std::string, std::string>&) {
  return folly::IOBuf::copyBuffer("some_content");
}

THRIFT_PLUGGABLE_FUNC_SET(
    std::unique_ptr<folly::IOBuf>,
    makeFrameworkMetadata,
    const RpcOptions& rpcOptions,
    folly::dynamic& logMessages,
    folly::F14NodeMap<std::string, std::string>& headers) {
  return makeFrameworkMetadataHook(rpcOptions, logMessages, headers);
}
} // namespace apache::thrift::detail

namespace {
class Handler : public apache::thrift::ServiceHandler<test::TestService> {
 public:
  folly::SemiFuture<std::unique_ptr<std::string>> semifuture_sendResponse(
      int64_t size) final {
    lastTimeoutMsec_ =
        getConnectionContext()->getHeader()->getClientTimeout().count();
    frameworkMetadata_ = getRequestContext()->getFrameworkMetadata();

    return folly::makeSemiFuture()
        .delayed(std::chrono::milliseconds(sleepDelayMsec_))
        .defer([size](auto&&) {
          return std::make_unique<std::string>(folly::to<std::string>(size));
        });
  }

  folly::SemiFuture<folly::Unit> semifuture_noResponse(int64_t) final {
    return folly::makeSemiFuture();
  }

  folly::SemiFuture<std::unique_ptr<test::IOBufPtr>> semifuture_echoIOBuf(
      std::unique_ptr<folly::IOBuf> iobuf) final {
    return folly::makeSemiFuture(
        std::make_unique<test::IOBufPtr>(std::move(iobuf)));
  }

  folly::SemiFuture<folly::Unit> semifuture_noResponseIOBuf(
      std::unique_ptr<folly::IOBuf>) final {
    return folly::makeSemiFuture();
  }

  ServerStream<int8_t> echoIOBufAsByteStream(
      std::unique_ptr<folly::IOBuf> iobuf, int32_t delayMs) final {
    auto [stream, publisher] = ServerStream<int8_t>::createPublisher();
    std::ignore = folly::makeSemiFuture()
                      .delayed(std::chrono::milliseconds(delayMs))
                      .via(getThreadManager())
                      .thenValue([publisher = std::move(publisher),
                                  iobuf = std::move(iobuf)](auto&&) mutable {
                        folly::io::Cursor cursor(iobuf.get());
                        int8_t byte;
                        while (cursor.tryRead(byte)) {
                          publisher.next(byte);
                        }
                        std::move(publisher).complete();
                      });
    return std::move(stream);
  }

  int32_t getLastTimeoutMsec() const { return lastTimeoutMsec_; }
  void setSleepDelayMs(int32_t delay) { sleepDelayMsec_ = delay; }

  folly::Optional<folly::IOBuf>& getFrameworkMetadata() {
    return frameworkMetadata_;
  }

 private:
  int32_t lastTimeoutMsec_{-1};
  int32_t sleepDelayMsec_{0};
  folly::Optional<folly::IOBuf> frameworkMetadata_;
};

class RocketClientChannelTest : public testing::Test {
 public:
  template <typename F>
  test::TestServiceAsyncClient makeClient(
      folly::EventBase& evb, F&& configureChannel) {
    auto channel =
        RocketClientChannel::newChannel(folly::AsyncSocket::UniquePtr(
            new folly::AsyncSocket(&evb, runner_.getAddress())));
    configureChannel(*channel);
    return test::TestServiceAsyncClient(std::move(channel));
  }

  test::TestServiceAsyncClient makeClient(folly::EventBase& evb) {
    return makeClient(evb, [](auto&) {});
  }

 protected:
  std::shared_ptr<Handler> handler_{std::make_shared<Handler>()};
  ScopedServerInterfaceThread runner_{handler_};
};
} // namespace

TEST_F(RocketClientChannelTest, SyncThriftFrameworkMetadataPropagated) {
  folly::EventBase evb;
  auto client = makeClient(evb);

  RpcOptions opts;
  std::string response;
  client.sync_sendResponse(opts, response, 123);
  EXPECT_EQ("123", response);

  auto frameworkMetadata = handler_->getFrameworkMetadata();
  EXPECT_TRUE(frameworkMetadata);
}

TEST_F(RocketClientChannelTest, RocketSkipProtocolKey) {
  THRIFT_FLAG_SET_MOCK(rocket_client_rocket_skip_protocol_key, true);
  folly::EventBase evb;
  auto client = makeClient(evb);

  std::string response;
  client.sync_sendResponse(response, 123);
  EXPECT_EQ("123", response);
}

TEST_F(RocketClientChannelTest, SyncThread) {
  folly::EventBase evb;
  auto client = makeClient(evb);

  std::string response;
  client.sync_sendResponse(response, 123);
  EXPECT_EQ("123", response);
}

TEST_F(RocketClientChannelTest, SyncFiber) {
  folly::EventBase evb;
  auto& fm = folly::fibers::getFiberManager(evb);
  auto client = makeClient(evb);

  size_t responses = 0;
  fm.addTaskFinally(
      [&client] {
        std::string response;
        client.sync_sendResponse(response, 123);
        return response;
      },
      [&responses](folly::Try<std::string>&& tryResponse) {
        EXPECT_TRUE(tryResponse.hasValue());
        EXPECT_EQ("123", *tryResponse);
        ++responses;
      });
  while (fm.hasTasks()) {
    evb.loopOnce();
  }
  EXPECT_EQ(1, responses);
}

TEST_F(RocketClientChannelTest, SyncThreadOneWay) {
  folly::EventBase evb;
  auto client = makeClient(evb);
  client.sync_noResponse(123);
}

TEST_F(RocketClientChannelTest, SyncFiberOneWay) {
  folly::EventBase evb;
  auto& fm = folly::fibers::getFiberManager(evb);
  auto client = makeClient(evb);

  size_t sent = 0;
  fm.addTaskFinally(
      [&client] { client.sync_noResponse(123); },
      [&sent](folly::Try<void>&& tryResponse) {
        EXPECT_TRUE(tryResponse.hasValue());
        ++sent;
      });
  while (fm.hasTasks()) {
    evb.loopOnce();
  }
  EXPECT_EQ(1, sent);
}

TEST_F(RocketClientChannelTest, SyncThreadCheckTimeoutPropagated) {
  folly::EventBase evb;
  auto client = makeClient(evb);

  RpcOptions opts;
  std::string response;
  // Ensure that normally, the timeout value gets propagated.
  opts.setTimeout(std::chrono::milliseconds(500));
  client.sync_sendResponse(opts, response, 123);
  EXPECT_EQ("123", response);
  EXPECT_EQ(500, handler_->getLastTimeoutMsec());
  // And when we set client-only, it's not propagated.
  opts.setClientOnlyTimeouts(true);
  client.sync_sendResponse(opts, response, 456);
  EXPECT_EQ("456", response);
  EXPECT_EQ(0, handler_->getLastTimeoutMsec());

  // Double-check that client enforces the timeouts in both cases.
  handler_->setSleepDelayMs(600);
  ASSERT_ANY_THROW(client.sync_sendResponse(opts, response, 456));
  opts.setClientOnlyTimeouts(false);
  ASSERT_ANY_THROW(client.sync_sendResponse(opts, response, 456));

  // Ensure that a 0 timeout is actually infinite
  auto infiniteTimeoutClient =
      makeClient(evb, [](auto& channel) { channel.setTimeout(0); });
  opts.setTimeout(std::chrono::milliseconds::zero());
  handler_->setSleepDelayMs(300);
  infiniteTimeoutClient.sync_sendResponse(opts, response, 456);
  EXPECT_EQ("456", response);
}

TEST_F(RocketClientChannelTest, ThriftClientLifetime) {
  folly::EventBase evb;
  folly::Optional<test::TestServiceAsyncClient> client = makeClient(evb);

  auto& fm = folly::fibers::getFiberManager(evb);
  auto future = fm.addTaskFuture([&] {
    std::string response;
    client->sync_sendResponse(response, 123);
    EXPECT_EQ("123", response);
  });

  // Trigger request sending.
  evb.loopOnce();

  // Reset the client.
  client.reset();

  // Wait for the response.
  std::move(future).getVia(&evb);
}

TEST_F(RocketClientChannelTest, LargeRequestResponse) {
  // send and receive large IOBufs to test rocket parser correctness in handling
  // large (larger than kMaxBufferSize) payloads
  folly::EventBase evb;
  auto client = makeClient(evb);

  auto orig = std::string(1024 * 1024, 'x');
  auto iobuf = folly::IOBuf::copyBuffer(orig);

  test::IOBufPtr response;
  client.sync_echoIOBuf(
      RpcOptions().setTimeout(std::chrono::seconds(30)), response, *iobuf);
  EXPECT_EQ(
      response->computeChainDataLength(), iobuf->computeChainDataLength());
  auto res = response->moveToFbString();
  EXPECT_EQ(orig, res);
}

namespace {

folly::SemiFuture<std::unique_ptr<folly::IOBuf>> echoSync(
    test::TestServiceAsyncClient& client,
    size_t nbytes,
    std::optional<std::chrono::milliseconds> timeout = std::nullopt) {
  auto& fm =
      folly::fibers::getFiberManager(*client.getChannel()->getEventBase());
  return fm.addTaskFuture([&, nbytes, timeout] {
    auto iobuf = folly::IOBuf::copyBuffer(std::string(nbytes, 'x'));
    test::IOBufPtr response;
    client.sync_echoIOBuf(
        RpcOptions().setTimeout(timeout.value_or(std::chrono::seconds(30))),
        response,
        *iobuf);
    return response;
  });
}

folly::SemiFuture<std::unique_ptr<folly::IOBuf>> echoSemiFuture(
    test::TestServiceAsyncClient& client,
    size_t nbytes,
    std::optional<std::chrono::milliseconds> timeout = std::nullopt) {
  return folly::makeSemiFutureWith([&] {
    auto iobuf = folly::IOBuf::copyBuffer(std::string(nbytes, 'x'));
    auto options =
        RpcOptions().setTimeout(timeout.value_or(std::chrono::seconds(30)));
    return client.semifuture_echoIOBuf(options, *iobuf);
  });
}

folly::SemiFuture<folly::Unit> noResponseIOBufSync(
    test::TestServiceAsyncClient& client, size_t nbytes) {
  auto& fm =
      folly::fibers::getFiberManager(*client.getChannel()->getEventBase());
  return fm.addTaskFuture([&, nbytes] {
    auto iobuf = folly::IOBuf::copyBuffer(std::string(nbytes, 'x'));
    client.sync_noResponseIOBuf(
        RpcOptions().setTimeout(std::chrono::seconds(30)), *iobuf);
  });
}

folly::SemiFuture<folly::Unit> noResponseIOBufSemiFuture(
    test::TestServiceAsyncClient& client, size_t nbytes) {
  return folly::makeSemiFutureWith([&] {
    auto iobuf = folly::IOBuf::copyBuffer(std::string(nbytes, 'x'));
    auto options = RpcOptions().setTimeout(std::chrono::seconds(30));
    client.semifuture_noResponseIOBuf(options, *iobuf);
  });
}

folly::SemiFuture<ClientBufferedStream<int8_t>> echoIOBufAsByteStreamSync(
    test::TestServiceAsyncClient& client, size_t nbytes) {
  auto& fm =
      folly::fibers::getFiberManager(*client.getChannel()->getEventBase());
  return fm.addTaskFuture([&, nbytes] {
    auto iobuf = folly::IOBuf::copyBuffer(std::string(nbytes, 'x'));
    return client.sync_echoIOBufAsByteStream(
        RpcOptions().setTimeout(std::chrono::seconds(30)),
        *iobuf,
        0 /* delayMs */);
  });
}

folly::SemiFuture<ClientBufferedStream<int8_t>> echoIOBufAsByteStreamSemiFuture(
    test::TestServiceAsyncClient& client, size_t nbytes) {
  return folly::makeSemiFutureWith([&] {
    auto iobuf = folly::IOBuf::copyBuffer(std::string(nbytes, 'x'));
    auto options = RpcOptions().setTimeout(std::chrono::seconds(30));
    return client.semifuture_echoIOBufAsByteStream(
        options, *iobuf, 0 /* delayMs */);
  });
}
} // namespace

TEST_F(RocketClientChannelTest, BatchedWriteFastFirstResponseFiberSync) {
  folly::EventBase evb;
  auto* slowWritingSocket = new SlowWritingSocket(&evb, runner_.getAddress());
  test::TestServiceAsyncClient client(RocketClientChannel::newChannel(
      folly::AsyncSocket::UniquePtr(slowWritingSocket)));

  // Allow first requests to be written completely to the socket quickly, but
  // hold off on sending the complete second request.
  slowWritingSocket->delayWritingAfterFirstNBytes(2000);

  std::vector<folly::SemiFuture<folly::Unit>> futures;
  auto sf =
      folly::makeSemiFuture()
          .delayed(std::chrono::seconds(2))
          .via(&evb)
          .thenValue([&](auto&&) { slowWritingSocket->flushBufferedWrites(); });
  futures.push_back(std::move(sf));

  for (size_t i = 0; i < 5; ++i) {
    sf = echoSync(client, 25).via(&evb).thenTry([](auto&& response) {
      EXPECT_TRUE(response.hasValue());
      EXPECT_EQ(25, response.value()->computeChainDataLength());
    });
    futures.push_back(std::move(sf));

    sf = noResponseIOBufSync(client, 25).via(&evb).thenTry([](auto&& response) {
      EXPECT_TRUE(response.hasValue());
    });
    futures.push_back(std::move(sf));

    sf = echoIOBufAsByteStreamSync(client, 25)
             .via(&evb)
             .thenTry([&](auto&& stream) {
               EXPECT_TRUE(stream.hasValue());
               return std::move(*stream)
                   .subscribeExTry(
                       &evb,
                       [](auto&& next) {
                         EXPECT_FALSE(next.hasException())
                             << next.exception().what();
                       })
                   .futureJoin();
             });
    futures.push_back(std::move(sf));
  }

  sf = echoSync(client, 2000).via(&evb).thenTry([](auto&& response) {
    EXPECT_TRUE(response.hasValue());
    EXPECT_EQ(2000, response.value()->computeChainDataLength());
  });
  futures.push_back(std::move(sf));

  folly::collectAllUnsafe(std::move(futures)).getVia(&evb);
}

TEST_F(RocketClientChannelTest, BatchedWriteFastFirstResponseSemiFuture) {
  folly::EventBase evb;
  auto* slowWritingSocket = new SlowWritingSocket(&evb, runner_.getAddress());
  test::TestServiceAsyncClient client(RocketClientChannel::newChannel(
      folly::AsyncSocket::UniquePtr(slowWritingSocket)));

  // Allow first requests to be written completely to the socket quickly, but
  // hold off on sending the complete second request.
  slowWritingSocket->delayWritingAfterFirstNBytes(2000);

  std::vector<folly::SemiFuture<folly::Unit>> futures;
  auto sf =
      folly::makeSemiFuture()
          .delayed(std::chrono::seconds(2))
          .via(&evb)
          .thenValue([&](auto&&) { slowWritingSocket->flushBufferedWrites(); });
  futures.push_back(std::move(sf));

  for (size_t i = 0; i < 5; ++i) {
    sf = echoSemiFuture(client, 25).via(&evb).thenTry([&](auto&& response) {
      EXPECT_TRUE(response.hasValue());
      EXPECT_EQ(25, response.value()->computeChainDataLength());
    });
    futures.push_back(std::move(sf));

    sf = noResponseIOBufSemiFuture(client, 25)
             .via(&evb)
             .thenTry(
                 [&](auto&& response) { EXPECT_TRUE(response.hasValue()); });
    futures.push_back(std::move(sf));

    sf = echoIOBufAsByteStreamSemiFuture(client, 25)
             .via(&evb)
             .thenTry([&](auto&& stream) {
               EXPECT_TRUE(stream.hasValue());
               return std::move(*stream)
                   .subscribeExTry(
                       &evb,
                       [](auto&& next) {
                         EXPECT_FALSE(next.hasException())
                             << next.exception().what();
                       })
                   .futureJoin();
             });
    futures.push_back(std::move(sf));
  }

  sf = echoSemiFuture(client, 2000).via(&evb).thenTry([&](auto&& response) {
    EXPECT_TRUE(response.hasValue());
    EXPECT_EQ(2000, response.value()->computeChainDataLength());
  });
  futures.push_back(std::move(sf));

  folly::collectAllUnsafe(std::move(futures)).getVia(&evb);
}

namespace {
void doFailLastRequestsInBatchFiber(
    const folly::SocketAddress& serverAddr,
    folly::Optional<size_t> failLastRequestWithNBytesWritten = folly::none) {
  folly::EventBase evb;
  auto* slowWritingSocket = new SlowWritingSocket(&evb, serverAddr);
  test::TestServiceAsyncClient client(RocketClientChannel::newChannel(
      folly::AsyncSocket::UniquePtr(slowWritingSocket)));

  // Allow first requests to be written completely to the socket quickly, but
  // hold off on sending the complete second request.
  slowWritingSocket->delayWritingAfterFirstNBytes(2000);

  std::vector<folly::SemiFuture<folly::Unit>> futures;
  auto sf = folly::makeSemiFuture()
                .delayed(std::chrono::seconds(2))
                .via(&evb)
                .thenValue([&](auto&&) {
                  slowWritingSocket->errorOutBufferedWrites(
                      failLastRequestWithNBytesWritten);
                });
  futures.push_back(std::move(sf));

  for (size_t i = 0; i < 5; ++i) {
    sf = echoSync(client, 25).via(&evb).thenTry([](auto&& response) {
      EXPECT_TRUE(response.hasValue());
      EXPECT_EQ(25, response.value()->computeChainDataLength());
    });
    futures.push_back(std::move(sf));

    sf = noResponseIOBufSync(client, 25).via(&evb).thenTry([](auto&& response) {
      EXPECT_FALSE(response.hasValue());
    });
    futures.push_back(std::move(sf));

    sf = echoIOBufAsByteStreamSync(client, 25)
             .via(&evb)
             .thenTry([&](auto&& stream) {
               EXPECT_TRUE(stream.hasValue());
               return std::move(*stream)
                   .subscribeExTry(
                       &evb,
                       [](auto&& next) {
                         EXPECT_FALSE(next.hasException())
                             << next.exception().what();
                       })
                   .futureJoin();
             });
    futures.push_back(std::move(sf));
  }

  for (size_t i = 0; i < 5; ++i) {
    sf = echoSync(client, 2000).via(&evb).thenTry([](auto&& response) {
      EXPECT_TRUE(response.hasException());
      EXPECT_TRUE(
          response.exception()
              .template is_compatible_with<transport::TTransportException>());
    });
    futures.push_back(std::move(sf));

    sf = echoIOBufAsByteStreamSync(client, 2000)
             .via(&evb)
             .thenTry(
                 [&](auto&& stream) { EXPECT_TRUE(stream.hasException()); });
    futures.push_back(std::move(sf));
  }

  folly::collectAllUnsafe(std::move(futures)).getVia(&evb);
}

void doFailLastRequestsInBatchSemiFuture(
    const folly::SocketAddress& serverAddr,
    folly::Optional<size_t> failLastRequestWithNBytesWritten = folly::none) {
  folly::EventBase evb;
  auto* slowWritingSocket = new SlowWritingSocket(&evb, serverAddr);
  test::TestServiceAsyncClient client(RocketClientChannel::newChannel(
      folly::AsyncSocket::UniquePtr(slowWritingSocket)));

  // Allow first requests to be written completely to the socket quickly, but
  // hold off on sending the complete second request.
  slowWritingSocket->delayWritingAfterFirstNBytes(2000);

  std::vector<folly::SemiFuture<folly::Unit>> futures;
  auto sf = folly::makeSemiFuture()
                .delayed(std::chrono::seconds(2))
                .via(&evb)
                .thenValue([&](auto&&) {
                  slowWritingSocket->errorOutBufferedWrites(
                      failLastRequestWithNBytesWritten);
                });
  futures.push_back(std::move(sf));

  for (size_t i = 0; i < 5; ++i) {
    sf = echoSemiFuture(client, 25).via(&evb).thenTry([&](auto&& response) {
      EXPECT_TRUE(response.hasValue());
      EXPECT_EQ(25, response.value()->computeChainDataLength());
    });
    futures.push_back(std::move(sf));

    sf = noResponseIOBufSemiFuture(client, 25)
             .via(&evb)
             .thenTry(
                 [&](auto&& response) { EXPECT_TRUE(response.hasValue()); });
    futures.push_back(std::move(sf));

    sf = echoIOBufAsByteStreamSemiFuture(client, 25)
             .via(&evb)
             .thenTry([&](auto&& stream) {
               EXPECT_TRUE(stream.hasValue());
               return std::move(*stream)
                   .subscribeExTry(
                       &evb,
                       [](auto&& next) {
                         EXPECT_FALSE(next.hasException())
                             << next.exception().what();
                       })
                   .futureJoin();
             });
    futures.push_back(std::move(sf));
  }

  for (size_t i = 0; i < 5; ++i) {
    sf = echoSemiFuture(client, 2000).via(&evb).thenTry([&](auto&& response) {
      EXPECT_TRUE(response.hasException());
      EXPECT_TRUE(
          response.exception()
              .template is_compatible_with<transport::TTransportException>());
    });
    futures.push_back(std::move(sf));

    sf = echoIOBufAsByteStreamSemiFuture(client, 2000)
             .via(&evb)
             .thenTry(
                 [&](auto&& stream) { EXPECT_TRUE(stream.hasException()); });
    futures.push_back(std::move(sf));
  }

  folly::collectAllUnsafe(std::move(futures)).getVia(&evb);
}
} // namespace

TEST_F(RocketClientChannelTest, FailLastRequestInBatchFiberSync) {
  doFailLastRequestsInBatchFiber(runner_.getAddress());
}

TEST_F(RocketClientChannelTest, FailLastRequestWithZeroBytesWrittenFiberSync) {
  doFailLastRequestsInBatchFiber(
      runner_.getAddress(), folly::Optional<size_t>(0));
}

TEST_F(RocketClientChannelTest, FailLastRequestInBatchSemiFuture) {
  doFailLastRequestsInBatchSemiFuture(runner_.getAddress());
}

TEST_F(RocketClientChannelTest, FailLastRequestWithZeroBytesWrittenSemiFuture) {
  doFailLastRequestsInBatchSemiFuture(
      runner_.getAddress(), folly::Optional<size_t>(0));
}

TEST_F(
    RocketClientChannelTest, BatchedWriteRequestResponseWithFastClientTimeout) {
  folly::EventBase evb;
  auto* slowWritingSocket = new SlowWritingSocket(&evb, runner_.getAddress());
  test::TestServiceAsyncClient client(RocketClientChannel::newChannel(
      folly::AsyncSocket::UniquePtr(slowWritingSocket)));

  // Hold off on writing any requests. This ensures that this test exercises the
  // path where a client request timeout fires while the request is still in the
  // WRITE_SENDING queue.
  slowWritingSocket->delayWritingAfterFirstNBytes(1);

  std::vector<folly::SemiFuture<folly::Unit>> futures;
  const std::chrono::seconds flushDelay(2);
  auto sf =
      folly::makeSemiFuture()
          .delayed(flushDelay)
          .via(&evb)
          .thenValue([&](auto&&) { slowWritingSocket->flushBufferedWrites(); });
  futures.push_back(std::move(sf));

  auto checkResponse = [](const auto& response, size_t expectedResponseSize) {
    if (expectedResponseSize == 0) {
      EXPECT_TRUE(response.hasException());
      EXPECT_TRUE(
          response.exception()
              .template is_compatible_with<transport::TTransportException>());
      response.exception()
          .template with_exception<transport::TTransportException>(
              [](const auto& tex) {
                EXPECT_EQ(
                    transport::TTransportException::TTransportExceptionType::
                        TIMED_OUT,
                    tex.getType());
              });
    } else {
      EXPECT_TRUE(response.hasValue());
      EXPECT_EQ(
          expectedResponseSize, response.value()->computeChainDataLength());
    }
  };

  // Over several event loops, force some timeouts to fire before any socket
  // writes complete at varying positions within each batch of requests.
  std::vector<uint32_t> timeouts = {50, 50, 10000, 10000, 10000, 10000};
  for (size_t requestSize = 20, loops = 0; loops < 20; ++loops) {
    for (uint32_t timeoutMs : timeouts) {
      const std::chrono::milliseconds timeout(timeoutMs);

      sf = echoSync(client, requestSize, timeout)
               .via(&evb)
               .thenTry([&checkResponse,
                         responseSize = timeout < flushDelay ? 0 : requestSize](
                            auto&& response) {
                 checkResponse(response, responseSize);
               });
      futures.push_back(std::move(sf));

      sf = echoSemiFuture(client, requestSize, timeout)
               .via(&evb)
               .thenTry([&checkResponse,
                         responseSize = timeout < flushDelay ? 0 : requestSize](
                            auto&& response) {
                 checkResponse(response, responseSize);
               });
      futures.push_back(std::move(sf));

      ++requestSize;
    }

    // Start writing the current batch of requests and ensure a new batch is
    // started next iteration
    evb.loopOnce();
    evb.loopOnce();

    std::rotate(timeouts.begin(), timeouts.begin() + 1, timeouts.end());
  }

  folly::collectAllUnsafe(std::move(futures)).getVia(&evb);
}

TEST_F(RocketClientChannelTest, StreamInitialResponseBeforeBatchedWriteFails) {
  folly::EventBase evb;
  auto* slowWritingSocket = new SlowWritingSocket(&evb, runner_.getAddress());
  test::TestServiceAsyncClient client(RocketClientChannel::newChannel(
      folly::AsyncSocket::UniquePtr(slowWritingSocket)));

  // Ensure the first request is written completely to the socket quickly, but
  // force the write for the whole batch of requests to fail.
  slowWritingSocket->delayWritingAfterFirstNBytes(1000);

  std::vector<folly::SemiFuture<folly::Unit>> futures;
  auto sf = folly::makeSemiFuture()
                .delayed(std::chrono::seconds(1))
                .via(&evb)
                .thenValue([&](auto&&) {
                  slowWritingSocket->errorOutBufferedWrites(
                      folly::Optional<size_t>(0));
                });
  futures.push_back(std::move(sf));

  // Keep the stream alive on both client and server until the end of the test
  std::optional<ClientBufferedStream<signed char>::Subscription> subscription;
  sf = folly::makeSemiFutureWith([&] {
         auto iobuf = folly::IOBuf::copyBuffer(std::string(25, 'x'));
         auto options = RpcOptions().setTimeout(std::chrono::seconds(30));
         return client.semifuture_echoIOBufAsByteStream(
             options, *iobuf, 2000 /* delayMs */);
       })
           .via(&evb)
           .thenTry([&](auto&& stream) {
             subscription.emplace(
                 std::move(*stream).subscribeExTry(&evb, [](auto&&) {}));
           });
  futures.push_back(std::move(sf));

  // Include more requests in the write batch
  for (size_t i = 0; i < 10; ++i) {
    sf = echoSemiFuture(client, 1000).via(&evb).thenTry([&](auto&& response) {
      EXPECT_TRUE(response.hasException());
      EXPECT_TRUE(
          response.exception()
              .template is_compatible_with<transport::TTransportException>());
    });
    futures.push_back(std::move(sf));
  }

  folly::collectAllUnsafe(std::move(futures)).getVia(&evb);
  subscription->cancel();
  std::move(*subscription).join();
}
