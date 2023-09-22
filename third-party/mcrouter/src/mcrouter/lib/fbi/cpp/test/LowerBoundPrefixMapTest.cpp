/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "mcrouter/lib/fbi/cpp/LowerBoundPrefixMap.h"

#include <gtest/gtest.h>
#include <memory>

namespace facebook::memcache {
namespace {

// Most of the heavy lifting for this test is done by a Fuzzer

using KeyValue = std::pair<std::string, int>;
const std::vector<KeyValue> k2v = {
    KeyValue{"b", 4},
    KeyValue{"bc", 3},
    KeyValue{"e", 2},
    KeyValue{"ef:", 1}};

std::vector<KeyValue> toKV(auto f, auto l) {
  std::vector<KeyValue> res;
  res.reserve(l - f);
  while (f != l) {
    res.emplace_back(std::string(f->key()), f->value());
    ++f;
  }
  return res;
}

void testBasicContentsImpl(auto& lbMap) {
  ASSERT_EQ(k2v.size(), lbMap.size());

  ASSERT_EQ("b", lbMap.findPrefix("b")->key());
  ASSERT_EQ(4, lbMap.findPrefix("b")->value());
  ASSERT_FALSE(lbMap.findPrefix("b")->previousPrefix());

  ASSERT_EQ("bc", lbMap.findPrefix("bc")->key());
  ASSERT_EQ(3, lbMap.findPrefix("bc")->value());
  ASSERT_EQ("b", lbMap.findPrefix("bc")->previousPrefix()->key());

  ASSERT_EQ("b", lbMap.findPrefix("be")->key());
  ASSERT_EQ(4, lbMap.findPrefix("be")->value());
  ASSERT_FALSE(lbMap.findPrefix("be")->previousPrefix());

  ASSERT_EQ("bc", lbMap.findPrefix("bcd")->key());
  ASSERT_EQ(3, lbMap.findPrefix("bcd")->value());
  ASSERT_EQ("b", lbMap.findPrefix("bcd")->previousPrefix()->key());

  ASSERT_EQ("e", lbMap.findPrefix("e")->key());
  ASSERT_EQ(2, lbMap.findPrefix("e")->value());
  ASSERT_FALSE(lbMap.findPrefix("e")->previousPrefix());

  ASSERT_EQ(lbMap.end(), lbMap.findPrefix("da"));
  ASSERT_EQ(lbMap.end(), lbMap.findPrefix("a"));
  ASSERT_EQ(lbMap.end(), lbMap.findPrefix("f"));

  ASSERT_EQ("e", lbMap.findPrefix("ed")->key());
  ASSERT_EQ(2, lbMap.findPrefix("ed")->value());
  ASSERT_FALSE(lbMap.findPrefix("ed")->previousPrefix());

  ASSERT_EQ("ef:", lbMap.findPrefix("ef:a")->key());
  ASSERT_EQ(1, lbMap.findPrefix("ef:a")->value());
  ASSERT_EQ("e", lbMap.findPrefix("ef:a")->previousPrefix()->key());

  ASSERT_EQ(k2v, toKV(lbMap.begin(), lbMap.end()));
  ASSERT_EQ(k2v, toKV(lbMap.cbegin(), lbMap.cend()));

  std::vector<KeyValue> k2vReverse = k2v;
  std::reverse(k2vReverse.begin(), k2vReverse.end());
  ASSERT_EQ(k2vReverse, toKV(lbMap.rbegin(), lbMap.rend()));
  ASSERT_EQ(k2vReverse, toKV(lbMap.crbegin(), lbMap.crend()));
}

void testBasicContents(auto& lbMap) {
  testBasicContentsImpl(lbMap);
  testBasicContentsImpl(std::as_const(lbMap));
}

TEST(LowerBoundPrefixMapTest, SmokeTest) {
  const LowerBoundPrefixMap<int> lowerBoundMap{k2v};
  testBasicContents(lowerBoundMap);

  {
    auto copy = lowerBoundMap;
    testBasicContents(copy);

    auto moved = std::move(copy);
    testBasicContents(moved);
  }

  {
    LowerBoundPrefixMap<int> assigned;
    assigned = lowerBoundMap;
    testBasicContents(assigned);

    LowerBoundPrefixMap<int> moveAssigned;

    moveAssigned = std::move(assigned);
    testBasicContents(moveAssigned);
  }
}

TEST(LowerBoundPrefixMapTest, Builder) {
  LowerBoundPrefixMap<int>::Builder builder;

  builder.reserve(k2v.size());
  for (const auto& [k, v] : k2v) {
    builder.insert({k, v});
  }

  auto prefixMap = std::move(builder).build();
  testBasicContents(prefixMap);
}

TEST(LowerBoundPrefixMapTest, DuplicateKeyReturnsLast) {
  for (std::size_t i = 1; i != 100; ++i) {
    std::vector<KeyValue> kv(i, KeyValue("abc", 0));
    for (int j = 0; auto& x : kv) {
      x.second = ++j;
    }
    LowerBoundPrefixMap<int> lbMap{kv};
    ASSERT_EQ(static_cast<int>(i), lbMap.findPrefix("abc")->value());
  }
}

TEST(LowerBoundPrefixMapTest, EmptyKey) {
  std::vector<KeyValue> kv = {
      KeyValue{"", 1},
      KeyValue{"z", 2},
  };
  LowerBoundPrefixMap<int> lbMap{kv};
  for (const auto& [k, v] : k2v) {
    ASSERT_EQ(1, lbMap.findPrefix(k)->value());
  }
}

TEST(LowerBoundPrefixMapTest, VeryLongStringCrash) {
  std::vector<KeyValue> kv = {
      KeyValue{std::string(10000u, 'a'), 0},
      KeyValue{"", 0},
      KeyValue{"", 0},
  };
  LowerBoundPrefixMap<int> lbMap{std::move(kv)};
}

TEST(LowerBoundPrefixMapTest, MoveOnly) {
  LowerBoundPrefixMap<std::unique_ptr<int>>::Builder builder;
  builder.insert({"abc", std::make_unique<int>(1)});
  auto lbmap = std::move(builder).build();
  ASSERT_NE(lbmap.end(), lbmap.findPrefix("abc"));
}

TEST(LowerBoundPrefixMapTest, OverrideValues) {
  std::vector<KeyValue> kv = {
      KeyValue{"", 0},
      KeyValue{"1", 0},
      KeyValue{"11", 0},
      KeyValue{"a", 0},
      KeyValue{"aa", 0},
      KeyValue{"c", 0},
  };
  static_assert('1' < 'a');

  LowerBoundPrefixMap<int> lbMap{std::move(kv)};

  std::optional<LowerBoundPrefixMap<int>::reference> ref =
      *lbMap.findPrefix("aa");

  for (; ref; ref = ref->previousPrefix()) {
    ref->value()++;
  }
  ref = *lbMap.findPrefix("11");
  for (; ref; ref = ref->previousPrefix()) {
    ref->value()++;
  }

  std::vector<KeyValue> expected = {
      KeyValue{"", 2},
      KeyValue{"1", 1},
      KeyValue{"11", 1},
      KeyValue{"a", 1},
      KeyValue{"aa", 1},
      KeyValue{"c", 0},
  };

  ASSERT_EQ(expected.size(), lbMap.size());

  auto it = lbMap.begin();

  for (std::size_t i = 0; i != lbMap.size(); ++i) {
    ASSERT_EQ(expected[i].first, it[i].key());
    ASSERT_EQ(expected[i].second, it[i].value());
  }
}

TEST(LowerBoundPrefixMapTest, EmptyMap) {
  LowerBoundPrefixMap<int> lbMap;
  ASSERT_EQ(lbMap.findPrefix("a"), lbMap.end());
  ASSERT_EQ(lbMap.size(), 0);
  ASSERT_TRUE(lbMap.empty());
}

} // namespace
} // namespace facebook::memcache
