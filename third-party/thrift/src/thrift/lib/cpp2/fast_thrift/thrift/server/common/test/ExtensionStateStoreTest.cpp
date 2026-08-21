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

// Unit test for ExtensionStateStore — the per-connection slot table that lets
// cooperating extensions share one object. What it must guarantee: the same
// type reaches the same object, different types never alias, and a reference
// handed out early survives later slots being added.

#include <cstddef>
#include <string>
#include <utility>

#include <gtest/gtest.h>

#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/ExtensionStateStore.h>

namespace apache::thrift::fast_thrift::thrift {

namespace {

struct Identity {
  std::string name;
};

struct Counters {
  int requests{0};
};

struct Tracked {
  explicit Tracked(int& live) : live_(live) { ++live_; }
  ~Tracked() { --live_; }
  Tracked(const Tracked&) = delete;
  Tracked& operator=(const Tracked&) = delete;

  int& live_;
};

// A family of distinct state types, so a test can fill the store with slots
// that actually grow it — resolving one type repeatedly would reuse the single
// slot it already has.
template <size_t N>
struct Slot {
  size_t value{N};
};

template <size_t... Ns>
void createSlots(ExtensionStateStore& store, std::index_sequence<Ns...>) {
  (store.getOrCreate<Slot<Ns>>(), ...);
}

} // namespace

// The property the whole mechanism rests on: two lookups of the same type are
// the same object, so one extension's write is another's read.
TEST(ExtensionStateStoreTest, SameTypeResolvesToTheSameObject) {
  ExtensionStateStore store;

  store.getOrCreate<Identity>().name = "svc:caller";

  EXPECT_EQ(store.getOrCreate<Identity>().name, "svc:caller");
  EXPECT_EQ(&store.getOrCreate<Identity>(), &store.getOrCreate<Identity>());
}

// Distinct types are distinct slots — the collision the type-keyed store
// exists to make impossible.
TEST(ExtensionStateStoreTest, DistinctTypesGetDistinctSlots) {
  ExtensionStateStore store;

  store.getOrCreate<Identity>().name = "svc:caller";
  store.getOrCreate<Counters>().requests = 3;

  EXPECT_EQ(store.getOrCreate<Identity>().name, "svc:caller");
  EXPECT_EQ(store.getOrCreate<Counters>().requests, 3);
  EXPECT_EQ(store.size(), 2);
}

// Extensions take their reference at construction and keep it for the
// connection, so a slot added later must not move an earlier one. The slots
// added here are all distinct types, which is what makes the table grow and
// rehash — repeating one type would reuse its existing slot and prove nothing.
TEST(ExtensionStateStoreTest, EarlierReferencesSurviveLaterSlots) {
  ExtensionStateStore store;

  Identity& identity = store.getOrCreate<Identity>();
  identity.name = "svc:caller";
  constexpr size_t kLaterSlots = 64;
  createSlots(store, std::make_index_sequence<kLaterSlots>{});

  EXPECT_EQ(store.size(), kLaterSlots + 1);
  EXPECT_EQ(identity.name, "svc:caller");
  EXPECT_EQ(&identity, &store.getOrCreate<Identity>());
}

// Only the first call constructs: a second extension naming the type joins the
// existing object rather than resetting what the first one wrote.
TEST(ExtensionStateStoreTest, ArgumentsApplyOnlyToTheFirstCall) {
  ExtensionStateStore store;

  EXPECT_EQ(store.getOrCreate<Identity>(Identity{"first"}).name, "first");
  EXPECT_EQ(store.getOrCreate<Identity>(Identity{"second"}).name, "first");
}

TEST(ExtensionStateStoreTest, ContainsReportsOnlyCreatedSlots) {
  ExtensionStateStore store;

  EXPECT_FALSE(store.contains<Identity>());
  store.getOrCreate<Identity>();
  EXPECT_TRUE(store.contains<Identity>());
  EXPECT_FALSE(store.contains<Counters>());
}

// The store owns its slots, so a connection's state goes away with it.
TEST(ExtensionStateStoreTest, SlotsAreDestroyedWithTheStore) {
  int live = 0;
  {
    ExtensionStateStore store;
    store.getOrCreate<Tracked>(live);
    EXPECT_EQ(live, 1);
  }
  EXPECT_EQ(live, 0);
}

} // namespace apache::thrift::fast_thrift::thrift
