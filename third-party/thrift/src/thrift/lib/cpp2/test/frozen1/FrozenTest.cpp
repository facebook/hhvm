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

#include <random>
#include <gtest/gtest.h>

#include <folly/Conv.h>
#include <folly/MapUtil.h>
#include <thrift/lib/cpp2/protocol/DebugProtocol.h>
#include <thrift/lib/cpp2/test/frozen1/gen-cpp2/FrozenTypes_types.h>

using namespace apache::thrift;
using namespace FrozenTypes;
using folly::fbstring;
using folly::StringPiece;
using std::map;
using std::string;
using std::unordered_map;
using std::vector;

auto hasher = std::hash<int64_t>();

double randomDouble(double max) {
  static std::mt19937_64 engine;
  return std::uniform_real_distribution<double>(0, max)(engine);
}

Team testValue() {
  Team team;
  team.peopleById() = {};
  team.peopleByName() = {};
  for (int i = 1; i <= 10; ++i) {
    auto id = hasher(i);
    Person p;
    *p.id() = id;
    p.nums()->insert(i);
    p.nums()->insert(-i);
    p.dob() = randomDouble(1e9);
    folly::toAppend("Person ", i, &(*p.name()));
    (*team.peopleById())[*p.id()] = p;
    auto& peopleByNameEntry = (*team.peopleByName())[*p.name()];
    peopleByNameEntry = std::move(p);
  }
  team.projects() = {};
  team.projects()->insert("alpha");
  team.projects()->insert("beta");

  return team;
}

TEST(Frozen, Basic) {
  Team team = testValue();
  EXPECT_EQ(*team.peopleById()->at(hasher(3)).name(), "Person 3");
  EXPECT_EQ(*team.peopleById()->at(hasher(4)).name(), "Person 4");
  EXPECT_EQ(*team.peopleById()->at(hasher(5)).name(), "Person 5");
  EXPECT_EQ(*team.peopleByName()->at("Person 3").id(), 3);
  EXPECT_EQ(team.peopleByName()->begin()->second.nums()->count(-1), 1);
  EXPECT_EQ(team.projects()->count("alpha"), 1);
  EXPECT_EQ(team.projects()->count("beta"), 1);

  size_t size = frozenSize(team);
  (void)size;
  for (int misalign = 0; misalign < 16; ++misalign) {
    std::vector<byte> bytes(frozenSize(team) + misalign);
    byte* const freezeLocation = &bytes[misalign];
    byte* buffer = freezeLocation;
    auto* freezeResult = freeze(team, buffer);
    const byte* const frozenLocation =
        static_cast<const byte*>(static_cast<const void*>(freezeResult));
    // verify that freeze didn't yeild a different address.
    EXPECT_EQ(freezeLocation - &bytes[0], frozenLocation - &bytes[0]);

    std::vector<byte> copy(bytes);
    byte* copyBuffer = &bytes[misalign];
    auto& frozen = *(Frozen<Team>*)copyBuffer;

    auto thawedTeam = thaw(frozen);
    EXPECT_EQ(
        frozen.peopleById.at(static_cast<int64_t>(hasher(3))).name.range(),
        "Person 3");
    EXPECT_EQ(
        frozen.peopleById.at(static_cast<int64_t>(hasher(4))).name, "Person 4");
    EXPECT_EQ(
        frozen.peopleById.at(static_cast<int64_t>(hasher(5))).name, "Person 5");
    EXPECT_EQ(
        frozen.peopleById.at(static_cast<int64_t>(hasher(3))).dob,
        *team.peopleById()->at(hasher(3)).dob());
    EXPECT_EQ(frozen.peopleByName.at("Person 3").id, 3);
    EXPECT_EQ(frozen.peopleByName.at(string("Person 4")).id, 4);
    EXPECT_EQ(frozen.peopleByName.at(fbstring("Person 5")).id, 5);
    EXPECT_EQ(frozen.peopleByName.at(StringPiece("Person 6")).id, 6);
    EXPECT_EQ(frozen.peopleByName.begin()->second.nums.count(-1), 1);
    EXPECT_EQ(frozen.projects.count("alpha"), 1);
    EXPECT_EQ(frozen.projects.count("beta"), 1);

    EXPECT_THROW(
        frozen.peopleById.at(static_cast<int64_t>(hasher(50))),
        std::out_of_range);
  }
}

TEST(Frozen, FieldOrdering) {
  Pod p;
  *p.a() = 0x012345;
  *p.b() = 0x0678;
  *p.c() = 0x09;
  EXPECT_LT(static_cast<void*>(&(*p.a())), static_cast<void*>(&(*p.b())));
  EXPECT_LT(static_cast<void*>(&(*p.b())), static_cast<void*>(&(*p.c())));
  auto pf = freeze(p);
  auto& f = *pf;
  EXPECT_EQ(sizeof(f.__isset), 1);
  EXPECT_EQ(sizeof(f), sizeof(int32_t) + sizeof(int16_t) + sizeof(uint8_t) + 1);
  EXPECT_LT(static_cast<const void*>(&f.a), static_cast<const void*>(&f.b));
  EXPECT_LT(static_cast<const void*>(&f.b), static_cast<const void*>(&f.c));
}

TEST(Frozen, IntHashMap) {
  std::unordered_map<int, int> umap{
      {1, 2},
      {3, 4},
      {7, 8},
      {5, 6},
  };
  auto pfmap = freeze(umap);
  auto& fmap = *pfmap;
  std::unordered_map<int, int> tmap;
  thaw(fmap, tmap);
  EXPECT_EQ(umap, tmap);
  auto e = fmap.end();
  EXPECT_TRUE(fmap.find(0) == e);
  EXPECT_TRUE(fmap.find(4) == e);
  EXPECT_TRUE(fmap.find(9) == e);
  EXPECT_TRUE(fmap.count(0) == 0);
  EXPECT_TRUE(fmap.count(1) == 1);
  EXPECT_TRUE(fmap.count(3) == 1);
  EXPECT_TRUE(fmap.count(4) == 0);
  EXPECT_TRUE(fmap.count(9) == 0);

  EXPECT_EQ(fmap.at(1), 2);
  EXPECT_EQ(fmap.at(3), 4);
  EXPECT_EQ(fmap.at(7), 8);
  EXPECT_THROW(fmap.at(9), std::out_of_range);

  EXPECT_TRUE(folly::get_ptr(fmap, 1));
  EXPECT_EQ(2, *folly::get_ptr(fmap, 1));

  EXPECT_TRUE(!folly::get_ptr(fmap, 2));

  EXPECT_TRUE(folly::get_ptr(fmap, 3));
  EXPECT_EQ(4, *folly::get_ptr(fmap, 3));
}

TEST(Frozen, StringHashMap) {
  std::unordered_map<string, int> umap{
      {"1", 2},
      {"3", 4},
      {"7", 8},
      {"5", 6},
  };
  auto pfmap = freeze(umap);
  auto& fmap = *pfmap;
  std::unordered_map<string, int> tmap;
  thaw(fmap, tmap);
  EXPECT_EQ(umap, tmap);
  auto e = fmap.end();
  EXPECT_TRUE(fmap.find("0") == e);
  EXPECT_TRUE(fmap.find("4") == e);
  EXPECT_TRUE(fmap.find("9") == e);
  EXPECT_TRUE(fmap.count("0") == 0);
  EXPECT_TRUE(fmap.count("1") == 1);
  EXPECT_TRUE(fmap.count("3") == 1);
  EXPECT_TRUE(fmap.count("4") == 0);
  EXPECT_TRUE(fmap.count("9") == 0);

  EXPECT_EQ(fmap.at("1"), 2);
  EXPECT_EQ(fmap.at("1"), 2);
  EXPECT_EQ(fmap.at("7"), 8);
  EXPECT_THROW(fmap.at("9"), std::out_of_range);

  EXPECT_TRUE(folly::get_ptr(fmap, *freezeStr("1")));
  EXPECT_EQ(2, *folly::get_ptr(fmap, *freezeStr("1")));

  EXPECT_TRUE(!folly::get_ptr(fmap, *freezeStr("2")));

  EXPECT_TRUE(folly::get_ptr(fmap, *freezeStr("3")));
  EXPECT_EQ(4, *folly::get_ptr(fmap, *freezeStr("3")));
}

TEST(Frozen, IntHashMapBig) {
  std::unordered_map<int, int> tmap;
  for (int i = 0; i < 100; ++i) {
    int k = i * 100;
    tmap[k] = i;
  }
  auto pfmap = freeze(tmap);
  auto& fmap = *pfmap;
  for (int i = 0; i < 100; ++i) {
    int k = i * 100;
    EXPECT_EQ(i, fmap.at(k));
  }
  for (int i = 100; i < 200; ++i) {
    int k = i * 100;
    EXPECT_EQ(0, fmap.count(k));
  }
}

TEST(Frozen, StringHashMapBig) {
  std::unordered_map<string, int> tmap;
  for (int i = 0; i < 100; ++i) {
    auto k = folly::to<string>(i);
    tmap[k] = i;
  }
  auto pfmap = freeze(tmap);
  auto& fmap = *pfmap;
  for (int i = 0; i < 100; ++i) {
    auto k = folly::to<string>(i);
    EXPECT_EQ(i, fmap.at(*freeze(k)));
  }
  for (int i = 100; i < 200; ++i) {
    auto k = folly::to<string>(i);
    EXPECT_EQ(0, fmap.count(*freeze(k)));
  }
}

TEST(Frozen, LookupDemo) {
  unordered_map<string, vector<string>> roots{
      {"1", {"1", "2", "3"}},
      {"2", {"4", "5", "6", "7", "8"}},
      {"3", {"9", "10", "11", "12", "13", "14", "15"}},
  };
  size_t bytes = frozenSize(roots);
  // corresponding source data: >500 by
  EXPECT_LT(bytes, 250);
  auto frozen = freeze(roots);
  EXPECT_EQ(frozen->at(*freezeStr("2"))[3].range(), "7");
}

TEST(Frozen, StringMap) {
  map<string, int> tmap{
      {"1", 2},
      {"3", 4},
      {"7", 8},
      {"5", 6},
  };
  auto pfmap = freeze(tmap);
  auto& fmap = *pfmap;
  EXPECT_EQ(fmap.at("3"), 4);
  auto b = fmap.begin();
  auto e = fmap.end();
  using std::make_pair;
  EXPECT_TRUE(fmap.find("0") == e);
  EXPECT_TRUE(fmap.find("3") == b + 1);
  EXPECT_TRUE(fmap.find("4") == e);
  EXPECT_TRUE(fmap.find("9") == e);
  EXPECT_TRUE(fmap.lower_bound("0") == b);
  EXPECT_TRUE(fmap.lower_bound("1") == b);
  EXPECT_TRUE(fmap.lower_bound("3") == b + 1);
  EXPECT_TRUE(fmap.lower_bound("4") == b + 2);
  EXPECT_TRUE(fmap.lower_bound("9") == e);
  EXPECT_TRUE(fmap.upper_bound("0") == b);
  EXPECT_TRUE(fmap.upper_bound("1") == b + 1);
  EXPECT_TRUE(fmap.upper_bound("3") == b + 2);
  EXPECT_TRUE(fmap.upper_bound("4") == b + 2);
  EXPECT_TRUE(fmap.upper_bound("9") == e);
  EXPECT_TRUE(fmap.equal_range("0") == make_pair(b, b));
  EXPECT_TRUE(fmap.equal_range("1") == make_pair(b, b + 1));
  EXPECT_TRUE(fmap.equal_range("3") == make_pair(b + 1, b + 2));
  EXPECT_TRUE(fmap.equal_range("4") == make_pair(b + 2, b + 2));
  EXPECT_TRUE(fmap.equal_range("9") == make_pair(e, e));
  EXPECT_TRUE(fmap.count("0") == 0);
  EXPECT_TRUE(fmap.count("1") == 1);
  EXPECT_TRUE(fmap.count("3") == 1);
  EXPECT_TRUE(fmap.count("4") == 0);
  EXPECT_TRUE(fmap.count("9") == 0);

  EXPECT_EQ(fmap.at("1"), 2);
  EXPECT_EQ(fmap.at("7"), 8);
  EXPECT_THROW(fmap.at("9"), std::out_of_range);

  EXPECT_TRUE(folly::get_ptr(fmap, *freezeStr("1")));
  EXPECT_EQ(2, *folly::get_ptr(fmap, *freezeStr("1")));

  EXPECT_TRUE(!folly::get_ptr(fmap, *freezeStr("2")));

  EXPECT_TRUE(folly::get_ptr(fmap, *freezeStr("3")));
  EXPECT_EQ(4, *folly::get_ptr(fmap, *freezeStr("3")));
}

TEST(Frozen, SetString) {
  std::set<string> tset{"1", "3", "7", "5"};
  auto pfset = freeze(tset);
  auto& fset = *pfset;
  auto b = fset.begin();
  auto e = fset.end();
  using std::make_pair;
  EXPECT_TRUE(fset.find("3") == b + 1);
  EXPECT_TRUE(fset.find(folly::fbstring("3")) == b + 1);
  EXPECT_TRUE(fset.find(std::string("3")) == b + 1);
  EXPECT_TRUE(fset.find(folly::StringPiece("3")) == b + 1);
  EXPECT_TRUE(fset.find("0") == e);
  EXPECT_TRUE(fset.find("3") == b + 1);
  EXPECT_TRUE(fset.find("4") == e);
  EXPECT_TRUE(fset.find("9") == e);
  EXPECT_TRUE(fset.lower_bound("0") == b);
  EXPECT_TRUE(fset.lower_bound("1") == b);
  EXPECT_TRUE(fset.lower_bound("3") == b + 1);
  EXPECT_TRUE(fset.lower_bound("4") == b + 2);
  EXPECT_TRUE(fset.lower_bound("9") == e);
  EXPECT_TRUE(fset.upper_bound("0") == b);
  EXPECT_TRUE(fset.upper_bound("1") == b + 1);
  EXPECT_TRUE(fset.upper_bound("3") == b + 2);
  EXPECT_TRUE(fset.upper_bound("4") == b + 2);
  EXPECT_TRUE(fset.upper_bound("9") == e);
  EXPECT_TRUE(fset.equal_range("0") == make_pair(b, b));
  EXPECT_TRUE(fset.equal_range("1") == make_pair(b, b + 1));
  EXPECT_TRUE(fset.equal_range("3") == make_pair(b + 1, b + 2));
  EXPECT_TRUE(fset.equal_range("4") == make_pair(b + 2, b + 2));
  EXPECT_TRUE(fset.equal_range("9") == make_pair(e, e));
  EXPECT_TRUE(fset.count("0") == 0);
  EXPECT_TRUE(fset.count("1") == 1);
  EXPECT_TRUE(fset.count("3") == 1);
  EXPECT_TRUE(fset.count("4") == 0);
  EXPECT_TRUE(fset.count("9") == 0);
}

TEST(Frozen, VectorInt) {
  std::vector<int> tvect{1, 3, 7, 5};
  auto pfvect = freeze(tvect);
  auto& fvect = *pfvect;
  auto b = fvect.begin();
  auto e = fvect.end();
  EXPECT_EQ(fvect.front(), 1);
  EXPECT_EQ(fvect.back(), 5);
  EXPECT_EQ(fvect.size(), 4);
  EXPECT_EQ(fvect[1], 3);
  EXPECT_EQ(b[0], 1);
  EXPECT_EQ(b[1], 3);
  EXPECT_EQ(e[-1], 5);
  EXPECT_EQ(e[-2], 7);
}

TEST(Frozen, RelativePtr) {
  struct {
    RelativePtr<int> rptr;
    int after;
  } locals;

  CHECK_LT((void*)&locals.rptr, (void*)&locals.after);

  locals.rptr.reset(&locals.after);
  EXPECT_EQ(locals.rptr.get(), &locals.after); // basics
  locals.rptr.reset(&locals.after - 8 + (1 << 30)); // within 4GB = okay

  // pointing to lower addresses = underflow
  EXPECT_DEATH(locals.rptr.reset(&locals.after - 4), "address");
  // pointing to addresses more than 4GB away = overflow
  EXPECT_DEATH(locals.rptr.reset(&locals.after + (1 << 30)), "address");
}

TEST(Frozen, Utf8StringMap) {
  map<string, int> tmap{
      {"anxiety", 1},
      {"a\u00F1onuevo", 2},
      {"aot", 3},
      {"bacon", 4},
  };
  auto pfmap = freeze(tmap);
  auto& fmap = *pfmap;
  EXPECT_NE(fmap.find("anxiety"), fmap.end());
  EXPECT_NE(fmap.find("a\u00F1onuevo"), fmap.end());
  EXPECT_NE(fmap.find("aot"), fmap.end());
  EXPECT_NE(fmap.find("bacon"), fmap.end());
  EXPECT_EQ(fmap.at("anxiety"), 1);
  EXPECT_EQ(fmap.at("a\u00F1onuevo"), 2);
  EXPECT_EQ(fmap.at("aot"), 3);
  EXPECT_EQ(fmap.at("bacon"), 4);
}
