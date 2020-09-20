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

#include "hphp/runtime/vm/jit/id-set.h"

#include <folly/Lazy.h>
#include <folly/portability/GTest.h>

namespace HPHP { namespace jit {

namespace {

struct Foo {
  explicit Foo(uint32_t id) : m_id{id} {}
  uint32_t id() const { return m_id; }
  uint32_t m_id;

  bool operator<(const Foo& o) const {
    return m_id < o.m_id;
  }
  bool operator==(const Foo& o) const {
    return m_id == o.m_id;
  }
};

using FooVector = std::vector<Foo>;
using FooSet = IdSet<Foo>;

void PrintTo(const Foo& f, std::ostream* os) {
  *os << f.id();
}

void PrintTo(const FooVector& v, std::ostream* os) {
  *os << '{';
  auto first = true;
  for (auto const& foo : v) {
    if (!first) *os << ", ";
    first = false;
    *os << foo.id();
  }
  *os << '}';
}

FooVector toFooVector(const FooSet& s) {
  FooVector ret;
  s.forEach([&] (uint32_t id) { ret.emplace_back(Foo{id}); });
  return ret;
}

bool inFooVector(const FooVector& v, const Foo& f) {
  return std::find(v.begin(), v.end(), f) != v.end();
}

void removeFromFooVector(FooVector& v, const Foo& f) {
  v.erase(std::remove(v.begin(), v.end(), f), v.end());
}

FooVector unionFooVector(const FooVector& v1, const FooVector& v2) {
  auto out = v1;
  out.insert(out.end(), v2.begin(), v2.end());
  std::sort(out.begin(), out.end());
  out.erase(std::unique(out.begin(), out.end()), out.end());
  return out;
}

auto const foos = folly::lazy(
  [] {
    return FooVector{
      Foo{0},
      Foo{1},
      Foo{FooSet::kBlockSize - 1},
      Foo{FooSet::kBlockSize},
      Foo{FooSet::kBlockSize + 1},
      Foo{FooSet::kBlockSize * 2 - 1},
      Foo{FooSet::kBlockSize * 2},
      Foo{FooSet::kBlockSize * 2 + 1},
      Foo{FooSet::kBlockSize * 5 + 27}
    };
  }
);

auto const membershipFoos = folly::lazy(
  [] {
    auto out = foos();
    out.emplace_back(Foo{2});
    out.emplace_back(FooSet::kBlockSize + 5);
    out.emplace_back(FooSet::kBlockSize * 4 - 7);
    out.emplace_back(1000001);
    return out;
  }
);

auto const eraseOpFoos = folly::lazy(
  [] {
    return FooVector{
      Foo{1},
      Foo{FooSet::kBlockSize + 1},
      Foo{FooSet::kBlockSize * 2 + 1},
      Foo{FooSet::kBlockSize * 3 + 1},
      Foo{FooSet::kBlockSize * 4 + 1}
    };
  }
);

auto const eraseOpFoosWithZero = folly::lazy(
  [] {
    auto out = eraseOpFoos();
    out.emplace_back(Foo{0});
    return out;
  }
);

auto const eraseOpFoosWithZeroPairs = folly::lazy(
  [] {
    std::vector<std::pair<Foo, Foo>> out;
    for (auto const& f1 : eraseOpFoosWithZero()) {
      for (auto const& f2 : eraseOpFoosWithZero()) {
        out.emplace_back(f1, f2);
      }
    }
    return out;
  }
);

std::vector<FooVector> makeSubsets(const FooVector& v) {
  std::vector<FooVector> out;

  uint64_t selected = 0;
  while (selected < (1ULL << v.size())) {
    FooVector current;
    for (size_t i = 0; i < v.size(); ++i) {
      if (selected & (1ULL << i)) current.emplace_back(v[i]);
    }
    out.emplace_back(std::move(current));
    ++selected;
  }

  return out;
}

void sortSubsets(std::vector<FooVector>& subsets) {
  for (auto& subset : subsets) std::sort(subset.begin(), subset.end());
}

std::vector<FooVector> permutations(const std::vector<FooVector>& sorted) {
  std::vector<FooVector> out;
  for (auto subset : sorted) {
    do {
      out.emplace_back(subset);
    } while (std::next_permutation(subset.begin(), subset.end()));
  }
  return out;
}

auto const subsets =
  folly::lazy([] { return makeSubsets(foos()); });
auto const eraseOpSubsets =
  folly::lazy([] { return makeSubsets(eraseOpFoos()); });

auto const sortedSubsets = folly::lazy(
  [] {
    auto s = subsets();
    sortSubsets(s);
    return s;
  }
);

auto const sortedEraseOpSubsets = folly::lazy(
  [] {
    auto s = eraseOpSubsets();
    sortSubsets(s);
    return s;
  }
);

auto const all =
  folly::lazy([] { return permutations(sortedSubsets()); });
auto const eraseOpAll =
  folly::lazy([] { return permutations(sortedEraseOpSubsets()); });

}

TEST(IdSet, Add) {
  for (auto values : all()) {
    FooSet s;
    for (auto const& value : values) s.add(value);

    EXPECT_EQ(s.none(), values.empty());
    for (auto const& foo : membershipFoos()) {
      EXPECT_EQ(s[foo], inFooVector(values, foo));
    }
    std::sort(values.begin(), values.end());
    EXPECT_EQ(toFooVector(s), values);
  }
}

TEST(IdSet, Erase) {
  for (auto values : sortedSubsets()) {
    FooSet s;
    for (auto const& value : values) s.add(value);

    for (auto const& foo : membershipFoos()) {
      s.erase(foo);
      removeFromFooVector(values, foo);
      EXPECT_FALSE(s[foo]);
      EXPECT_EQ(toFooVector(s), values);
      for (auto const& foo : membershipFoos()) {
        EXPECT_EQ(s[foo], inFooVector(values, foo));
      }
    }

    EXPECT_TRUE(s.none());
  }
}

TEST(IdSet, Clear) {
  for (auto const& values : sortedSubsets()) {
    FooSet s;
    for (auto const& value : values) s.add(value);
    s.clear();

    EXPECT_TRUE(s.none());
    for (auto const& foo : membershipFoos()) EXPECT_FALSE(s[foo]);
    EXPECT_EQ(toFooVector(s), FooVector{});
  }
}

TEST(IdSet, AddDuplicate) {
  for (auto const& values : sortedSubsets()) {
    FooSet s;
    for (auto const& value : values) s.add(value);

    for (auto const& foo : foos()) {
      if (!inFooVector(values, foo)) continue;
      EXPECT_TRUE(s[foo]);
      s.add(foo);
      EXPECT_TRUE(s[foo]);
      EXPECT_EQ(toFooVector(s), values);
      for (auto const& foo : membershipFoos()) {
        EXPECT_EQ(s[foo], inFooVector(values, foo));
      }
    }
  }
}

TEST(IdSet, Copy) {
  for (auto const& values : sortedSubsets()) {
    FooSet s;
    for (auto const& value : values) s.add(value);

    auto const s2 = s;
    EXPECT_EQ(toFooVector(s), toFooVector(s2));
    EXPECT_EQ(toFooVector(s2), values);
    for (auto const& foo : membershipFoos()) {
      EXPECT_EQ(s[foo], s2[foo]);
      EXPECT_EQ(s2[foo], inFooVector(values, foo));
    }
    EXPECT_EQ(s.none(), s2.none());
    EXPECT_EQ(values.empty(), s2.none());
  }
}

TEST(IdSet, MoveCopy) {
  for (auto const& values : sortedSubsets()) {
    FooSet s;
    for (auto const& value : values) s.add(value);

    auto const orig = s;
    auto const s2 = std::move(s);
    EXPECT_EQ(toFooVector(orig), toFooVector(s2));
    EXPECT_EQ(toFooVector(s2), values);
    for (auto const& foo : membershipFoos()) {
      EXPECT_EQ(orig[foo], s2[foo]);
      EXPECT_EQ(s2[foo], inFooVector(values, foo));
    }
    EXPECT_EQ(orig.none(), s2.none());
    EXPECT_EQ(values.empty(), s2.none());
  }
}

TEST(IdSet, Assign) {
  for (auto const& values1 : sortedSubsets()) {
    for (auto const& values2 : sortedSubsets()) {
      FooSet s1, s2;
      for (auto const& value : values1) s1.add(value);
      for (auto const& value : values2) s2.add(value);
      s1 = s2;

      EXPECT_EQ(toFooVector(s1), toFooVector(s2));
      EXPECT_EQ(toFooVector(s1), values2);
      for (auto const& foo : membershipFoos()) {
        EXPECT_EQ(s1[foo], s2[foo]);
        EXPECT_EQ(s1[foo], inFooVector(values2, foo));
      }
      EXPECT_EQ(s1.none(), s2.none());
      EXPECT_EQ(values2.empty(), s1.none());
    }
  }
}

TEST(IdSet, MoveAssign) {
  for (auto const& values1 : sortedSubsets()) {
    for (auto const& values2 : sortedSubsets()) {
      FooSet s1, s2;
      for (auto const& value : values1) s1.add(value);
      for (auto const& value : values2) s2.add(value);
      auto const s3 = s2;
      s1 = std::move(s2);

      EXPECT_EQ(toFooVector(s1), toFooVector(s3));
      EXPECT_EQ(toFooVector(s1), values2);
      for (auto const& foo : membershipFoos()) {
        EXPECT_EQ(s1[foo], s3[foo]);
        EXPECT_EQ(s1[foo], inFooVector(values2, foo));
      }
      EXPECT_EQ(s1.none(), s3.none());
      EXPECT_EQ(values2.empty(), s1.none());
    }
  }
}

TEST(IdSet, Swap) {
  for (auto const& values1 : sortedSubsets()) {
    for (auto const& values2 : sortedSubsets()) {
      FooSet s1, s2;
      for (auto const& value : values1) s1.add(value);
      for (auto const& value : values2) s2.add(value);
      s1.swap(s2);

      EXPECT_EQ(toFooVector(s1), values2);
      EXPECT_EQ(toFooVector(s2), values1);
      for (auto const& foo : membershipFoos()) {
        EXPECT_EQ(s1[foo], inFooVector(values2, foo));
        EXPECT_EQ(s2[foo], inFooVector(values1, foo));
      }
      EXPECT_EQ(values2.empty(), s1.none());
      EXPECT_EQ(values1.empty(), s2.none());
    }
  }
}

TEST(IdSet, Union) {
  for (auto const& values1 : sortedSubsets()) {
    for (auto const& values2 : sortedSubsets()) {
      FooSet s1, s2;
      for (auto const& value : values1) s1.add(value);
      for (auto const& value : values2) s2.add(value);
      auto s3 = s1;
      s3 |= s2;
      auto const values3 = unionFooVector(values1, values2);
      EXPECT_EQ(toFooVector(s3), values3);
      for (auto const& foo : membershipFoos()) {
        EXPECT_EQ(s1[foo] || s2[foo], s3[foo]);
        EXPECT_EQ(s3[foo], inFooVector(values3, foo));
      }
      EXPECT_EQ(s1.none() && s2.none(), s3.none());
      EXPECT_EQ(values3.empty(), s3.none());
    }
  }
}

namespace {

template <typename F> void forEachEraseOpCase(F&& f) {
  for (auto const& values : sortedEraseOpSubsets()) {
    for (auto const& p : eraseOpFoosWithZeroPairs()) {
      FooSet s;
      for (auto const& value : values) s.add(value);
      s.erase(p.first);
      s.erase(p.second);
      auto v = values;
      removeFromFooVector(v, p.first);
      removeFromFooVector(v, p.second);
      f(s, v);
    }
  }
}

template <typename F> void forEachEraseOpPair(F&& f) {
  for (auto const& values1 : sortedEraseOpSubsets()) {
    for (auto const& values2 : sortedEraseOpSubsets()) {
      for (auto const& erase1 : eraseOpFoosWithZeroPairs()) {
        for (auto const& erase2 : eraseOpFoosWithZeroPairs()) {
          FooSet s1, s2;
          for (auto const& value : values1) s1.add(value);
          for (auto const& value : values2) s2.add(value);
          s1.erase(erase1.first);
          s1.erase(erase1.second);
          s2.erase(erase2.first);
          s2.erase(erase2.second);

          auto v1 = values1;
          auto v2 = values2;
          removeFromFooVector(v1, erase1.first);
          removeFromFooVector(v1, erase1.second);
          removeFromFooVector(v2, erase2.first);
          removeFromFooVector(v2, erase2.second);
          f(s1, v1, s2, v2);
        }
      }
    }
  }
}

}

TEST(IdSet, EraseCopy) {
  forEachEraseOpCase(
    [&] (const FooSet& s1, const FooVector& v1) {
      auto const s2 = s1;
      EXPECT_EQ(toFooVector(s1), toFooVector(s2));
      EXPECT_EQ(toFooVector(s2), v1);
      for (auto const& foo : eraseOpFoos()) {
        EXPECT_EQ(s1[foo], s2[foo]);
        EXPECT_EQ(s2[foo], inFooVector(v1, foo));
      }
      EXPECT_EQ(s1.none(), s2.none());
      EXPECT_EQ(v1.empty(), s2.none());
    }
  );
}

TEST(IdSet, EraseAssign) {
  forEachEraseOpPair(
    [&] (FooSet& s1, const FooVector&,
         const FooSet& s2, const FooVector& v2) {
      s1 = s2;
      EXPECT_EQ(toFooVector(s1), toFooVector(s2));
      EXPECT_EQ(toFooVector(s1), v2);
      for (auto const& foo : eraseOpFoos()) {
        EXPECT_EQ(s1[foo], s2[foo]);
        EXPECT_EQ(s1[foo], inFooVector(v2, foo));
      }
      EXPECT_EQ(s1.none(), s2.none());
      EXPECT_EQ(v2.empty(), s1.none());
    }
  );
}

TEST(IdSet, EraseUnion) {
  forEachEraseOpPair(
    [&] (FooSet& s1, const FooVector& v1,
         const FooSet& s2, const FooVector& v2) {
      auto const v3 = unionFooVector(v1, v2);
      auto const copy = s1;
      s1 |= s2;
      EXPECT_EQ(toFooVector(s1), v3);
      for (auto const& foo : eraseOpFoos()) {
        EXPECT_EQ(copy[foo] || s2[foo], s1[foo]);
        EXPECT_EQ(s1[foo], inFooVector(v3, foo));
      }
      EXPECT_EQ(copy.none() && s2.none(), s1.none());
      EXPECT_EQ(v3.empty(), s1.none());
    }
  );
}

}}
