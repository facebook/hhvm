/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include "hphp/util/coro.h"

#include <folly/Format.h>
#include <folly/executors/GlobalExecutor.h>

#include <gtest/gtest.h>

namespace HPHP {

using namespace std::chrono_literals;
namespace coro = folly::coro;

namespace {

struct Error {};

bool always_true() {
  return std::chrono::steady_clock::now().time_since_epoch().count() > 0;
}

coro::Task<int> make_task(int x) {
  co_return x;
}

coro::Task<int> make_task_throws(int x) {
  if (always_true()) throw Error{};
  co_return x;
}

coro::Task<std::string> make_task(std::string x) {
  co_return x;
}

coro::Task<void> make_task() {
  co_return;
}

coro::Task<void> make_task_throws() {
  if (always_true()) throw Error{};
  co_return;
}

struct C {
  C() = default;
  C(const C&) = delete;
  C(C&&) = default;
  C& operator=(const C&) = delete;
  C& operator=(C&&) = default;
  int m_int{0};
};

coro::Task<C> make_task_move(int x) {
  C c;
  c.m_int = x;
  co_return c;
}

}

TEST(Coro, AsyncValue) {
  CoroAsyncValue<int> a1{
    [] () -> coro::Task<int> {
      co_await coro::sleep(250ms);
      co_return co_await make_task(123);
    },
    folly::getGlobalCPUExecutor()
  };
  coro::Task<const int*> t1 = *a1;
  EXPECT_EQ(*coro::blockingWait(std::move(t1)), 123);

  CoroAsyncValue<int> a2{
    [] () -> coro::Task<int> {
      co_await coro::sleep(250ms);
      co_return co_await make_task(456);
    },
    folly::getGlobalCPUExecutor()
  };
  coro::Task<int> t2 = a2.getCopy();
  EXPECT_EQ(coro::blockingWait(std::move(t2)), 456);

  CoroAsyncValue<C> a3{
    [] () -> coro::Task<C> {
      co_await coro::sleep(250ms);
      co_return co_await make_task_move(605);
    },
    folly::getGlobalCPUExecutor()
  };
  coro::Task<const C*> t3 = *a3;
  const C* c1 = coro::blockingWait(std::move(t3));
  EXPECT_EQ(c1->m_int, 605);

  CoroAsyncValue<int> a4{
    [] () -> coro::Task<int> {
      co_await coro::sleep(250ms);
      co_return co_await make_task_throws(1);
    },
    folly::getGlobalCPUExecutor()
  };

  try {
    coro::Task<const int*> t = *a4;
    coro::blockingWait(std::move(t));
    ADD_FAILURE();
  } catch (const Error&) {
    SUCCEED();
  }

  CoroAsyncValue<int> a5{
    [] () -> coro::Task<int> {
      co_await coro::sleep(250ms);
      co_return co_await make_task_throws(1);
    },
    folly::getGlobalCPUExecutor()
  };

  try {
    coro::Task<int> t = a5.getCopy();
    coro::blockingWait(std::move(t));
    ADD_FAILURE();
  } catch (const Error&) {
    SUCCEED();
  }
}

TEST(Coro, AsyncMap) {
  CoroAsyncMap<std::string, int> map;
  coro::Task<int> t1 = map.get(
    "123",
    [&] {
      return [] () -> coro::Task<int> {
        co_await coro::sleep(250ms);
        co_return co_await make_task(123);
      };
    },
    folly::getGlobalCPUExecutor()
  );
  EXPECT_EQ(coro::blockingWait(std::move(t1)), 123);

  coro::Task<int> t2 = map.get(
    "123",
    [&] {
      return [] () -> coro::Task<int> {
        ADD_FAILURE();
        co_return co_await make_task(456);
      };
    },
    folly::getGlobalCPUExecutor()
  );
  EXPECT_EQ(coro::blockingWait(std::move(t2)), 123);

  coro::Task<int> t3 = map.get(
    "456",
    [&] {
      return [] () -> coro::Task<int> {
        co_await coro::sleep(250ms);
        co_return co_await make_task(456);
      };
    },
    folly::getGlobalCPUExecutor()
  );
  EXPECT_EQ(coro::blockingWait(std::move(t3)), 456);

  try {
    coro::Task<int> t4 = map.get(
      "789",
      [&] {
        return [] () -> coro::Task<int> {
          co_await coro::sleep(250ms);
          co_return co_await make_task_throws(789);
        };
      },
      folly::getGlobalCPUExecutor()
    );
    coro::blockingWait(std::move(t4));
    ADD_FAILURE();
  } catch (const Error&) {
    SUCCEED();
  }

  try {
    coro::Task<int> t5 = map.get(
      "789",
      [&] {
        return [] () -> coro::Task<int> {
          ADD_FAILURE();
          co_return co_await make_task(789);
        };
      },
      folly::getGlobalCPUExecutor()
    );
    coro::blockingWait(std::move(t5));
    ADD_FAILURE();
  } catch (const Error&) {
    SUCCEED();
  }
}

TEST(Coro, TicketExecutor) {
  TicketExecutor exec{
    "coro-gtest",
    0, 0,
    [] {},
    [] {},
    24h
  };
  EXPECT_EQ(exec.numThreads(), 0);

  std::vector<int> order;
  auto const async = [&] (int i1, int i2) -> coro::Task<void> {
    co_await coro::co_reschedule_on_current_executor;
    order.emplace_back(i1);
    co_await coro::co_reschedule_on_current_executor;
    order.emplace_back(i2);
    co_return;
  };

  auto a1 = async(1, 10).scheduleOn(exec.sticky());
  auto a2 = async(2, 20).scheduleOn(exec.sticky());
  auto a3 = async(3, 30).scheduleOn(exec.sticky());
  auto a4 = async(4, 40).scheduleOn(exec.sticky());

  EXPECT_EQ(exec.getPendingTaskCount(), 0);

  coro::AsyncScope scope;
  scope.add(std::move(a4));
  scope.add(std::move(a3));
  scope.add(std::move(a2));
  scope.add(std::move(a1));

  EXPECT_EQ(exec.getPendingTaskCount(), 4);

  exec.setNumThreads(1);
  EXPECT_EQ(exec.numThreads(), 1);
  coro::blockingWait(scope.joinAsync());

  EXPECT_EQ(exec.getPendingTaskCount(), 0);

  const std::vector<int> expected{1, 10, 2, 20, 3, 30, 4, 40};
  EXPECT_EQ(order, expected);
}

TEST(Coro, Latch) {
  CoroLatch latch1{3};
  CoroLatch latch2{1};

  std::atomic<int> sum{0};
  auto const worker = [&] (int x) -> coro::Task<void> {
    co_await coro::co_reschedule_on_current_executor;
    sum += x;
    latch1.count_down();
    co_await latch2.wait();
    sum += x;
    co_return;
  };

  coro::AsyncScope scope;
  scope.add(worker(101).scheduleOn(folly::getGlobalCPUExecutor()));
  scope.add(worker(50).scheduleOn(folly::getGlobalCPUExecutor()));
  scope.add(worker(60).scheduleOn(folly::getGlobalCPUExecutor()));

  coro::blockingWait(latch1.wait());
  EXPECT_EQ(sum, 211);
  latch2.count_down();
  coro::blockingWait(scope.joinAsync());
  EXPECT_EQ(sum, 422);
}

}
