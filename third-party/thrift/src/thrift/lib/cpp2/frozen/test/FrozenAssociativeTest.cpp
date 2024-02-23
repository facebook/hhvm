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

#include <folly/portability/GTest.h>

#include <folly/Conv.h>
#include <folly/Optional.h>
#include <folly/container/F14Map.h>
#include <folly/container/F14Set.h>
#include <thrift/lib/cpp2/frozen/Frozen.h>
#include <thrift/lib/cpp2/frozen/FrozenUtil.h>
#include <thrift/lib/cpp2/frozen/test/gen-cpp2/Example_layouts.h>
#include <thrift/lib/cpp2/frozen/test/gen-cpp2/Example_types.h>
#include <thrift/lib/cpp2/protocol/DebugProtocol.h>

using namespace apache::thrift::frozen;
using namespace apache::thrift::test;

std::map<int, int> osquares{{1, 1}, {2, 4}, {3, 9}, {4, 16}};
std::unordered_map<int, int> usquares{{1, 1}, {2, 4}, {3, 9}, {4, 16}};

TEST(FrozenPair, Basic) {
  std::pair<int, int> ip{3, 7};
  std::pair<std::string, std::string> sp{"three", "seven"};
  std::pair<std::vector<int>, std::vector<int>> vip{{1, 3, 5}, {2, 4, 8}};
  auto fip = freeze(ip);
  auto fsp = freeze(sp);
  auto fvip = freeze(vip);
  EXPECT_EQ(ip, fip.thaw());
  EXPECT_EQ(sp, fsp.thaw());
  EXPECT_EQ(vip, fvip.thaw());

  EXPECT_EQ(sp.first, fsp.first());
  EXPECT_EQ(sp.second, fsp.second());
}

template <typename Map>
void checkBasicFrozenMap(const Map& map) {
  auto fmap = freeze(map);
  EXPECT_EQ(map.at(3), fmap.at(3));
  EXPECT_EQ(map.find(3)->second, fmap.find(3)->second());
  EXPECT_TRUE(fmap.count(2));
  EXPECT_FALSE(fmap.count(8));
  auto thawed = fmap.thaw();
  EXPECT_EQ(map.at(3), thawed.at(3));
  EXPECT_EQ(map.find(3)->second, thawed.find(3)->second);
  EXPECT_TRUE(thawed.count(2));
  EXPECT_FALSE(thawed.count(8));
}

template <typename T>
struct FrozenMapBasic : public ::testing::Test {};
TYPED_TEST_SUITE_P(FrozenMapBasic);

TYPED_TEST_P(FrozenMapBasic, Basic) {
  checkBasicFrozenMap(TypeParam{osquares.begin(), osquares.end()});
}

REGISTER_TYPED_TEST_SUITE_P(FrozenMapBasic, Basic);
using FrozenMapTypes = testing::Types<
    std::map<int, int>,
    folly::sorted_vector_map<int, int>,
    folly::heap_vector_map<int, int>,
    folly::small_heap_vector_map<int, int>>;
INSTANTIATE_TYPED_TEST_SUITE_P(
    FrozenAssociativeTest, FrozenMapBasic, FrozenMapTypes);

TEST(FrozenMap, BasicNonDefaultComparison) {
  std::map<int, int, std::greater<int>> map(osquares.begin(), osquares.end());
  checkBasicFrozenMap(map);
}

TEST(FrozenMap, NonSortValue) {
  std::map<int, std::unordered_map<int, int>> mult{
      {1, {{1, 1}, {2, 2}}}, {2, {{1, 2}, {2, 4}}}};
  auto fmap = freeze(mult);
  EXPECT_EQ(fmap.at(1).at(1), 1);
  EXPECT_EQ(fmap.find(2)->second().find(2)->second(), 4);
  EXPECT_FALSE(fmap.count(3));
  EXPECT_TRUE(fmap.count(1));
  EXPECT_TRUE(fmap.at(2).count(1));
  EXPECT_FALSE(fmap.at(2).count(3));
}

TEST(FrozenMap, Iteration) {
  auto& map = osquares;
  auto fmap = freeze(map);
  int sumK = 0, sumV = 0;
  for (auto entry : fmap) {
    sumK += entry.first();
    sumV += entry.second();
  }
  EXPECT_EQ(10, sumK);
  EXPECT_EQ(30, sumV);
}

namespace {
template <typename HashMap>
void testFrozenHashMapBasic() {
  HashMap map{{1, 1}, {2, 4}, {3, 9}, {4, 16}};

  auto fmap = freeze(map);
  EXPECT_EQ(map.at(3), fmap.at(3));
  EXPECT_EQ(map.find(3)->second, fmap.find(3)->second());
  EXPECT_TRUE(fmap.count(2));
  EXPECT_FALSE(fmap.count(8));
}
} // namespace

TEST(FrozenHashMap, Basic) {
  testFrozenHashMapBasic<std::unordered_map<int, int>>();
  testFrozenHashMapBasic<folly::F14NodeMap<int, int>>();
  testFrozenHashMapBasic<folly::F14ValueMap<int, int>>();
  testFrozenHashMapBasic<folly::F14VectorMap<int, int>>();
  testFrozenHashMapBasic<folly::F14FastMap<int, int>>();
}

TEST(FrozenHashMap, Iteration) {
  auto& map = usquares;
  auto fmap = freeze(map);
  int sumK = 0, sumV = 0;
  for (auto entry : fmap) {
    sumK += entry.first();
    sumV += entry.second();
  }
  EXPECT_EQ(10, sumK);
  EXPECT_EQ(30, sumV);
}

TEST(FrozenMap, Strings) {
  std::map<std::string, std::string> map;
  for (uint32_t i = 0; i < 100; ++i) {
    map[folly::to<std::string>(i)] = folly::to<std::string>(sqrt(i));
  }
  auto fmap = freeze(map);
  EXPECT_EQ(map.at("4"), fmap.at("4"));
  EXPECT_EQ(map.find("4")->second, fmap.find("4")->second());
  EXPECT_TRUE(fmap.count("8"));
  EXPECT_FALSE(fmap.count("z"));
  EXPECT_EQ("2", fmap.lower_bound("4")->second());
  EXPECT_EQ("5", fmap.lower_bound("24a")->second());
  EXPECT_EQ("7", fmap.upper_bound("48x")->second());
  EXPECT_EQ("2", fmap.upper_bound("39x")->second());
  auto eq = fmap.equal_range("24a");
  EXPECT_EQ(0, eq.second - eq.first);
  EXPECT_EQ("5", eq.first->second());
}

TEST(FrozenHashMap, Strings) {
  std::unordered_map<std::string, std::string> map;
  for (uint32_t i = 0; i < 100; ++i) {
    map[folly::to<std::string>(i)] = folly::to<std::string>(sqrt(i));
  }
  auto fmap = freeze(map);
  EXPECT_EQ(map.at("4"), fmap.at("4"));
  EXPECT_EQ(map.find("4")->second, fmap.find("4")->second());
  EXPECT_TRUE(fmap.count("8"));
  EXPECT_FALSE(fmap.count("z"));
  auto eq = fmap.equal_range("24a");
  EXPECT_EQ(0, eq.second - eq.first);
}

TEST(FrozenMap, GetDefault) {
  auto fmap = freeze(osquares);
  EXPECT_EQ(4, fmap.getDefault(2));
  EXPECT_EQ(9, fmap.getDefault(3));
  EXPECT_EQ(0, fmap.getDefault(5));
  EXPECT_EQ(-1, fmap.getDefault(5, -1));
}

TEST(FrozenMap, GetOptional) {
  auto fmap = freeze(osquares);
  EXPECT_EQ(4, *fmap.getOptional(2));
  EXPECT_EQ(9, *fmap.getOptional(3));
  EXPECT_FALSE(fmap.getOptional(5));
}

TEST(FrozenMap, Nested) {
  std::map<uint32_t, std::map<uint32_t, uint32_t>> roots;
  for (uint32_t i = 0; i < 100; ++i) {
    roots[i][sqrt(i)] = sqrt(sqrt(i));
  }
  // TDOO(T44041774): fix over-alignment of maps leading to oversized
  // single-element maps.
  EXPECT_LT(frozenSize(roots), 640);
  auto froots = freeze(roots);
  if (auto l1 = froots.getDefault(4)) {
    if (auto l2 = l1.getDefault(2)) {
      EXPECT_EQ(l2, 1);
    } else {
      EXPECT_EQ(0, 1);
    }
  } else {
    EXPECT_EQ(0, 1);
  }
}

TEST(Frozen, Empty) {
  std::vector<int> zeros(100);
  // Only 1 byte needed to store length. All values represented in 0 bits,
  // since a zero-bit integer can faithfully reproduce 0.
  EXPECT_EQ(frozenSize(zeros), 1);

  // same here
  std::vector<std::pair<int, int>> zeroPairs(100);
  auto frozenPairs = freeze(zeroPairs);
  EXPECT_EQ(frozenSize(zeros), 1);

  // zero length fits in zero bits, too
  std::vector<std::pair<int, int>> noPairs;
  EXPECT_EQ(frozenSize(noPairs), 0);

  // views of a zero-length fields evaluate to false due to terse layouts
  EXPECT_FALSE(bool(frozenPairs[0]));

  // default-constructed views refer to nothing, evaluate to fales
  decltype(frozenPairs[0]) nothing;
  EXPECT_FALSE(bool(nothing));
}

TEST(FrozenMap, Size) {
  std::map<uint32_t, uint32_t> roots;
  for (uint32_t i = 0; i < 100; ++i) {
    roots[i] = sqrt(i);
  }
  EXPECT_LE(frozenSize(roots), (100 * (7 + 4) + 7) / 8 + 8);
}

template <typename T>
struct FrozenSetFull : public ::testing::Test {};
TYPED_TEST_SUITE_P(FrozenSetFull);

TYPED_TEST_P(FrozenSetFull, Full) {
  TypeParam primes{2};
  for (uint32_t i = 3; i < 100; i += 2) {
    bool prime = false;
    for (auto& op : primes) {
      if (op * op > i) {
        prime = true;
        break;
      }
    }
    if (prime) {
      primes.insert(i);
    }
  }
  auto fprimes = freeze(primes);
  EXPECT_EQ(primes, fprimes.thaw());
  EXPECT_LT(frozenSize(primes), primes.size() * 2);

  EXPECT_TRUE(fprimes.count(23));
  auto primeBefore50 = fprimes.upper_bound(50) - 1;
  auto primeAfter50 = fprimes.lower_bound(50);
  EXPECT_EQ(49, *primeBefore50);
  EXPECT_EQ(51, *primeAfter50);
  EXPECT_FALSE(fprimes.count(24));
}

REGISTER_TYPED_TEST_SUITE_P(FrozenSetFull, Full);
using FrozenSetTypes = testing::Types<
    std::set<uint32_t>,
    folly::sorted_vector_set<uint32_t>,
    folly::heap_vector_set<uint32_t>>;
INSTANTIATE_TYPED_TEST_SUITE_P(
    FrozenAssociativeTest, FrozenSetFull, FrozenSetTypes);

namespace {
template <typename HashSet>
void testFrozenHashSetFull() {
  HashSet primes{2};
  for (uint32_t i = 3; i < 100; i += 2) {
    bool prime = false;
    for (auto& op : primes) {
      if (op * op > i) {
        prime = true;
        break;
      }
    }
    if (prime) {
      primes.insert(i);
    }
  }
  auto fprimes = freeze(primes);
  EXPECT_EQ(primes, fprimes.thaw());
  EXPECT_LT(frozenSize(primes), primes.size() * 2);

  EXPECT_TRUE(fprimes.count(23));
  EXPECT_FALSE(fprimes.count(24));
}
} // namespace

TEST(FrozenHashSet, Full) {
  testFrozenHashSetFull<std::unordered_set<uint32_t>>();
  testFrozenHashSetFull<folly::F14NodeSet<uint32_t>>();
  testFrozenHashSetFull<folly::F14ValueSet<uint32_t>>();
  testFrozenHashSetFull<folly::F14VectorSet<uint32_t>>();
  testFrozenHashSetFull<folly::F14FastSet<uint32_t>>();
}

TEST(Frozen, IntHashMapBig) {
  std::unordered_map<int, int> map;
  for (int i = 0; i < 100; ++i) {
    int k = i * 100;
    map[k] = i;
  }
  auto fmap = freeze(map);
  for (int i = 0; i < 100; ++i) {
    int k = i * 100;
    EXPECT_EQ(i, fmap.at(k));
  }
  for (int i = 100; i < 200; ++i) {
    int k = i * 100;
    EXPECT_EQ(0, fmap.count(k));
  }
}

TEST(Frozen, StringHashSetBig) {
  std::unordered_set<std::string> set;
  for (int i = 0; i < 100; ++i) {
    auto k = folly::to<std::string>(i);
    set.insert(k);
  }
  auto fset = freeze(set);
  for (int i = 0; i < 100; ++i) {
    auto k = folly::to<std::string>(i);
    EXPECT_TRUE(fset.count(k));
  }
  for (int i = 100; i < 200; ++i) {
    auto k = folly::to<std::string>(i);
    EXPECT_FALSE(fset.count(k));
  }
}

TEST(Frozen, IntHashSetBig) {
  std::unordered_set<int> set;
  for (int i = 0; i < 100; ++i) {
    int k = i * 100;
    set.insert(k);
  }
  auto fset = freeze(set);
  for (int i = 0; i < 100; ++i) {
    int k = i * 100;
    EXPECT_TRUE(fset.count(k));
  }
  for (int i = 100; i < 200; ++i) {
    int k = i * 100;
    EXPECT_FALSE(fset.count(k));
  }
}

TEST(Frozen, StringHashMapBig) {
  std::unordered_map<std::string, int> map;
  for (int i = 0; i < 100; ++i) {
    auto k = folly::to<std::string>(i);
    map[k] = i;
  }
  auto fmap = freeze(map);
  for (int i = 0; i < 100; ++i) {
    auto k = folly::to<std::string>(i);
    EXPECT_EQ(i, fmap.at(k));
  }
  for (int i = 100; i < 200; ++i) {
    auto k = folly::to<std::string>(i);
    EXPECT_EQ(0, fmap.count(k));
  }
}

template <class T>
size_t distance(const std::pair<T, T>& pair) {
  return pair.second - pair.first;
}

TEST(Frozen, SpillBug) {
  std::vector<std::map<int, int>> maps{
      {{-4, -3}, {-2, -1}}, {{-1, -2}, {-3, -4}}};
  auto fmaps = freeze(maps);
  EXPECT_EQ(distance(fmaps[0].equal_range(3)), 0);
  EXPECT_EQ(distance(fmaps[0].equal_range(-1)), 0);
  EXPECT_EQ(distance(fmaps[1].equal_range(-1)), 1);
}

// Define hash and equal_to for User
namespace std {
size_t hash<User>::operator()(const User& user) const {
  return folly::hash::hash_combine_generic(
      folly::Hash{}, folly::StringPiece(*user.name()), *user.uid());
}

bool equal_to<User>::operator()(const User& lhs, const User& rhs) const {
  return *lhs.uid() == *rhs.uid() && *lhs.name() == *rhs.name();
}
} // namespace std

namespace apache::thrift::frozen {

size_t Layout<test::User>::hash(const test::User& user) {
  return folly::hash::hash_combine_generic(
      folly::Hash{}, 12345, folly::StringPiece(*user.name()), *user.uid());
}

size_t Layout<test::User>::hash(const Layout<test::User>::View& user) {
  return folly::hash::hash_combine_generic(
      folly::Hash{}, 12345, folly::StringPiece(user.name()), user.uid());
}

bool Layout<test::User>::View::operator==(
    const Layout<test::User>::View& rhs) const {
  return std::make_tuple(uid(), name()) ==
      std::make_tuple(rhs.uid(), rhs.name());
}

bool Layout<test::User>::View::operator<(
    const Layout<test::User>::View& rhs) const {
  return std::make_tuple(uid(), name()) <
      std::make_tuple(rhs.uid(), rhs.name());
}

bool operator==(const Layout<test::User>::View& lhs, const User& rhs) {
  return std::make_tuple(lhs.uid(), rhs.name()) ==
      std::make_tuple(*rhs.uid(), *rhs.name());
}

bool operator<(const Layout<test::User>::View& lhs, const User& rhs) {
  return std::make_tuple(lhs.uid(), rhs.name()) <
      std::make_tuple(*rhs.uid(), *rhs.name());
}

} // namespace apache::thrift::frozen

TEST(FrozenMap, StructAsKey) {
  auto user = [](int64_t uid, std::string name) {
    User u;
    *u.uid() = uid;
    *u.name() = name;
    return u;
  };

  const auto u1 = user(1, "mike");
  const auto u2 = user(2, "jack");
  const auto u3 = user(3, "andy");
  const auto u4 = user(4, "pat");

  std::vector<std::pair<User, int>> entries{
      {u1, 1},
      {u1, 2},
      {u2, 2},
      {u3, 3},
  };
  std::map<User, int> userMap(entries.begin(), entries.end());
  ASSERT_EQ(userMap.size(), 3);
  ASSERT_EQ(userMap.at(u1), 1);
  std::unordered_map<User, int> userHashMap(entries.begin(), entries.end());
  ASSERT_EQ(userHashMap.size(), 3);
  ASSERT_EQ(userHashMap.at(u1), 1);

  auto omap = freeze(userMap);
  auto hmap = freeze(userHashMap);
  for (auto& [k, v] : userMap) {
    auto fk = freeze(k);
    {
      auto found = omap.find(fk);
      ASSERT_FALSE(found == omap.end());
      EXPECT_EQ(found->second(), v);
      auto heterogenousFound = omap.find(k);
      EXPECT_EQ(heterogenousFound, found);
    }
    {
      auto found = hmap.find(fk);
      ASSERT_FALSE(found == hmap.end());
      EXPECT_EQ(found->second(), v);
      auto heterogenousFound = hmap.find(k);
      EXPECT_EQ(heterogenousFound, found);
    }
  }
  auto fu4 = freeze(u4);
  ASSERT_TRUE(omap.find(fu4) == omap.end());
  ASSERT_TRUE(hmap.find(fu4) == hmap.end());
  ASSERT_TRUE(omap.find(u4) == omap.end());
  ASSERT_TRUE(hmap.find(u4) == hmap.end());
}
