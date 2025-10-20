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

#include <folly/coro/GtestHelpers.h>
#include <folly/coro/Result.h>
#include <folly/coro/ValueOrError.h>
#include <folly/coro/safe/NowTask.h>

/// Besides `value_or_error_or_stopped`, this test also covers the
/// `folly::result<T>` integration with `coro::co_result` from `coro/Result.h`.

#if FOLLY_HAS_RESULT

namespace folly::coro {

CO_TEST(ValueOrErrorTest, value_or_error_or_stopped_of_error) {
  auto voidErrorTask = []() -> now_task<void> {
    co_yield co_error(std::runtime_error("foo"));
  };
  { // Capture the error
    auto res = co_await value_or_error_or_stopped(voidErrorTask());
    EXPECT_STREQ("foo", get_exception<std::runtime_error>(res)->what());
  }
  { // Also test `coro::co_result` integration
    auto res = co_await value_or_error_or_stopped([&]() -> now_task<void> {
      co_yield co_result(co_await value_or_error_or_stopped(voidErrorTask()));
    }());
    EXPECT_STREQ("foo", get_exception<std::runtime_error>(res)->what());
  }
}

CO_TEST(ValueOrErrorTest, value_or_error_or_stopped_of_value) {
  // Return a move-only thing to make sure we don't copy
  auto valueTask = []() -> now_task<std::unique_ptr<int>> {
    co_return std::make_unique<int>(1337);
  };
  { // Capture the value
    auto res = co_await value_or_error_or_stopped(valueTask());
    // Real code should use `co_await co_ready(res)` to unpack!
    EXPECT_EQ(1337, *res.value_or_throw());
  }
  { // Also test `coro::co_result` integration
    auto res = co_await value_or_error_or_stopped(
        [&]() -> now_task<std::unique_ptr<int>> {
          co_yield co_result(co_await value_or_error_or_stopped(valueTask()));
        }());
    EXPECT_EQ(1337, *res.value_or_throw());
  }
}

CO_TEST(ValueOrErrorTest, value_or_error_or_stopped_of_void) {
  int numAwaited = 0;
  auto voidTask = [&]() -> now_task<void> {
    ++numAwaited;
    co_return;
  };
  { // Capturing a "value" completion
    auto res = co_await value_or_error_or_stopped(voidTask());
    static_assert(std::is_same_v<decltype(res), result<>>);
    EXPECT_TRUE(res.has_value());
    EXPECT_EQ(1, numAwaited);
  }
  { // Also test `coro::co_result` integration
    auto res = co_await value_or_error_or_stopped([&]() -> now_task<void> {
      co_yield co_result(co_await value_or_error_or_stopped(voidTask()));
    }());
    static_assert(std::is_same_v<decltype(res), result<>>);
    EXPECT_TRUE(res.has_value());
    EXPECT_EQ(2, numAwaited);
  }
}

CO_TEST(ValueOrErrorTest, value_or_error_or_stopped_stopped) {
  auto stoppedTask = [&]() -> now_task<void> {
    co_yield co_cancelled;
    LOG(FATAL) << "not reached";
  };
  { // Capturing a "stopped" completion
    auto res = co_await value_or_error_or_stopped(stoppedTask());
    EXPECT_TRUE(res.has_stopped());
  }
  { // Also test `coro::co_result` integration
    auto res = co_await value_or_error_or_stopped([&]() -> now_task<void> {
      co_yield co_result(co_await value_or_error_or_stopped(stoppedTask()));
      LOG(FATAL) << "not reached";
    }());
    EXPECT_TRUE(res.has_stopped());
  }
}

TEST(ValueOrErrorTest, co_nothrow_value_or_error_or_stopped) {
  bool ran = []<typename T>(T) { // `requires` does SFINAE inside templates
    auto errorTask = [&]() -> now_task<> {
      co_yield co_error(std::runtime_error("foo"));
    };
    (void)co_nothrow(errorTask()); // compile
    (void)value_or_error_or_stopped(errorTask()); // compiles
    static_assert(!requires {
      co_nothrow(value_or_error_or_stopped(errorTask()));
    });
#if 0 // manual test equivalent of above `static_assert`
    (void)co_nothrow(value_or_error_or_stopped(errorTask())); // constraint failure
#endif
    return true;
  }(5);
  EXPECT_TRUE(ran); // it's easy to forget to call the lambda
}

struct ThrowingMove {
  ThrowingMove(const ThrowingMove&) = default;
  ThrowingMove& operator=(const ThrowingMove&) = default;
  ThrowingMove(ThrowingMove&&) {}
  ThrowingMove& operator=(ThrowingMove&&) { return *this; }
};
struct NothrowMove {};

template <typename T>
using TestAwaiter = detail::ResultAwaiter<TaskWithExecutor<T>>;

// While it's unclear if anything cares about `await_resume()` `noexcept`
// discipline, it's easy enough to check.
static_assert(noexcept(FOLLY_DECLVAL(TestAwaiter<int>).await_resume()));
static_assert(noexcept(FOLLY_DECLVAL(TestAwaiter<NothrowMove>).await_resume()));
static_assert(
    !noexcept(FOLLY_DECLVAL(TestAwaiter<ThrowingMove>).await_resume()));

} // namespace folly::coro

#endif
