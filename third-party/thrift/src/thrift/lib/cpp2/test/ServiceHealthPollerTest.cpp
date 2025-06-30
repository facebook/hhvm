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

#include <atomic>
#include <memory>
#include <mutex>
#include <tuple>
#include <utility>

#include <gtest/gtest.h>
#include <folly/Synchronized.h>
#include <folly/coro/AsyncScope.h>
#include <folly/coro/Baton.h>
#include <folly/coro/CurrentExecutor.h>
#include <folly/coro/GtestHelpers.h>
#include <folly/coro/Invoke.h>
#include <folly/coro/Sleep.h>
#include <folly/coro/Task.h>
#include <folly/executors/GlobalExecutor.h>
#include <folly/observer/SimpleObservable.h>
#include <folly/synchronization/Baton.h>

#include <thrift/lib/cpp2/server/ServiceHealthPoller.h>

using namespace std::literals;

using apache::thrift::PolledServiceHealth;
using apache::thrift::ServiceHealthPoller;
using ServiceHealth = PolledServiceHealth::ServiceHealth;

namespace {

const folly::observer::Observer<std::chrono::milliseconds> kDefaultLiveness =
    folly::observer::makeStaticObserver(50ms);

class SimpleServiceHealth : public PolledServiceHealth {
 public:
  explicit SimpleServiceHealth(ServiceHealth health) : health_(health) {}

  folly::coro::Task<ServiceHealth> co_getServiceHealth() override {
    co_return health_.load(std::memory_order_relaxed);
  }

  void set(ServiceHealth health) {
    health_.store(health, std::memory_order_relaxed);
  }

 private:
  std::atomic<ServiceHealth> health_{};
};

class ServiceHealthResult {
 private:
  struct PrivateTag {};

 public:
  explicit ServiceHealthResult(PrivateTag) {}

  using PollLoop =
      std::tuple<std::shared_ptr<ServiceHealthResult>, folly::coro::Task<void>>;

  static PollLoop createLoop(folly::coro::AsyncGenerator<ServiceHealth> loop) {
    auto result = std::make_shared<ServiceHealthResult>(PrivateTag{});
    auto backgroundLoop = folly::coro::co_invoke(
        [result, loop = std::move(loop)]() mutable -> folly::coro::Task<void> {
          while (auto value = co_await loop.next()) {
            co_await folly::coro::co_safe_point;
            result->value_.store(*value, std::memory_order_relaxed);
            // ensure the baton is not reset() while posting
            result->called_.lock()->post();
          }
        });
    return std::make_tuple(result, std::move(backgroundLoop));
  }

  ServiceHealth get() const { return value_.load(std::memory_order_relaxed); }

  folly::coro::Task<void> co_wait() {
    CHECK(!waiting_.exchange(true, std::memory_order_relaxed))
        << "Only one concurrent waiter is allowed";
    called_.lock()->reset();
    // waiting and posting a baton concurrently is safe
    co_await called_.unsafeGetUnlocked();
    waiting_.store(false, std::memory_order_relaxed);
  }

  folly::coro::Task<ServiceHealth> co_next() {
    co_await co_wait();
    co_return get();
  }

 private:
  std::atomic<ServiceHealth> value_{};
  folly::Synchronized<folly::coro::Baton, std::mutex> called_;
  std::atomic<bool> waiting_{};
};

} // namespace

CO_TEST(ServiceHealthPoller, Basic) {
  folly::coro::CancellableAsyncScope scope;

  SimpleServiceHealth handler{ServiceHealth::OK};
  std::vector<PolledServiceHealth*> handlers{&handler};

  auto poll = ServiceHealthPoller::poll(std::move(handlers), kDefaultLiveness);
  auto [result, loop] = ServiceHealthResult::createLoop(std::move(poll));
  scope.add(co_withExecutor(folly::getGlobalCPUExecutor(), std::move(loop)));

  EXPECT_EQ(co_await result->co_next(), ServiceHealth::OK);

  co_await scope.cancelAndJoinAsync();
}

CO_TEST(ServiceHealthPoller, MergeStatusWithoutError) {
  folly::coro::CancellableAsyncScope scope;

  SimpleServiceHealth handler1{ServiceHealth::OK}, handler2{ServiceHealth::OK};
  std::vector<PolledServiceHealth*> handlers{&handler1, &handler2};

  auto poll = ServiceHealthPoller::poll(std::move(handlers), kDefaultLiveness);
  auto [result, loop] = ServiceHealthResult::createLoop(std::move(poll));
  scope.add(co_withExecutor(folly::getGlobalCPUExecutor(), std::move(loop)));

  EXPECT_EQ(co_await result->co_next(), ServiceHealth::OK);

  co_await scope.cancelAndJoinAsync();
}

CO_TEST(ServiceHealthPoller, MergeStatusWithError) {
  folly::coro::CancellableAsyncScope scope;

  SimpleServiceHealth handler1{ServiceHealth::OK},
      handler2{ServiceHealth::ERROR}, handler3{ServiceHealth::OK};
  std::vector<PolledServiceHealth*> handlers{&handler1, &handler2, &handler3};

  auto poll = ServiceHealthPoller::poll(std::move(handlers), kDefaultLiveness);
  auto [result, loop] = ServiceHealthResult::createLoop(std::move(poll));
  scope.add(co_withExecutor(folly::getGlobalCPUExecutor(), std::move(loop)));

  EXPECT_EQ(co_await result->co_next(), ServiceHealth::ERROR);

  co_await scope.cancelAndJoinAsync();
}

CO_TEST(ServiceHealthPoller, ChangingStatus) {
  folly::coro::CancellableAsyncScope scope;

  SimpleServiceHealth handler1{ServiceHealth::OK},
      handler2{ServiceHealth::ERROR}, handler3{ServiceHealth::OK};
  std::vector<PolledServiceHealth*> handlers{&handler1, &handler2, &handler3};

  auto poll = ServiceHealthPoller::poll(std::move(handlers), kDefaultLiveness);
  auto [result, loop] = ServiceHealthResult::createLoop(std::move(poll));
  scope.add(co_withExecutor(folly::getGlobalCPUExecutor(), std::move(loop)));

  EXPECT_EQ(co_await result->co_next(), ServiceHealth::ERROR);

  handler2.set(ServiceHealth::OK);
  EXPECT_EQ(co_await result->co_next(), ServiceHealth::OK);

  handler1.set(ServiceHealth::ERROR);
  EXPECT_EQ(co_await result->co_next(), ServiceHealth::ERROR);

  co_await scope.cancelAndJoinAsync();
}

CO_TEST(ServiceHealthPoller, NoChangeBetweenPolls) {
  folly::coro::CancellableAsyncScope scope;

  SimpleServiceHealth handler1{ServiceHealth::OK},
      handler2{ServiceHealth::ERROR}, handler3{ServiceHealth::OK};
  std::vector<PolledServiceHealth*> handlers{&handler1, &handler2, &handler3};

  auto liveness = kDefaultLiveness;

  auto poll = ServiceHealthPoller::poll(std::move(handlers), liveness);
  auto [result, loop] = ServiceHealthResult::createLoop(std::move(poll));
  scope.add(co_withExecutor(folly::getGlobalCPUExecutor(), std::move(loop)));

  EXPECT_EQ(co_await result->co_next(), ServiceHealth::ERROR);

  handler2.set(ServiceHealth::OK);
  co_await folly::coro::sleep(**liveness / 10);
  // unchanged - polling should not happen
  EXPECT_EQ(result->get(), ServiceHealth::ERROR);

  co_await folly::coro::sleep(**liveness * 5);
  // Change should be picked up after waiting long enough
  EXPECT_EQ(result->get(), ServiceHealth::OK);

  co_await scope.cancelAndJoinAsync();
}

CO_TEST(ServiceHealthPoller, Cancellation) {
  folly::coro::CancellableAsyncScope scope;

  SimpleServiceHealth handler{ServiceHealth::ERROR};
  std::vector<PolledServiceHealth*> handlers{&handler};

  auto liveness = folly::observer::makeStaticObserver(25ms);

  auto poll = ServiceHealthPoller::poll(std::move(handlers), liveness);
  auto [result, loop] = ServiceHealthResult::createLoop(std::move(poll));
  scope.add(co_withExecutor(folly::getGlobalCPUExecutor(), std::move(loop)));

  EXPECT_EQ(co_await result->co_next(), ServiceHealth::ERROR);

  co_await scope.cancelAndJoinAsync();

  handler.set(ServiceHealth::OK);
  co_await folly::coro::sleep(**liveness * 5);
  // Change should be picked up because polling is cancelled
  EXPECT_EQ(result->get(), ServiceHealth::ERROR);
}

CO_TEST(ServiceHealthPoller, DynamicLiveness) {
  folly::coro::CancellableAsyncScope scope;

  SimpleServiceHealth handler{ServiceHealth::ERROR};
  std::vector<PolledServiceHealth*> handlers{&handler};

  folly::observer::SimpleObservable<std::chrono::milliseconds> liveness{10ms};

  auto poll =
      ServiceHealthPoller::poll(std::move(handlers), liveness.getObserver());
  auto [result, loop] = ServiceHealthResult::createLoop(std::move(poll));
  scope.add(co_withExecutor(folly::getGlobalCPUExecutor(), std::move(loop)));

  EXPECT_EQ(co_await result->co_next(), ServiceHealth::ERROR);

  handler.set(ServiceHealth::OK);
  co_await folly::coro::sleep(50ms);
  EXPECT_EQ(result->get(), ServiceHealth::OK);

  liveness.setValue(200ms);
  folly::observer_detail::ObserverManager::waitForAllUpdates();
  // wait for any existing polling to be done
  co_await result->co_next();

  handler.set(ServiceHealth::ERROR);
  co_await folly::coro::sleep(50ms);
  // unchanged because liveness is large
  EXPECT_EQ(result->get(), ServiceHealth::OK);

  EXPECT_EQ(co_await result->co_next(), ServiceHealth::ERROR);

  co_await scope.cancelAndJoinAsync();
}
