/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/coro/filters/RateLimitFilter.h>

#include <folly/coro/Collect.h>
#include <folly/coro/GmockHelpers.h>
#include <folly/coro/GtestHelpers.h>
#include <folly/io/async/ScopedEventBaseThread.h>
#include <proxygen/lib/http/coro/test/Mocks.h>

using namespace testing;
using namespace proxygen;
using namespace proxygen::coro;
using namespace std::literals::chrono_literals;

namespace {
constexpr auto kLargeReadAmount = 1024 * 1024 * 200;
}

namespace proxygen::coro::test {
class RateLimitFilterTest : public testing::Test {
 public:
  void SetUp() override {
    testFilter_ = new RateLimitFilter(&mockSource_);
    testFilter_->setHeapAllocated();
  }

 protected:
  void verifyReadSize(uint64_t readSizeExpected, bool eom = false) {
    auto buf = folly::IOBuf::create(readSizeExpected);
    buf->append(readSizeExpected);
    HTTPBodyEvent body(std::move(buf), eom);
    EXPECT_CALL(mockSource_, readBodyEvent(Ge(readSizeExpected)))
        .Times(1)
        .WillOnce(folly::coro::gmock_helpers::CoReturnByMove(std::move(body)));
  }

  // Helper function to do a read on an eventbase, verify size on the underlying
  // source, and also return the duration it takes to read.
  std::chrono::seconds timedRead(uint64_t readAmount,
                                 uint64_t verifyAmount,
                                 bool eom = false) {
    verifyReadSize(verifyAmount, eom);
    auto currentTime = std::chrono::steady_clock::now();
    folly::ScopedEventBaseThread sEvb;
    auto task = co_withExecutor(sEvb.getEventBase(),
                                testFilter_->readBodyEvent(readAmount));
    folly::coro::blockingWait(std::move(task));
    return std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::steady_clock::now() - currentTime);
  }

  std::chrono::seconds timedRead(uint64_t readAmount) {
    return timedRead(readAmount, readAmount);
  }

  folly::coro::Task<void> errorOutFilter() {
    testFilter_->setLimit(1000);
    EXPECT_CALL(mockSource_, readBodyEvent(1))
        .Times(1)
        .WillOnce(folly::coro::gmock_helpers::CoInvoke(
            [&](uint64_t) -> folly::coro::Task<HTTPBodyEvent> {
              co_yield folly::coro::co_error(HTTPError(
                  HTTPErrorCode::READ_TIMEOUT,
                  "Make sure no memory leak of a heap source filter"));
            }));
    auto error = co_await co_awaitTry(testFilter_->readBodyEvent(1));
    EXPECT_TRUE(error.hasException());
  }

  // This is heap allocated without a smart pointer by design, to verify
  // lifetime in base class takes care of deallocating this filter.
  RateLimitFilter* testFilter_;
  MockHTTPSource mockSource_;
};

CO_TEST_F(RateLimitFilterTest, NoLimit) {
  EXPECT_LT(timedRead(kLargeReadAmount), 1s);
  co_await errorOutFilter();
}

TEST_F(RateLimitFilterTest, FirstReadAlwaysOK) {
  testFilter_->setLimit(1000);
  EXPECT_LT(timedRead(kLargeReadAmount), 1s);
  testFilter_->stopReading();
}

TEST_F(RateLimitFilterTest, SecondReadBelowLimit) {
  testFilter_->setLimit(70000 * 1000 * 8);
  EXPECT_LT(timedRead(1), 1s);
  EXPECT_LT(timedRead(1), 1s);
  testFilter_->stopReading();
}

TEST_F(RateLimitFilterTest, SecondReadBeyondLimit) {
  // Rate: 100 * 1000 per second
  testFilter_->setLimit(100 * 1000 * 8);

  // First read is always fine. Read 200 * 1000 bytes from source
  EXPECT_LT(timedRead(200 * 1000), 1s);

  // Read again, even 1 byte needs to wait 1s
  auto secondsElapsed = timedRead(1);
  EXPECT_GE(secondsElapsed, 1s);
  testFilter_->stopReading();
}

TEST_F(RateLimitFilterTest, WontSleepTooLong) {
  // Replace with a new filter with a low sleep limit.
  delete testFilter_;
  testFilter_ = new RateLimitFilter(&mockSource_, 2s, 4096);
  testFilter_->setHeapAllocated();

  // Rate: 100 * 1000 per second
  testFilter_->setLimit(100 * 1000 * 8);

  // First read is always fine. Read 2100 * 1000 bytes from source
  EXPECT_LT(timedRead(2100 * 1000), 1s);

  // Read again, it sleeps but up to a limit
  EXPECT_LT(timedRead(1), 3s);
  testFilter_->stopReading();
}

TEST_F(RateLimitFilterTest, SetLimitClearCounter) {
  // Rate: 100 * 1000 per second
  testFilter_->setLimit(100 * 1000 * 8);

  // First read is always fine. Read 1100 * 1000 bytes from source
  EXPECT_LT(timedRead(1100 * 1000), 1s);

  // Set the limit again, we will start to count again
  testFilter_->setLimit(100 * 1000 * 8);

  // Then the next read will also go through for free
  EXPECT_LT(timedRead(1100 * 1000), 1s);
  testFilter_->stopReading();
}

CO_TEST_F(RateLimitFilterTest, CancelRead) {
  // Rate: 100 * 1000 per second
  testFilter_->setLimit(100 * 1000 * 8);

  // First read is always fine. Read an amount that'd block later.
  EXPECT_LT(timedRead(500 * 1000), 1s);

  // Read again, we'd sleep. But let's cancel it before the sleep time is up
  folly::CancellationSource cancelSource;
  EXPECT_CALL(mockSource_, stopReading(_)).Times(1);
  co_await folly::coro::collectAll(
      [&]() -> folly::coro::Task<void> {
        auto result = co_await co_awaitTry(folly::coro::co_withCancellation(
            cancelSource.getToken(), testFilter_->readBodyEvent(1)));
        EXPECT_TRUE(result.hasException());
        EXPECT_EQ(HTTPErrorCode::CORO_CANCELLED, getHTTPError(result).code);
      }(),
      [&]() -> folly::coro::Task<void> {
        co_await folly::coro::co_reschedule_on_current_executor;
        cancelSource.requestCancellation();
      }());
}

TEST_F(RateLimitFilterTest, MinReadAmount) {
  testFilter_->setLimit(4096 * 8); // 4096 bytes per second
  // First read goes through, takes 4096 from limit
  EXPECT_LT(timedRead(4096), 1s);
  // Without any delay, another 4096 read will sleep instead of returning a
  // small amount of bytes.
  EXPECT_GE(timedRead(4096), 1s);

  // reset limit:
  testFilter_->setLimit(4096 * 8);
  EXPECT_LT(timedRead(4096), 1s);
  // This time a smaller than chunk size read
  EXPECT_LT(timedRead(400), 2s);

  // reset limit:
  testFilter_->setLimit(4096 * 8);
  EXPECT_LT(timedRead(4096), 1s);
  // This time a greater than chunk size read.
  EXPECT_LT(timedRead(4096 * 2, 4096), 3s);

  testFilter_->stopReading();
}

TEST_F(RateLimitFilterTest, ReadEOM) {
  testFilter_->setLimit(100 * 1000 * 8);
  // Underlying source gives a EOM body event:
  EXPECT_CALL(mockSource_, readBodyEvent(2000))
      .Times(1)
      .WillOnce(folly::coro::gmock_helpers::CoReturnByMove(
          HTTPBodyEvent(nullptr, true)));
  folly::ScopedEventBaseThread sEvb;
  auto task = co_awaitTry(
      co_withExecutor(sEvb.getEventBase(), testFilter_->readBodyEvent(2000)));
  auto body = folly::coro::blockingWait(std::move(task));
  EXPECT_TRUE(body->eom);
  // EOM body takes care of deleting filter. No need to error it out.
}

TEST_F(RateLimitFilterTest, SourceHasLessData) {
  testFilter_->setLimit(100 * 1000 * 8);
  // First read, source has no data returned:
  EXPECT_LT(timedRead(1000, 0), 1s);
  // Then next read won't have any delay:
  EXPECT_LT(timedRead(1000, 900, true), 1s);
  // The EOM read in last event also kills the filter
}

} // namespace proxygen::coro::test
