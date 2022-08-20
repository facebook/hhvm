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

#include <thrift/lib/cpp2/async/tests/util/TestSinkService.h>

#include <folly/ScopeGuard.h>
#include <folly/experimental/coro/Sleep.h>
#include <folly/portability/GTest.h>

namespace testutil {
namespace testservice {

apache::thrift::SinkConsumer<int32_t, bool> TestSinkService::range(
    int32_t from, int32_t to) {
  return apache::thrift::SinkConsumer<int32_t, bool>{
      [from, to](folly::coro::AsyncGenerator<int32_t&&> gen)
          -> folly::coro::Task<bool> {
        int32_t i = from;
        while (auto item = co_await gen.next()) {
          EXPECT_EQ(i++, *item);
        }
        EXPECT_EQ(i, to + 1);
        co_return true;
      },
      10 /* buffer size */
  };
}

apache::thrift::SinkConsumer<int32_t, bool> TestSinkService::rangeThrow(
    int32_t from, int32_t) {
  return apache::thrift::SinkConsumer<int32_t, bool>{
      [from](folly::coro::AsyncGenerator<int32_t&&> gen)
          -> folly::coro::Task<bool> {
        bool throwed = false;
        try {
          int32_t i = from;
          while (auto item = co_await gen.next()) {
            EXPECT_EQ(i++, *item);
          }
        } catch (const std::exception& ex) {
          throwed = true;
          EXPECT_EQ("std::runtime_error: test", std::string(ex.what()));
        }
        EXPECT_TRUE(throwed);
        co_return true;
      },
      10 /* buffer size */
  };
}

apache::thrift::SinkConsumer<int32_t, bool>
TestSinkService::rangeFinalResponseThrow(int32_t from, int32_t) {
  return apache::thrift::SinkConsumer<int32_t, bool>{
      [from](folly::coro::AsyncGenerator<int32_t&&> gen)
          -> folly::coro::Task<bool> {
        int32_t i = from;
        int counter = 5;
        while (auto item = co_await gen.next()) {
          if (counter-- > 0) {
            break;
          }
          EXPECT_EQ(i++, *item);
        }
        throw std::runtime_error("test");
      },
      10 /* buffer size */
  };
}

apache::thrift::SinkConsumer<int32_t, int32_t>
TestSinkService::rangeEarlyResponse(int32_t from, int32_t, int32_t early) {
  return apache::thrift::SinkConsumer<int32_t, int32_t>{
      [from, early](folly::coro::AsyncGenerator<int32_t&&> gen)
          -> folly::coro::Task<int32_t> {
        int32_t i = from;
        while (auto item = co_await gen.next()) {
          EXPECT_EQ(i++, *item);
          if (i == early) {
            co_return early;
          }
        }
        // shouldn't reach here
        co_return -1;
      },
      10 /* buffer size */
  };
}

apache::thrift::SinkConsumer<int32_t, bool>
TestSinkService::unSubscribedSink() {
  activeSinks_++;
  return apache::thrift::SinkConsumer<int32_t, bool>{
      [g = folly::makeGuard([this]() { activeSinks_--; })](
          folly::coro::AsyncGenerator<int32_t&&> gen) mutable
      -> folly::coro::Task<bool> {
        EXPECT_THROW(
            co_await gen.next(), apache::thrift::TApplicationException);
        co_return true;
      },
      10 /* buffer size */
  };
}

folly::SemiFuture<apache::thrift::SinkConsumer<int32_t, bool>>
TestSinkService::semifuture_unSubscribedSinkSlowReturn() {
  return folly::futures::sleep(std::chrono::seconds(1)).deferValue([=](auto&&) {
    activeSinks_++;
    return apache::thrift::SinkConsumer<int32_t, bool>{
        [g = folly::makeGuard([this]() { activeSinks_--; })](
            folly::coro::AsyncGenerator<int32_t&&> gen) mutable
        -> folly::coro::Task<bool> {
          co_await gen.next();
          co_return true;
        },
        10 /* buffer size */
    };
  });
}

bool TestSinkService::isSinkUnSubscribed() {
  return activeSinks_ == 0;
}

apache::thrift::ResponseAndSinkConsumer<bool, int32_t, bool>
TestSinkService::initialThrow() {
  MyException ex;
  *ex.reason_ref() = "reason";
  throw ex;
}

apache::thrift::SinkConsumer<int32_t, bool>
TestSinkService::rangeChunkTimeout() {
  return apache::thrift::SinkConsumer<int32_t, bool>{
      [](folly::coro::AsyncGenerator<int32_t&&> gen)
          -> folly::coro::Task<bool> {
        EXPECT_THROW(
            co_await [&]() -> folly::coro::Task<void> {
              int32_t i = 0;
              while (auto item = co_await gen.next()) {
                EXPECT_EQ(i++, *item);
              }
            }(),
            apache::thrift::TApplicationException);
        co_return true;
      },
      10 /* buffer size */
  }
      .setChunkTimeout(std::chrono::milliseconds(200));
}

apache::thrift::SinkConsumer<int32_t, bool> TestSinkService::sinkThrow() {
  return apache::thrift::SinkConsumer<int32_t, bool>{
      [](folly::coro::AsyncGenerator<int32_t&&> gen)
          -> folly::coro::Task<bool> {
        bool throwed = false;
        try {
          while (auto item = co_await gen.next()) {
          }
        } catch (const SinkException& ex) {
          throwed = true;
          EXPECT_EQ("test", *ex.reason_ref());
        } catch (const std::exception& ex) {
          LOG(ERROR) << "catched unexpected exception " << ex.what();
        }
        EXPECT_TRUE(throwed);
        co_return true;
      },
      10 /* buffer size */
  };
}

apache::thrift::SinkConsumer<int32_t, bool> TestSinkService::sinkFinalThrow() {
  return apache::thrift::SinkConsumer<int32_t, bool>{
      [](folly::coro::AsyncGenerator<int32_t&&>) -> folly::coro::Task<bool> {
        FinalException ex;
        *ex.reason_ref() = "test";
        throw ex;
      },
      10 /* buffer size */
  };
}

void TestSinkService::purge() {}

apache::thrift::SinkConsumer<IOBuf, int32_t> TestSinkService::alignment(
    std::unique_ptr<std::string> expected) {
  return apache::thrift::SinkConsumer<IOBuf, int32_t>{
      [expected = std::move(expected)](folly::coro::AsyncGenerator<IOBuf&&> gen)
          -> folly::coro::Task<int32_t> {
        int32_t res = 0;
        while (auto buf = co_await gen.next()) {
          EXPECT_EQ(*expected, folly::StringPiece(buf->coalesce()));
          res = reinterpret_cast<std::uintptr_t>(buf->data()) % 4096u;
        }
        co_return res;
      },
      10 /* buffer size */
  };
}

apache::thrift::SinkConsumer<IOBuf, bool> TestSinkService::custom(
    std::unique_ptr<std::string> expected) {
  return apache::thrift::SinkConsumer<IOBuf, bool>{
      [expected = std::move(expected), this](
          folly::coro::AsyncGenerator<IOBuf&&> gen) -> folly::coro::Task<bool> {
        bool res = false;
        while (auto buf = co_await gen.next()) {
          if (buf->countChainElements() == 1) {
            res = findAndRemoveBuf(buf->buffer());
          }

          EXPECT_EQ(*expected, folly::StringPiece(buf->coalesce()));
        }
        co_return res;
      },
      10 /* buffer size */
  };
}

apache::thrift::SinkConsumer<int32_t, bool> TestSinkService::rangeCancelAt(
    int32_t from, int32_t to, int32_t cancelAt) {
  return apache::thrift::SinkConsumer<int32_t, bool>{
      [from, to, cancelAt](folly::coro::AsyncGenerator<int32_t&&> gen)
          -> folly::coro::Task<bool> {
        // create custom CancellationSource
        folly::CancellationSource cancelSource;
        folly::CancellationToken parentCt =
            co_await folly::coro::co_current_cancellation_token;
        folly::CancellationCallback cb{
            std::move(parentCt), [&] { cancelSource.requestCancellation(); }};

        // cancellation will be requested asynchronously
        auto cancelTask = [&cancelSource]() -> folly::coro::Task<void> {
          co_await folly::coro::sleep(std::chrono::milliseconds{200});
          cancelSource.requestCancellation();
        };

        int32_t i = from;
        try {
          while (auto item = co_await folly::coro::co_withCancellation(
                     cancelSource.getToken(), gen.next())) {
            EXPECT_EQ(i++, *item);
            if (i == cancelAt) {
              cancelTask()
                  .scheduleOn(co_await folly::coro::co_current_executor)
                  .start();
            }
          }
          EXPECT_EQ(i, to + 1);
          co_return true;
        } catch (const folly::OperationCancelled&) {
          EXPECT_LE(i, to);
          co_return false;
        }
      },
      10 /* buffer size */
  };
}

apache::thrift::SinkConsumer<int32_t, bool>
TestSinkService::rangeSlowFinalResponse(int32_t from, int32_t to) {
  return apache::thrift::SinkConsumer<int32_t, bool>{
      [from, to](folly::coro::AsyncGenerator<int32_t&&> gen)
          -> folly::coro::Task<bool> {
        int32_t i = from;
        while (auto item = co_await gen.next()) {
          EXPECT_EQ(i++, *item);
        }
        EXPECT_EQ(i, to + 1);
        co_await folly::coro::sleep(std::chrono::seconds{5});
        co_return true;
      },
      10 /* buffer size */
  };
}

void TestSinkService::addBuf(const void* buf) {
  bufs_.withWLock([buf](auto& bufs) { bufs.insert(buf); });
}

bool TestSinkService::findAndRemoveBuf(const void* buf) {
  return bufs_.withWLock([buf](auto& bufs) {
    auto iter = bufs.find(buf);

    if (iter != bufs.end()) {
      bufs.erase(iter);

      return true;
    }

    return false;
  });
}

} // namespace testservice
} // namespace testutil
