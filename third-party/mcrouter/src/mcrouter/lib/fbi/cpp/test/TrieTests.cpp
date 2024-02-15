/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <gtest/gtest.h>

#include <folly/init/Init.h>

#include "mcrouter/lib/fbi/cpp/Trie.h"
#include "mcrouter/lib/fbi/cpp/util.h"

using facebook::memcache::Trie;

namespace {

const int kNumRandKeys = 1 << 15;

std::string keys[kNumRandKeys];
Trie<int> randTrie;

void prepareRand() {
  srand(1234);
  for (long i = 0; i < kNumRandKeys; ++i) {
    keys[i] = facebook::memcache::randomString(10, 20);

    randTrie.emplace(keys[i], i);
  }
}

} // anonymous namespace

TEST(Trie, SanityTest) {
  Trie<void*> trie;

  std::string k[] = {"hello", "world", "!helloo~", ""};

  // Get Misses
  for (auto& str : k) {
    EXPECT_TRUE(trie.find(str) == trie.end());
  }

  // Sets
  for (auto& str : k) {
    trie.emplace(str, &str);
  }

  // Get Hits
  for (auto& str : k) {
    EXPECT_TRUE(trie.find(str)->second == &str);
  }

  // Get out of range
  EXPECT_TRUE(trie.find(" ") == trie.end());
  EXPECT_TRUE(trie.findPrefix(" ")->second == &k[3]);
}

TEST(Trie, PrefixTest) {
  Trie<void*> trie;

  std::string k[] = {"a", "ab", "abc", "abd"};

  std::string h[] = {"a", "ab111", "abc", "abdasdfkljasdklfjsdaklfjsdflkj"};
  int nkh = sizeof(h) / sizeof(h[0]);

  std::string m[] = {"b", "cd", ""};

  // Get Misses
  for (auto& str : h) {
    EXPECT_TRUE(trie.find(str) == trie.end());
  }

  // Sets
  for (auto& str : k) {
    trie.emplace(str, &str);
  }

  // Get Hits
  for (int i = 0; i < nkh; ++i) {
    EXPECT_TRUE(trie.findPrefix(h[i])->second == &k[i]);
  }

  // Get Misses
  for (auto& str : m) {
    EXPECT_TRUE(trie.findPrefix(str) == trie.end());
  }
}

TEST(Trie, PrefixTest2) {
  Trie<int> trie;

  trie.emplace("a", 1);
  trie.emplace("abc", 2);

  EXPECT_TRUE(trie.findPrefix("ab")->second == 1);
}

TEST(Trie, IteratorTest) {
  Trie<int> trie;

  std::string k[] = {"a", "ab", "abc", "abd", "baa", "bab", "baac", ""};
  int nk = sizeof(k) / sizeof(k[0]);

  // Sets
  for (auto i = 0; i < nk; ++i) {
    trie.emplace(k[i], i);
  }

  std::vector<int> count(nk, 0);
  for (auto& it : trie) {
    ++count[it.second];
  }
  for (auto i = 0; i < nk; ++i) {
    EXPECT_TRUE(1 == count[i]);
  }

  std::vector<int> count2(nk + 1, 0);
  for (auto& it : trie) {
    ++it.second;
  }
  for (auto& it : trie) {
    ++count2[it.second];
  }
  for (auto i = 0; i < nk; ++i) {
    EXPECT_TRUE(1 == count2[i + 1]);
  }
}

TEST(Trie, ConstructorTest) {
  Trie<int> trie;

  std::string k[] = {"", "a", "ab", "abc", "abd", "baa", "baac", "bab"};
  int nk = sizeof(k) / sizeof(k[0]);

  // Sets
  for (auto i = 0; i < nk; ++i) {
    trie.emplace(k[i], i);
  }

  // copy constructor
  {
    Trie<int> trie2{trie};

    for (auto i = 0; i < nk; ++i) {
      EXPECT_TRUE(trie.find(k[i])->second == i);
      EXPECT_TRUE(trie2.find(k[i])->second == i);
      EXPECT_TRUE(trie.find(k[i]) != trie2.find(k[i]));
    }
  }

  // move constructor
  {
    Trie<int> trie2{trie};
    Trie<int> trie3{std::move(trie2)};
    auto it = trie3.begin();
    for (auto i = 0; i < nk; ++i) {
      EXPECT_EQ(trie3.find(k[i])->second, i);
      EXPECT_EQ(it->second, i);
      ++it;
    }
    EXPECT_EQ(it, trie3.end());
  }

  // copy assignment
  {
    Trie<int> trie2;
    trie2.emplace("z", 1);
    trie2 = trie;
    for (auto i = 0; i < nk; ++i) {
      EXPECT_TRUE(trie.find(k[i])->second == i);
      EXPECT_TRUE(trie2.find(k[i])->second == i);
      EXPECT_TRUE(trie.find(k[i]) != trie2.find(k[i]));
    }
    EXPECT_TRUE(trie2.find("z") == trie2.end());

    // clear
    trie2.clear();
    for (auto i = 0; i < nk; ++i) {
      EXPECT_TRUE(trie2.find(k[i]) == trie2.end());
    }
  }

  // move assignment
  {
    Trie<int> trie2;
    trie2.emplace("z", 1);
    trie2 = std::move(trie);
    for (auto i = 0; i < nk; ++i) {
      EXPECT_TRUE(trie2.find(k[i])->second == i);
    }
    EXPECT_TRUE(trie2.find("z") == trie2.end());
  }
}

TEST(Trie, RandTestGet) {
  std::unordered_map<std::string, int> map;
  for (int i = 0; i < kNumRandKeys; ++i) {
    map[keys[i]] = i;
  }

  for (int i = 0; i < (1 << 16); ++i) {
    auto s = facebook::memcache::randomString(1, 20);
    auto it = map.find(s);
    if (it == map.end()) {
      EXPECT_TRUE(randTrie.find(s) == randTrie.end());
    } else {
      EXPECT_TRUE(*randTrie.find(s) == *it);
    }
  }
}

TEST(Trie, RandTestGetPrefix) {
  std::unordered_map<std::string, int> map;
  for (int i = 0; i < kNumRandKeys; ++i) {
    map[keys[i]] = i;
  }

  for (int i = 0; i < (1 << 16); ++i) {
    auto s = facebook::memcache::randomString(1, 20);
    int need = -1;
    for (int j = s.length(); j >= 0; --j) {
      auto it = map.find(s.substr(0, j));
      if (it != map.end()) {
        need = it->second;
        break;
      }
    }
    if (need == -1) {
      ASSERT_TRUE(randTrie.findPrefix(s) == randTrie.end());
    } else {
      ASSERT_TRUE(randTrie.findPrefix(s)->second == need);
    }
  }
}

TEST(Trie, RandTestIter) {
  std::map<std::string, int> map;
  for (int i = 0; i < kNumRandKeys; ++i) {
    map[keys[i]] = i;
  }

  auto itMap = map.begin();
  auto itTrie = randTrie.begin();
  auto constItTrie = randTrie.cbegin();
  while (itMap != map.end()) {
    ASSERT_TRUE(*itMap == *itTrie);
    ASSERT_TRUE(*itMap == *constItTrie);
    ++itMap;
    ++itTrie;
    ++constItTrie;
  }
  EXPECT_TRUE(itTrie == randTrie.end());
  EXPECT_TRUE(constItTrie == randTrie.cend());
}

TEST(Trie, Const) {
  Trie<int> t;
  t.emplace("a", 1);
  const Trie<int> trie(t);
  EXPECT_TRUE(trie.find("a")->second == 1);
  EXPECT_TRUE(trie.findPrefix("aa")->second == 1);
  EXPECT_TRUE(trie.begin()->second == 1);
  EXPECT_TRUE(trie.cbegin()->second == 1);
}

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  folly::init(&argc, &argv);
  prepareRand();
  return RUN_ALL_TESTS();
}
