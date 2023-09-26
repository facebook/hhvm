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
#include "hphp/util/tiny-vector.h"
#include <gtest/gtest.h>

namespace HPHP {

namespace {

struct Tester {
  explicit Tester(int x, int* alive = nullptr): x{x}, alive{alive} {
    if (alive) ++*alive;
  }
  Tester(const Tester& o): x{o.x}, alive{o.alive} {
    if (alive) ++*alive;
  }
  Tester(Tester&& o) noexcept: x{o.x}, alive{o.alive} {
    if (alive) ++*alive;
  }
  ~Tester() { if (alive) --*alive; }
  Tester& operator=(const Tester&) = default;
  Tester& operator=(Tester&&) = default;
  bool operator==(const Tester& t) const { return x == t.x; }
  bool operator<(const Tester& t) const { return x < t.x; }
  int x;
  int* alive;
};

using TVec = TinyVector<Tester, 2>;
using TesterVec = std::vector<Tester>;
using TestPair = std::pair<TVec, TesterVec>;
using TestPairVec = std::vector<TestPair>;

TestPairVec all(int* alive = nullptr) {
  Tester t3{3, alive};
  Tester t7{7, alive};
  Tester t10{10, alive};
  Tester t21{21, alive};
  Tester t33{33, alive};

  return TestPairVec{
    TestPair{TVec{}, TesterVec{}},
    TestPair{TVec{t10}, TesterVec{t10}},
    TestPair{TVec{t21}, TesterVec{t21}},
    TestPair{TVec{t3, t7}, TesterVec{t3, t7}},
    TestPair{TVec{t7, t3}, TesterVec{t7, t3}},
    TestPair{TVec{t10, t7, t21}, TesterVec{t10, t7, t21}},
    TestPair{TVec{t21, t3, t10}, TesterVec{t21, t3, t10}},
    TestPair{TVec{t10, t10, t10}, TesterVec{t10, t10, t10}},
    TestPair{TVec{t33, t10, t3, t7, t21}, TesterVec{t33, t10, t3, t7, t21}},
    TestPair{TVec{t7, t10, t3, t33, t21}, TesterVec{t7, t10, t3, t33, t21}},
    TestPair{TVec{t21, t3, t10, t7, t33}, TesterVec{t21, t3, t10, t7, t33}}
  };
}

}

TEST(TinyVector, Basic) {
  auto const fillPush = [] (int s, int* alive = nullptr) {
    TVec v;
    for (int i = 0; i < s; ++i) v.push_back(Tester{i, alive});
    return v;
  };

  auto const fillEmplace = [] (int s, int* alive = nullptr) {
    TVec v;
    for (int i = 0; i < s; ++i) v.emplace_back(i, alive);
    return v;
  };

  auto const check = [&] (const TVec& v, int size) {
    EXPECT_EQ(v.size(), size);
    EXPECT_EQ(v.empty(), size == 0);
    if (size > 0) {
      EXPECT_EQ(v.front().x, 0);
      EXPECT_EQ(v.back().x, size - 1);
    }
    int current = 0;
    for (auto const& e : v) {
      EXPECT_EQ(e.x, current);
      EXPECT_EQ(e.x, v[current].x);
      ++current;
    }
    EXPECT_EQ(v.size(), current);
  };

  int alive = 0;
  for (int size = 0; size < 10; ++size) {
    EXPECT_EQ(alive, 0);
    auto const v = fillPush(size, &alive);
    EXPECT_EQ(alive, size);

    auto v2 = v;
    EXPECT_EQ(v, v2);
    EXPECT_FALSE(v != v2);
    check(v, size);
    check(v2, size);
    EXPECT_EQ(alive, size*2);

    for (int i = size; i > 0; --i) {
      v2.pop_back();
      check(v2, i - 1);
      EXPECT_NE(v, v2);
    }
    EXPECT_TRUE(v2.empty());
    EXPECT_EQ(alive, size);

    auto v3 = v;
    TVec v4(std::move(v3));
    check(v4, size);

    v3 = std::move(v4);
    check(v3, size);

    v4 = v3;
    check(v3, size);
    check(v4, size);
    EXPECT_EQ(v3, v4);

    v4.clear();
    check(v4, 0);
    EXPECT_EQ(v2, v4);

    auto const oldAlive = alive;
    v3.reserve(1000);
    EXPECT_EQ(alive, oldAlive);
    check(v3, size);
    EXPECT_EQ(v3, v);
  }
  EXPECT_EQ(alive, 0);

  for (int size = 0; size < 10; ++size) {
    check(fillEmplace(size), size);
    EXPECT_EQ(fillEmplace(size), fillPush(size));
  }
  EXPECT_EQ(alive, 0);

  auto const checkPair = [&] (const TestPair& p) {
    EXPECT_EQ(p.first.size(), p.second.size());
    EXPECT_EQ(p.first.empty(), p.second.empty());
    if (!p.first.empty()) {
      EXPECT_EQ(p.first.front(), p.second.front());
      EXPECT_EQ(p.first.back(), p.second.back());
    }
    for (int i = 0; i < p.first.size(); ++i) {
      EXPECT_EQ(p.first[i], p.second[i]);
    }
  };

  for (auto const& p : all(&alive)) checkPair(p);
  EXPECT_EQ(alive, 0);

  for (auto const& p1 : all(&alive)) {
    for (auto const& p2 : all(&alive)) {
      EXPECT_EQ(p1.first == p2.first, p1.second == p2.second);
      EXPECT_EQ(p1.first != p2.first, p1.second != p2.second);
      EXPECT_EQ(p1.first < p2.first, p1.second < p2.second);
      EXPECT_EQ(p1.first <= p2.first, p1.second <= p2.second);
      EXPECT_EQ(p1.first > p2.first, p1.second > p2.second);
      EXPECT_EQ(p1.first >= p2.first, p1.second >= p2.second);

      auto p3 = p1;
      auto p4 = p2;

      swap(p3.first, p4.first);
      swap(p3.second, p4.second);
      checkPair(p3);
      checkPair(p4);

      TestPair p5{std::move(p3)};
      TestPair p6{std::move(p4)};
      checkPair(p5);
      checkPair(p6);
    }
  }
  EXPECT_EQ(alive, 0);
}

struct Convertible1 {
  int i;
};
struct Convertible2 {
  explicit Convertible2(Convertible1 c) : i{c.i} {}
  int i;
};

TEST(TinyVector, Convertible) {
  TinyVector<Convertible2> vec{Convertible1{1}, Convertible1{2}, Convertible1{3}};

  Convertible1 c{4};
  vec.push_back(c);
  vec.push_back(Convertible1{5});

  EXPECT_EQ(vec.size(), 5);
  for (size_t i = 0; i < vec.size(); ++i) {
    EXPECT_EQ(vec[i].i, i + 1);
  }
}

TEST(TinyVector, Large) {
  TinyVector<int> v;
  for (size_t i = 0; i < 100000; ++i) v.emplace_back(i);

  ASSERT_EQ(v.size(), 100000);
  for (size_t i = 0; i < 100000; ++i) EXPECT_EQ(v[i], i);

  TinyVector<int> v2;
  v2.resize(100000);
  EXPECT_EQ(v2.size(), 100000);
}

}
