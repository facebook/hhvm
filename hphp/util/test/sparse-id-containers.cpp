/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/util/sparse-id-containers.h"
#include <gtest/gtest.h>

#include <cstdint>
#include <initializer_list>
#include <vector>
#include <stdexcept>

#include <folly/ScopeGuard.h>

namespace HPHP {

namespace {

//////////////////////////////////////////////////////////////////////

constexpr uint16_t kTestUniverseSize = 4096;

sparse_id_set<uint16_t> make_test_set(std::initializer_list<uint16_t> vals) {
  auto ret = sparse_id_set<uint16_t>{kTestUniverseSize};
  for (auto v : vals) ret.insert(v);
  return ret;
}

using IntMap = sparse_id_map<uint32_t,uint32_t>;

IntMap make_test_map(
    std::initializer_list<std::pair<uint32_t,uint32_t>> vals) {
  auto ret = IntMap{kTestUniverseSize};
  for (auto v : vals) ret.insert(v);
  return ret;
}

struct LiveCount {
  static uint32_t count;

  LiveCount() { ++count; }
  LiveCount(const LiveCount&) { ++count; }
  ~LiveCount() { --count; }
};
uint32_t LiveCount::count = 0;

struct CopyThrower {
  static uint32_t throw_plan;
  static uint32_t current_count;
  static void plan_throw(uint32_t when) {
    throw_plan = when;
    current_count = 0;
  }

  CopyThrower() = default;
  CopyThrower(const CopyThrower&) {
    if (++current_count == throw_plan) {
      throw std::runtime_error("ok");
    }
  }

  LiveCount counter;
};
uint32_t CopyThrower::throw_plan;
uint32_t CopyThrower::current_count;

//////////////////////////////////////////////////////////////////////

}

TEST(SparseIdSet, Basic) {
  sparse_id_set<uint16_t> set(2048);

  EXPECT_FALSE(set.contains(2));
  EXPECT_FALSE(set.contains(0));
  EXPECT_FALSE(set.contains(8));

  set.insert(2);
  set.insert(2);
  set.insert(8);
  set.insert(2);
  set.insert(0);

  EXPECT_TRUE(set.contains(2));
  EXPECT_TRUE(set.contains(8));
  EXPECT_TRUE(set.contains(0));
  EXPECT_EQ(3, set.size());
  set.erase(8);
  EXPECT_FALSE(set.contains(8));
  EXPECT_EQ(2, set.size());
}

TEST(SparseIdSet, Unions) {
  auto const a = make_test_set({1, 42, 1234, 1, 234, 2, 2, 3});
  auto const b = make_test_set({500, 600, 700, 800});
  auto const manual_union = make_test_set({
    1, 42, 1234, 1, 234, 2, 2, 3,
    500, 600, 700, 800
  });

  auto tmpSet = a;
  tmpSet |= b;

  EXPECT_EQ(tmpSet, manual_union);
}

TEST(SparseIdSet, Intersect) {
  auto const a = make_test_set({1, 2, 3, 4, 5, 6});
  auto const b = make_test_set({2, 3, 4, 5});

  auto c = a;
  c &= b;
  EXPECT_EQ(make_test_set({2,3,4,5}), c);

  c &= make_test_set({3});
  EXPECT_TRUE(c.contains(3));
  EXPECT_FALSE(c.contains(2));
  EXPECT_EQ(make_test_set({3}), c);
}

TEST(SparseIdSet, IterationOrder) {
  sparse_id_set<uint16_t> set(kTestUniverseSize);
  auto vec = std::vector<sparse_id_set<uint16_t>::value_type>{};

  for (auto i = uint16_t{0}; i < 1024; ++i) {
    auto val = std::rand() % kTestUniverseSize;
    if (!set.contains(val)) {
      set.insert(val);
      vec.push_back(val);
    }
  }

  auto iter = set.begin();
  for (auto t : vec) {
    EXPECT_TRUE(iter != set.end());
    if (iter == set.end()) break;
    EXPECT_EQ(t, *iter);
    ++iter;
  }
}

TEST(SparseIdSet, EraseBack) {
  sparse_id_set<uint16_t> set(kTestUniverseSize);

  EXPECT_TRUE(set.empty());

  set.insert(10);
  set.insert(2);
  set.insert(5);
  for (auto i = 0; i < 999; ++i) {
    set.insert(6);
  }

  EXPECT_FALSE(set.empty());
  EXPECT_TRUE(set.contains(6));
  EXPECT_EQ(6, set.back());
  EXPECT_EQ(10, set.front());
  EXPECT_EQ(4, set.size());

  set.erase(6);
  EXPECT_EQ(3, set.size());
  EXPECT_EQ(5, set.back());
  EXPECT_FALSE(set.contains(6));

  set.erase(5);
  EXPECT_EQ(2, set.size());
  EXPECT_EQ(2, set.back());
  EXPECT_FALSE(set.contains(5));
  EXPECT_FALSE(set.contains(6));
  EXPECT_TRUE(set.contains(2));
  EXPECT_TRUE(set.contains(10));

  set.erase(2);
  EXPECT_EQ(1, set.size());
  EXPECT_EQ(10, set.back());
  EXPECT_EQ(set.front(), set.back());
  EXPECT_TRUE(set.contains(10));
  EXPECT_FALSE(set.contains(5));
  EXPECT_FALSE(set.contains(6));
  EXPECT_FALSE(set.contains(2));

  set.erase(10);
  EXPECT_TRUE(set.empty());
}

TEST(SparseIdSet, Clear) {
  auto x = make_test_set({1234, 124, 1234, 5});
  EXPECT_EQ(3 /* no duplicate above */, x.size());
  x.clear();
  EXPECT_TRUE(x.empty());
  EXPECT_EQ(0, x.size());
}

TEST(SparseIdSet, Show) {
  auto x = make_test_set({55, 34, 12});
  EXPECT_EQ("55 34 12", show(x));
}

TEST(SparseIdSet, MoveAssign) {
  auto x = make_test_set({55, 34, 12});
  sparse_id_set<uint16_t> tmp(kTestUniverseSize);
  tmp = std::move(x);
  sparse_id_set<uint16_t> tmp2(0);
  tmp2 = std::move(tmp);
}

//////////////////////////////////////////////////////////////////////

TEST(SparseIdMap, Basic) {
  sparse_id_map<uint32_t,std::string> map(2048);

  EXPECT_TRUE(map.empty());

  map[1024] = "ok";
  EXPECT_FALSE(map.contains(0));
  map[0] = "hi";

  EXPECT_EQ("ok", map[1024]);
  EXPECT_EQ("hi", map[0]);
  EXPECT_EQ(std::string("ok"), map.front().second);
  EXPECT_EQ(std::string("hi"), map.back().second);

  EXPECT_TRUE(map.contains(0));
  EXPECT_TRUE(map.contains(1024));

  EXPECT_EQ(2, map.size());
  map.erase(1024);
  EXPECT_EQ(1, map.size());
}

TEST(SparseIdMap, Dtors) {
  SCOPE_EXIT { EXPECT_EQ(0, LiveCount::count); };
  sparse_id_map<uint32_t,LiveCount> map(4096);

  EXPECT_EQ(0, LiveCount::count);
  map[12] = LiveCount();
  EXPECT_EQ(1, LiveCount::count);
  map[12] = LiveCount();
  EXPECT_EQ(1, LiveCount::count);
  map[0] = LiveCount();
  EXPECT_EQ(2, LiveCount::count);
  map.clear();
  EXPECT_EQ(0, LiveCount::count);

  sparse_id_map<uint32_t,LiveCount> another(2048);
  for (auto i = uint32_t{0}; i < 10; ++i) {
    another[i] = LiveCount{};
  }
  EXPECT_EQ(10, LiveCount::count);
  map = std::move(another);
  EXPECT_EQ(10, LiveCount::count);
  EXPECT_EQ(10, map.size());
  auto idx = uint32_t{0};
  for (auto& kv : map) {
    EXPECT_EQ(idx, kv.first);
    ++idx;
  }
}

TEST(SparseIdMap, Erases) {
  auto test = make_test_map({{10, 10}, {24, 24}, {32, 32}});
  EXPECT_EQ(3, test.size());
  EXPECT_EQ(10, test.front().first);
  EXPECT_EQ(32, test.back().first);
  test.erase(24);
  EXPECT_EQ(2, test.size());
  EXPECT_EQ(10, test.front().first);
  EXPECT_EQ(32, test.back().first);

  for (auto i = uint32_t{0}; i < 1024; ++i) {
    test[i + 1024] = i + 1024;
  }
  EXPECT_EQ(1024 * 2 - 1, test.back().first);

  auto sz = 1024 + 2;
  do {
    EXPECT_EQ(sz, test.size());
    test.erase(test.front().first);
    --sz;
  } while (!test.empty());
}

TEST(SparseIdMap, Compare) {
  auto testa = make_test_map({{1, 1}, {2, 2}});
  auto testb = make_test_map({{2, 2}, {1, 1}});
  auto testc = make_test_map({{2, 1}, {1, 1}});
  EXPECT_EQ(testa, testa);
  EXPECT_EQ(testb, testb);
  EXPECT_EQ(testa, testb);
  EXPECT_NE(testa, testc);
  EXPECT_NE(testb, testc);
  EXPECT_EQ(testc, testc);
}

TEST(SparseIdMap, ExceptionCleanup) {
  using M = sparse_id_map<uint32_t,CopyThrower>;

  bool caught{true};
  for (auto plan_counter = uint32_t{0}; caught; ++plan_counter) {
    CopyThrower::plan_throw(plan_counter);
    caught = false;
    try {
      M thing(kTestUniverseSize);
      for (auto i = uint32_t{0}; i < 15; ++i) {
        thing.insert(std::make_pair(i, CopyThrower()));
      }
      M thing2 = thing;
      M thing3(0);
      thing3 = thing2;
    } catch (...) {
      EXPECT_EQ(0, LiveCount::count);
      caught = true;
    }
  }

  EXPECT_EQ(0, LiveCount::count);
}

TEST(SparseIdMap, Merge) {
  struct Foo {
    bool operator==(Foo f) const { return value == f.value; }
    uint32_t value;
  };

  sparse_id_map<uint32_t,Foo> a(1024);
  sparse_id_map<uint32_t,Foo> b(1024);

  a[0] = Foo { 0 };
  a[7] = Foo { 7 };
  for (auto i = uint32_t{0}; i < 200; ++i) {
    a[30 + i] = Foo { 30 + i };
  }
  a[7] = Foo { 3 };
  b[2] = Foo { 10 };
  b[7] = Foo { 70 };
  b[1] = Foo { 10 };
  b[0] = Foo { 10 };

  EXPECT_EQ(202, a.size());
  EXPECT_EQ(4, b.size());

  auto merge_calls = uint32_t{0};
  auto merge_fn = [&] (Foo& dst, const Foo& src) {
    auto const old = dst.value;
    dst.value = std::max(dst.value, src.value);
    ++merge_calls;
    return old != dst.value;
  };

  EXPECT_TRUE(a.merge(b, merge_fn));
  EXPECT_EQ(2, merge_calls);

  EXPECT_EQ(2, a.size());

  EXPECT_EQ(10, a[0].value);
  EXPECT_EQ(70, a[7].value);

  // Merge a back into b.  They should be the same now.
  EXPECT_EQ(4, b.size());
  merge_calls = 0;
  EXPECT_EQ(2, a.size());
  EXPECT_TRUE(b.merge(a, merge_fn));
  EXPECT_EQ(2, merge_calls);
  EXPECT_EQ(a.size(), b.size());
  EXPECT_EQ(a, b);

  // Now they shouldn't change either direction:
  EXPECT_FALSE(a.merge(b, merge_fn));
  EXPECT_FALSE(b.merge(a, merge_fn));
}

//////////////////////////////////////////////////////////////////////

TEST(SparseId, StructyLookups) {
  struct StrongerTypedId {
    uint32_t id;
    /* implicit */ operator uint32_t() { return id; }
  };

  sparse_id_map<uint32_t,int,StrongerTypedId> map(12);
  map[StrongerTypedId{0}] = 2;
  EXPECT_EQ(1, map.size());
  map.erase(StrongerTypedId{0});
  EXPECT_EQ(0, map.size());

  sparse_id_set<uint32_t,StrongerTypedId> set(12);
  set.insert(StrongerTypedId{2});
  EXPECT_EQ(1, set.size());
  set.erase(StrongerTypedId{0});
  EXPECT_EQ(1, set.size());
  set.erase(StrongerTypedId{2});
  EXPECT_EQ(0, set.size());
}

TEST(SparseId, PointeryLookups) {
  struct SSATmp {
    uint32_t id() const { return m_id; }
    uint32_t m_id;
  };

  struct IDEx {
    uint32_t operator()(SSATmp* tmp) const { return tmp->id(); }
  };

  SSATmp one { 1 };
  SSATmp two { 2 };
  SSATmp three { 3 };

  sparse_id_map<uint32_t,int,SSATmp*,IDEx> map(5);

  map[&one] = 2;
  map[&two] = 3;
  map[&three] = 4;

  EXPECT_EQ(3, map.size());
}

//////////////////////////////////////////////////////////////////////

}
