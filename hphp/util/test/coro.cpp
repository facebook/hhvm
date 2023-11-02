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

namespace {

struct Error {};

bool always_true() {
  return std::chrono::steady_clock::now().time_since_epoch().count() > 0;
}

coro::Task<int> make_task(int x) {
  HPHP_CORO_RETURN(x);
}

coro::Task<int> make_task_throws(int x) {
  if (always_true()) throw Error{};
  HPHP_CORO_RETURN(x);
}

coro::Task<std::string> make_task(std::string x) {
  HPHP_CORO_RETURN(x);
}

coro::Task<void> make_task() {
  HPHP_CORO_RETURN_VOID;
}

coro::Task<void> make_task_throws() {
  if (always_true()) throw Error{};
  HPHP_CORO_RETURN_VOID;
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
  HPHP_CORO_MOVE_RETURN(c);
}

coro::Task<void> test_awaits() {
  EXPECT_EQ(HPHP_CORO_AWAIT(make_task(1)), 1);
  coro::Task<int> t1 = make_task(7);
  EXPECT_EQ(HPHP_CORO_AWAIT(std::move(t1)), 7);

  HPHP_CORO_AWAIT(make_task());
  coro::Task<void> t2 = make_task();
  HPHP_CORO_AWAIT(std::move(t2));

  EXPECT_EQ(HPHP_CORO_AWAIT(make_task_move(5)).m_int, 5);
  coro::Task<C> t3 = make_task_move(9);
  EXPECT_EQ(HPHP_CORO_AWAIT(std::move(t3)).m_int, 9);

  try {
    HPHP_CORO_AWAIT(make_task_throws(123));
    ADD_FAILURE();
  } catch (const Error&) {
    SUCCEED();
  }

  try {
    coro::Task<int> x = make_task_throws(123);
    HPHP_CORO_AWAIT(std::move(x));
    ADD_FAILURE();
  } catch (const Error&) {
    SUCCEED();
  }

  try {
    HPHP_CORO_AWAIT(make_task_throws());
    ADD_FAILURE();
  } catch (const Error&) {
    SUCCEED();
  }

  try {
    coro::Task<void> x = make_task_throws();
    HPHP_CORO_AWAIT(std::move(x));
    ADD_FAILURE();
  } catch (const Error&) {
    SUCCEED();
  }

  HPHP_CORO_RETURN_VOID;
}

}

TEST(Coro, Await) {
  coro::wait(test_awaits());
}

TEST(Coro, Wait) {
  EXPECT_EQ(coro::wait(make_task(1)), 1);
  coro::Task<int> t1 = make_task(7);
  EXPECT_EQ(coro::wait(std::move(t1)), 7);

  coro::wait(make_task());
  coro::Task<void> t2 = make_task();
  static_assert(
    std::is_same<decltype(coro::wait(std::move(t2))), void>::value
  );
  coro::wait(std::move(t2));

  EXPECT_EQ(coro::wait(make_task_move(5)).m_int, 5);
  coro::Task<C> t3 = make_task_move(9);
  EXPECT_EQ(coro::wait(std::move(t3)).m_int, 9);

  try {
    coro::wait(make_task_throws(123));
    ADD_FAILURE();
  } catch (const Error&) {
    SUCCEED();
  }

  try {
    coro::Task<int> x = make_task_throws(123);
    coro::wait(std::move(x));
    ADD_FAILURE();
  } catch (const Error&) {
    SUCCEED();
  }

  try {
    coro::wait(make_task_throws());
    ADD_FAILURE();
  } catch (const Error&) {
    SUCCEED();
  }

  try {
    coro::Task<void> x = make_task_throws();
    coro::wait(std::move(x));
    ADD_FAILURE();
  } catch (const Error&) {
    SUCCEED();
  }
}

TEST(Coro, Collect) {
  coro::Task<std::tuple<int>> t1 = coro::collect(make_task(1));
  std::tuple<int> t2 = coro::wait(std::move(t1));
  EXPECT_EQ(std::get<0>(t2), 1);

  coro::Task<std::tuple<int, std::string, C, folly::Unit>> t3 = coro::collect(
    make_task(8), make_task("abc"), make_task_move(11), make_task()
  );
  std::tuple<int, std::string, C, folly::Unit> t4 = coro::wait(std::move(t3));
  EXPECT_EQ(std::get<0>(t4), 8);
  EXPECT_EQ(std::get<1>(t4), "abc");
  EXPECT_EQ(std::get<2>(t4).m_int, 11);

  coro::Task<std::tuple<folly::Unit, folly::Unit>> t5 =
    coro::collect(make_task(), make_task());
  std::tuple<folly::Unit, folly::Unit> t6 = coro::wait(std::move(t5));
  (void)t6;

  try {
    coro::Task<std::tuple<int, int, C>> t = coro::collect(
      make_task(8), make_task_throws(9), make_task_move(11)
    );
    coro::wait(std::move(t));
    ADD_FAILURE();
  } catch (const Error&) {
    SUCCEED();
  }
}

TEST(Coro, CollectRange) {
  for (size_t size = 0; size < 10; ++size) {
    std::vector<coro::Task<int>> tasks1;
    for (size_t i = 0; i < size; ++i) {
      tasks1.emplace_back(make_task(i));
    }
    coro::Task<std::vector<int>> c1 = coro::collectRange(std::move(tasks1));
    std::vector<int> ints = coro::wait(std::move(c1));
    EXPECT_EQ(ints.size(), size);
    for (size_t i = 0; i < ints.size(); ++i) {
      EXPECT_EQ(ints[i], i);
    }

    std::vector<coro::Task<C>> tasks2;
    for (size_t i = 0; i < size; ++i) {
      tasks2.emplace_back(make_task_move(i));
    }
    coro::Task<std::vector<C>> c2 = coro::collectRange(std::move(tasks2));
    std::vector<C> cs = coro::wait(std::move(c2));
    EXPECT_EQ(cs.size(), size);
    for (size_t i = 0; i < cs.size(); ++i) {
      EXPECT_EQ(cs[i].m_int, i);
    }

    try {
      std::vector<coro::Task<int>> tasks3;
      for (size_t i = 0; i < size; ++i) {
        tasks3.emplace_back(make_task_throws(i));
      }
      coro::Task<std::vector<int>> c3 = coro::collectRange(std::move(tasks3));
      coro::wait(std::move(c3));
      EXPECT_FALSE(size > 0);
    } catch (const Error&) {
      EXPECT_TRUE(size > 0);
    }

    std::vector<coro::Task<void>> tasks4;
    for (size_t i = 0; i < size; ++i) {
      tasks4.emplace_back(make_task());
    }
    coro::Task<void> c4 = coro::collectRange(std::move(tasks4));
    static_assert(
      std::is_same<decltype(coro::wait(std::move(c4))), void>::value
    );
    coro::wait(std::move(c4));
  }
}

TEST(Coro, Invoke) {
  coro::Task<C> t1 = coro::invoke(&make_task_move, 101);
  EXPECT_EQ(coro::wait(std::move(t1)).m_int, 101);

  coro::Task<int> t2 = coro::invoke(
    [] (int x) -> coro::Task<int> { HPHP_CORO_RETURN(x); },
    123
  );
  EXPECT_EQ(coro::wait(std::move(t2)), 123);

  try {
    coro::Task<int> t = coro::invoke(
      [] (int x) -> coro::Task<int> {
        if (always_true()) throw Error{};
        HPHP_CORO_RETURN(x);
      },
      123
    );
    coro::wait(std::move(t));
    ADD_FAILURE();
  } catch (const Error&) {
    SUCCEED();
  }

  auto const int1 =
    std::chrono::steady_clock::now().time_since_epoch().count();
  auto const int2 =
    std::chrono::steady_clock::now().time_since_epoch().count() + 100;
  auto const int3 =
    std::chrono::steady_clock::now().time_since_epoch().count() + 200;
  auto const int4 =
    std::chrono::steady_clock::now().time_since_epoch().count() + 300;

  auto const takesRef = [] (const std::string& s) -> coro::Task<std::string> {
    HPHP_CORO_RETURN(folly::sformat("{}-{}", s, s));
  };
  coro::Task<std::string> t3 = coro::invoke(takesRef, std::to_string(int1));
  coro::Task<std::string> t4 = coro::invoke(takesRef, std::to_string(int2));
  coro::Task<std::string> t5 = coro::invoke(takesRef, std::to_string(int3));
  coro::Task<std::string> t6 = coro::invoke(takesRef, std::to_string(int4));

  coro::Task<
    std::tuple<std::string, std::string, std::string, std::string>
  > t7 =
    coro::collect(std::move(t6), std::move(t5), std::move(t4), std::move(t3));
  std::tuple<std::string, std::string, std::string, std::string> t8 =
    coro::wait(std::move(t7));

  EXPECT_EQ(std::get<0>(t8), folly::sformat("{}-{}", int4, int4));
  EXPECT_EQ(std::get<1>(t8), folly::sformat("{}-{}", int3, int3));
  EXPECT_EQ(std::get<2>(t8), folly::sformat("{}-{}", int2, int2));
  EXPECT_EQ(std::get<3>(t8), folly::sformat("{}-{}", int1, int1));
}

TEST(Coro, Sleep) {
  auto const sleeper = [] () -> coro::Task<void> {
    HPHP_CORO_AWAIT(coro::sleep(1s));
    HPHP_CORO_RETURN_VOID;
  };

  auto const before = std::chrono::steady_clock::now();
  coro::wait(sleeper());
  auto const secs =
    std::chrono::duration_cast<std::chrono::seconds>(
      std::chrono::steady_clock::now() - before
    ).count();
  EXPECT_TRUE(secs >= 1);
}

TEST(Coro, AsyncValue) {
  coro::AsyncValue<int> a1{
    [] () -> coro::Task<int> {
      HPHP_CORO_AWAIT(coro::sleep(250ms));
      HPHP_CORO_RETURN(HPHP_CORO_AWAIT(make_task(123)));
    },
    folly::getGlobalCPUExecutor()
  };
  coro::Task<const int*> t1 = *a1;
  EXPECT_EQ(*coro::wait(std::move(t1)), 123);

  coro::AsyncValue<int> a2{
    [] () -> coro::Task<int> {
      HPHP_CORO_AWAIT(coro::sleep(250ms));
      HPHP_CORO_RETURN(HPHP_CORO_AWAIT(make_task(456)));
    },
    folly::getGlobalCPUExecutor()
  };
  coro::Task<int> t2 = a2.getCopy();
  EXPECT_EQ(coro::wait(std::move(t2)), 456);

  coro::AsyncValue<C> a3{
    [] () -> coro::Task<C> {
      HPHP_CORO_AWAIT(coro::sleep(250ms));
      HPHP_CORO_RETURN(HPHP_CORO_AWAIT(make_task_move(605)));
    },
    folly::getGlobalCPUExecutor()
  };
  coro::Task<const C*> t3 = *a3;
  const C* c1 = coro::wait(std::move(t3));
  EXPECT_EQ(c1->m_int, 605);

  coro::AsyncValue<int> a4{
    [] () -> coro::Task<int> {
      HPHP_CORO_AWAIT(coro::sleep(250ms));
      HPHP_CORO_RETURN(HPHP_CORO_AWAIT(make_task_throws(1)));
    },
    folly::getGlobalCPUExecutor()
  };

  try {
    coro::Task<const int*> t = *a4;
    coro::wait(std::move(t));
    ADD_FAILURE();
  } catch (const Error&) {
    SUCCEED();
  }

  coro::AsyncValue<int> a5{
    [] () -> coro::Task<int> {
      HPHP_CORO_AWAIT(coro::sleep(250ms));
      HPHP_CORO_RETURN(HPHP_CORO_AWAIT(make_task_throws(1)));
    },
    folly::getGlobalCPUExecutor()
  };

  try {
    coro::Task<int> t = a5.getCopy();
    coro::wait(std::move(t));
    ADD_FAILURE();
  } catch (const Error&) {
    SUCCEED();
  }
}

TEST(Coro, AsyncMap) {
  coro::AsyncMap<std::string, int> map;
  coro::Task<int> t1 = map.get(
    "123",
    [&] {
      return [] () -> coro::Task<int> {
        HPHP_CORO_AWAIT(coro::sleep(250ms));
        HPHP_CORO_RETURN(HPHP_CORO_AWAIT(make_task(123)));
      };
    },
    folly::getGlobalCPUExecutor()
  );
  EXPECT_EQ(coro::wait(std::move(t1)), 123);

  coro::Task<int> t2 = map.get(
    "123",
    [&] {
      return [] () -> coro::Task<int> {
        ADD_FAILURE();
        HPHP_CORO_RETURN(HPHP_CORO_AWAIT(make_task(456)));
      };
    },
    folly::getGlobalCPUExecutor()
  );
  EXPECT_EQ(coro::wait(std::move(t2)), 123);

  coro::Task<int> t3 = map.get(
    "456",
    [&] {
      return [] () -> coro::Task<int> {
        HPHP_CORO_AWAIT(coro::sleep(250ms));
        HPHP_CORO_RETURN(HPHP_CORO_AWAIT(make_task(456)));
      };
    },
    folly::getGlobalCPUExecutor()
  );
  EXPECT_EQ(coro::wait(std::move(t3)), 456);

  try {
    coro::Task<int> t4 = map.get(
      "789",
      [&] {
        return [] () -> coro::Task<int> {
          HPHP_CORO_AWAIT(coro::sleep(250ms));
          HPHP_CORO_RETURN(HPHP_CORO_AWAIT(make_task_throws(789)));
        };
      },
      folly::getGlobalCPUExecutor()
    );
    coro::wait(std::move(t4));
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
          HPHP_CORO_RETURN(HPHP_CORO_AWAIT(make_task(789)));
        };
      },
      folly::getGlobalCPUExecutor()
    );
    coro::wait(std::move(t5));
    ADD_FAILURE();
  } catch (const Error&) {
    SUCCEED();
  }
}

TEST(Coro, AsyncScope) {
  auto const asyncSet = [] (int in, int* out) -> coro::Task<void> {
    HPHP_CORO_RESCHEDULE_ON_CURRENT_EXECUTOR;
    HPHP_CORO_AWAIT(coro::sleep(1s));
    *out = in;
    HPHP_CORO_RETURN_VOID;
  };

  coro::AsyncScope scope;
  int v1 = 0;
  int v2 = 0;
  int v3 = 0;
  scope.add(asyncSet(123, &v1).scheduleOn(folly::getGlobalCPUExecutor()));
  scope.add(asyncSet(456, &v2).scheduleOn(folly::getGlobalCPUExecutor()));
  scope.add(asyncSet(789, &v3).scheduleOn(folly::getGlobalCPUExecutor()));
  coro::wait(scope.joinAsync());

  EXPECT_EQ(v1, 123);
  EXPECT_EQ(v2, 456);
  EXPECT_EQ(v3, 789);
}

TEST(Coro, TicketExecutor) {
  coro::TicketExecutor exec{
    "coro-gtest",
    0, 0,
    [] {},
    [] {},
    24h
  };
  EXPECT_EQ(exec.numThreads(), 0);

  std::vector<int> order;
  auto const async = [&] (int i1, int i2) -> coro::Task<void> {
    HPHP_CORO_RESCHEDULE_ON_CURRENT_EXECUTOR;
    order.emplace_back(i1);
    HPHP_CORO_RESCHEDULE_ON_CURRENT_EXECUTOR;
    order.emplace_back(i2);
    HPHP_CORO_RETURN_VOID;
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

  EXPECT_EQ(exec.getPendingTaskCount(), coro::using_coros ? 4 : 0);

  exec.setNumThreads(1);
  EXPECT_EQ(exec.numThreads(), 1);
  coro::wait(scope.joinAsync());

  EXPECT_EQ(exec.getPendingTaskCount(), 0);

  const std::vector<int> expected{1, 10, 2, 20, 3, 30, 4, 40};
  EXPECT_EQ(order, expected);
}

TEST(Coro, CurrentExecutor) {
  auto const async = [&] () -> coro::Task<folly::Executor*> {
    HPHP_CORO_RETURN(HPHP_CORO_CURRENT_EXECUTOR);
  };
  auto global = folly::getGlobalCPUExecutor();
  auto executor = coro::wait(async().scheduleOn(global));
  EXPECT_EQ(coro::using_coros ? global.get() : nullptr, executor);
}

TEST(Coro, Latch) {
  coro::Latch latch1{3};
  coro::Latch latch2{1};

  std::atomic<int> sum{0};
  auto const worker = [&] (int x) -> coro::Task<void> {
    HPHP_CORO_RESCHEDULE_ON_CURRENT_EXECUTOR;
    sum += x;
    latch1.count_down();
    HPHP_CORO_AWAIT(latch2.wait());
    sum += x;
    HPHP_CORO_RETURN_VOID;
  };

  coro::AsyncScope scope;
  scope.add(worker(101).scheduleOn(folly::getGlobalCPUExecutor()));
  scope.add(worker(50).scheduleOn(folly::getGlobalCPUExecutor()));
  scope.add(worker(60).scheduleOn(folly::getGlobalCPUExecutor()));

  coro::wait(latch1.wait());
  EXPECT_EQ(sum, coro::using_coros ? 211 : 422);
  latch2.count_down();
  coro::wait(scope.joinAsync());
  EXPECT_EQ(sum, 422);
}

}
