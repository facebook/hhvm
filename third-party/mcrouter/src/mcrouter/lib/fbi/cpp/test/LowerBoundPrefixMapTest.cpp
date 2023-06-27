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

void testBasicContents(const LowerBoundPrefixMap<int>& lbMap) {
  ASSERT_EQ(4, *lbMap.findPrefix("b"));
  ASSERT_EQ(3, *lbMap.findPrefix("bc"));
  ASSERT_EQ(4, *lbMap.findPrefix("be"));
  ASSERT_EQ(3, *lbMap.findPrefix("bcd"));
  ASSERT_EQ(2, *lbMap.findPrefix("e"));
  ASSERT_FALSE(lbMap.findPrefix("da"));
  ASSERT_FALSE(lbMap.findPrefix("a"));
  ASSERT_FALSE(lbMap.findPrefix("f"));
  ASSERT_EQ(2, *lbMap.findPrefix("ed:"));
  ASSERT_EQ(1, *lbMap.findPrefix("ef:a"));
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
    ASSERT_EQ(static_cast<int>(i), *lbMap.findPrefix("abc"));
  }
}

TEST(LowerBoundPrefixMapTest, EmptyKey) {
  std::vector<KeyValue> kv = {
      KeyValue{"", 1},
      KeyValue{"z", 2},
  };
  LowerBoundPrefixMap<int> lbMap{kv};
  for (const auto& [k, v] : k2v) {
    ASSERT_EQ(1, *lbMap.findPrefix(k));
  }
}

TEST(LowerBoundPrefixMapTest, VeryLongStringCrash) {
  std::vector<KeyValue> kv = {
      KeyValue{std::string(10000u, 'a'), 0},
      KeyValue{"", 0},
      KeyValue{"", 0},
  };
  LowerBoundPrefixMap<int> lbMap{kv};
}

TEST(LowerBoundPrefixMapTest, MoveOnly) {
  LowerBoundPrefixMap<std::unique_ptr<int>>::Builder builder;
  builder.insert({"abc", std::make_unique<int>(1)});
  auto lbmap = std::move(builder).build();
  ASSERT_TRUE(lbmap.findPrefix("abc"));
}

} // namespace
} // namespace facebook::memcache
