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

#include <thrift/lib/cpp2/async/ClientBufferedStream.h>

#include <gtest/gtest.h>
#include <folly/coro/Baton.h>
#include <folly/io/async/ScopedEventBaseThread.h>
#include <thrift/lib/cpp2/async/ServerStream.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>

namespace apache::thrift {

using namespace ::testing;

// Serializes i as int and pads by i bytes.
auto encode(folly::Try<int>&& i) -> folly::Try<StreamPayload> {
  if (i.hasValue()) {
    folly::IOBufQueue buf;
    CompactSerializer::serialize(*i, &buf);
    buf.allocate(*i);
    return folly::Try<StreamPayload>({buf.move(), {}});
  } else if (i.hasException()) {
    return folly::Try<StreamPayload>(i.exception());
  } else {
    return folly::Try<StreamPayload>();
  }
}
// Retrieves i and drops the padding.
auto decode(folly::Try<StreamPayload>&& i) -> folly::Try<int> {
  if (i.hasValue()) {
    int out;
    CompactSerializer::deserialize<int>(i.value().payload.get(), out);
    return folly::Try<int>(out);
  } else if (i.hasException()) {
    return folly::Try<int>(i.exception());
  } else {
    return folly::Try<int>();
  }
}

struct ServerCallback : StreamServerCallback {
  bool onStreamRequestN(int32_t credits) override {
    credits_ += credits;
    requested.post();
    return true;
  }
  void onStreamCancel() override { std::terminate(); }
  void resetClientCallback(StreamClientCallback&) override { std::terminate(); }

  folly::coro::Baton requested;
  std::atomic<uint64_t> credits_{0};
};

struct FirstResponseCb : detail::ClientStreamBridge::FirstResponseCallback {
  void onFirstResponse(
      FirstResponsePayload&&,
      detail::ClientStreamBridge::ClientPtr clientStreamBridge) override {
    ptr = std::move(clientStreamBridge);
  }
  void onFirstResponseError(folly::exception_wrapper) override {
    std::terminate();
  }
  detail::ClientStreamBridge::ClientPtr ptr;
};

struct ClientBufferedStreamTest : public Test {
  folly::ScopedEventBaseThread ebt;
  FirstResponseCb firstResponseCb;
  ServerCallback serverCb;
  StreamClientCallback* client;
  void SetUp() override {
    client = detail::ClientStreamBridge::create(&firstResponseCb);
    std::ignore =
        client->onFirstResponse({nullptr, {}}, ebt.getEventBase(), &serverCb);
  }
};

TEST_F(ClientBufferedStreamTest, Inline) {
  ClientBufferedStream<int> stream(
      std::move(firstResponseCb.ptr), decode, {2, 0});
  ebt.getEventBase()->runInEventBaseThreadAndWait([&] {
    for (int i = 1; i <= 10; ++i) {
      std::ignore = client->onStreamNext(*encode(folly::Try(i)));
    }
    client->onStreamComplete();
  });

  int i = 0;
  std::move(stream).subscribeInline([&](auto val) {
    if (val.hasValue()) {
      EXPECT_EQ(*val, ++i);
    }
  });
  EXPECT_EQ(i, 10);
}

TEST_F(ClientBufferedStreamTest, InlineCancel) {
  ClientBufferedStream<int> stream(
      std::move(firstResponseCb.ptr), decode, {2, 0});
  ebt.getEventBase()->runInEventBaseThreadAndWait([&] {
    for (int i = 1; i <= 10; ++i) {
      std::ignore = client->onStreamNext(*encode(folly::Try(i)));
    }
    client->onStreamComplete();
  });

  int i = 0;
  std::move(stream).subscribeInline([&](auto val) {
    if (val.hasValue()) {
      EXPECT_EQ(*val, ++i);
    }
    return i != 6;
  });
  EXPECT_EQ(i, 6);
}

TEST_F(ClientBufferedStreamTest, RefillByCount) {
  ClientBufferedStream<int> stream(
      std::move(firstResponseCb.ptr), decode, {10, 0});

  // Refills when half of buffer is used, i.e. after 5th payload
  auto task = co_withExecutor(
                  ebt.getEventBase(),
                  folly::coro::co_invoke([&]() -> folly::coro::Task<void> {
                    auto gen = std::move(stream).toAsyncGenerator();
                    int i = 0;
                    while (auto val = co_await gen.next()) {
                      EXPECT_EQ(*val, ++i);
                      if (i >= 6) {
                        co_await serverCb.requested;
                      } else {
                        EXPECT_FALSE(serverCb.requested.ready());
                      }
                    }
                  }))
                  .start();
  for (int i = 1; i <= 10; ++i) {
    ebt.getEventBase()->runInEventBaseThreadAndWait(
        [&] { std::ignore = client->onStreamNext(*encode(folly::Try(i))); });
  }
  ebt.getEventBase()->runInEventBaseThreadAndWait(
      [&] { client->onStreamComplete(); });
  std::move(task).get();
}

TEST_F(ClientBufferedStreamTest, RefillByCumulativeSize) {
  ClientBufferedStream<int> stream(
      std::move(firstResponseCb.ptr), decode, {100, 0});

  // Refills after reading 16kB from wire, i.e. after 4th 4kB payload
  auto task = co_withExecutor(
                  ebt.getEventBase(),
                  folly::coro::co_invoke([&]() -> folly::coro::Task<void> {
                    auto gen = std::move(stream).toAsyncGenerator();
                    int i = 0;
                    while (auto val = co_await gen.next()) {
                      if (++i >= 5) {
                        co_await serverCb.requested;
                      } else {
                        EXPECT_FALSE(serverCb.requested.ready());
                      }
                    }
                  }))
                  .start();
  for (int i = 1; i <= 10; ++i) {
    ebt.getEventBase()->runInEventBaseThreadAndWait([&] {
      std::ignore = client->onStreamNext(*encode(folly::Try(1 << 12)));
    });
  }
  ebt.getEventBase()->runInEventBaseThreadAndWait(
      [&] { client->onStreamComplete(); });
  std::move(task).get();
}

TEST_F(ClientBufferedStreamTest, RefillBySizeTarget) {
  ClientBufferedStream<int> stream(
      std::move(firstResponseCb.ptr), decode, {10, 64});

  // Refills when outstanding payload size (9B max * credits) drops below half
  // of 64B target, i.e. after 3 9B payloads left (7/10 consumed)
  auto task = co_withExecutor(
                  ebt.getEventBase(),
                  folly::coro::co_invoke([&]() -> folly::coro::Task<void> {
                    auto gen = std::move(stream).toAsyncGenerator();
                    int i = 0;
                    while (auto val = co_await gen.next()) {
                      if (++i >= 8) {
                        co_await serverCb.requested;
                      } else {
                        EXPECT_FALSE(serverCb.requested.ready());
                      }
                    }
                  }))
                  .start();
  for (int i = 1; i <= 10; ++i) {
    ebt.getEventBase()->runInEventBaseThreadAndWait(
        [&] { std::ignore = client->onStreamNext(*encode(folly::Try(8))); });
  }
  ebt.getEventBase()->runInEventBaseThreadAndWait(
      [&] { client->onStreamComplete(); });
  std::move(task).get();
}

TEST_F(ClientBufferedStreamTest, MaxChunkSize) {
  // Max credits 3
  ClientBufferedStream<int> stream(
      std::move(firstResponseCb.ptr), decode, {0, 64, 3});

  auto task = co_withExecutor(
                  ebt.getEventBase(),
                  folly::coro::co_invoke([&]() -> folly::coro::Task<void> {
                    auto gen = std::move(stream).toAsyncGenerator();
                    while (auto val = co_await gen.next()) {
                      EXPECT_GE(3, serverCb.credits_);
                    }
                  }))
                  .start();

  auto waitForCredits = [&]() {
    co_withExecutor(
        ebt.getEventBase(),
        folly::coro::co_invoke([&]() -> folly::coro::Task<void> {
          LOG(INFO) << "Waiting for credits...";
          co_await serverCb.requested;
          serverCb.requested.reset();
          LOG(INFO) << "Got credits " << serverCb.credits_;
        }))
        .start()
        .get();
  };

  for (int i = 1; i <= 10; ++i) {
    waitForCredits();
    ASSERT_LT(0, serverCb.credits_);
    serverCb.credits_--;
    ebt.getEventBase()->runInEventBaseThreadAndWait(
        [&] { std::ignore = client->onStreamNext(*encode(folly::Try(8))); });
  }
  ebt.getEventBase()->runInEventBaseThreadAndWait(
      [&] { client->onStreamComplete(); });
  std::move(task).get();
}

} // namespace apache::thrift
