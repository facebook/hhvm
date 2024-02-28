/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/executors/ManualExecutor.h>
#include <folly/init/Init.h>
#include <folly/portability/GTest.h>
#include <gtest/gtest-printers.h>
#include <deque>
#include <stdexcept>
#include <string>
#include "watchman/LRUCache.h"

using namespace watchman;

static constexpr const std::chrono::milliseconds kErrorTTL(1000);

namespace watchman::lrucache {
template <typename... A>
[[maybe_unused]] static std::ostream& operator<<(
    std::ostream& out,
    Node<A...> const& node) {
  return out << (const void*)&node;
}
} // namespace watchman::lrucache

TEST(CacheTest, basics) {
  LRUCache<std::string, bool> cache(5, kErrorTTL);

  EXPECT_EQ(cache.size(), 0) << "initially empty";
  EXPECT_EQ(cache.get("foo"), nullptr) << "nullptr for non-existent item";

  EXPECT_TRUE(cache.set("foo", true)->value()) << "inserted true";
  EXPECT_EQ(cache.size(), 1) << "size is now one";
  EXPECT_TRUE(cache.get("foo")->value()) << "looked up item";

  EXPECT_FALSE(cache.set("foo", false)->value()) << "replaced true with false";
  EXPECT_FALSE(cache.get("foo")->value()) << "looked up new false item";
  EXPECT_EQ(cache.size(), 1) << "replacement didn't change size";

  EXPECT_FALSE(cache.erase("foo")->value()) << "erased and returned false foo";
  EXPECT_EQ(cache.erase("foo"), nullptr)
      << "double erase doesn't return anything";
  EXPECT_EQ(cache.get("foo"), nullptr) << "nullptr for non-existent item";

  for (size_t i = 0; i < 6; ++i) {
    EXPECT_NE(cache.set(std::to_string(i), true), nullptr) << "inserted";
  }

  EXPECT_EQ(cache.size(), 5)
      << "limited to 5 items, despite inserting 6 total. size=" << cache.size();

  EXPECT_EQ(cache.get("0"), nullptr) << "we expect 0 to have been evicted";
  for (size_t i = 1; i < 6; ++i) {
    EXPECT_TRUE(cache.get(std::to_string(i))) << "found later node " << i;
  }

  EXPECT_TRUE(cache.set("bar", true)) << "added new item";
  EXPECT_EQ(cache.get("1"), nullptr) << "we expect 1 to be evicted";
  EXPECT_TRUE(cache.get("2")) << "2 should be there, and we just touched it";
  EXPECT_TRUE(cache.set("baz", true)) << "added new item";
  EXPECT_EQ(cache.size(), 5) << "max size still respected";
  EXPECT_TRUE(cache.get("2")) << "2 should still be there; not evicted";
  EXPECT_EQ(cache.get("3"), nullptr) << "we expect 3 to be evicted";

  cache.clear();
  EXPECT_EQ(cache.size(), 0) << "cleared out and have zero items";
}

TEST(CacheTest, future) {
  using Cache = LRUCache<int, int>;
  using Node = typename Cache::NodeType;
  Cache cache(5, kErrorTTL);
  folly::ManualExecutor exec;

  auto now = std::chrono::steady_clock::now();

  auto okGetter = [&exec](int k) {
    return folly::makeFuture(k).via(&exec).thenTry(
        [](folly::Try<int>&& key) { return (1 + key.value()) * 2; });
  };

  auto failGetter = [&exec](int k) {
    return folly::makeFuture(k).via(&exec).thenTry(
        [](folly::Try<int>&&) -> int { throw std::runtime_error("bleet"); });
  };

  // Queue up a get via a getter that will succeed
  auto f = cache.get(0, okGetter, now);
  EXPECT_FALSE(f.isReady()) << "future didn't finish yet";

  EXPECT_THROW(cache.get(0), std::runtime_error);

  // Queue up a second get using the same getter
  auto f2 = cache.get(0, okGetter, now);
  EXPECT_FALSE(f2.isReady()) << "also not ready";

  exec.drain();

  EXPECT_TRUE(f.isReady()) << "first is ready";
  EXPECT_TRUE(f2.isReady()) << "second is ready";

  EXPECT_EQ(f.value()->value(), 2) << "got correct value for first";
  EXPECT_EQ(f.value()->value(), f2.value()->value())
      << "got same value for second";

  // Now to saturate the cache with failed lookups

  cache.clear();
  std::vector<folly::Future<std::shared_ptr<const Node>>> futures;
  for (size_t i = 1; i < 7; ++i) {
    futures.emplace_back(
        cache.get(i, failGetter, now + std::chrono::milliseconds(i)));
  }

  auto drained = exec.drain();
  EXPECT_EQ(drained, 13) << "should be 13 things pending, but have " << drained;

  // Let them make progress
  exec.drain();

  EXPECT_EQ(cache.size(), 5)
      << "cache should be full, but has " << cache.size();

  folly::collectAll(futures.begin(), futures.end())
      .defer(
          [](folly::Try<std::vector<folly::Try<std::shared_ptr<const Node>>>>&&
                 result) {
            for (auto& r : result.value()) {
              EXPECT_TRUE(r.value()->result().hasException())
                  << "should be an error node";
            }
          })
      .wait();

  EXPECT_EQ(cache.size(), 5)
      << "cache should still be full (no excess) but has " << cache.size();

  EXPECT_EQ(cache.get(42, now), nullptr) << "we don't have 42 yet";

  // Now if we "sleep" for long enough, we should be able to evict
  // the error nodes and allow the insert to happen.
  EXPECT_EQ(cache.get(42, now), nullptr) << "we don't have 42 yet";
  EXPECT_TRUE(cache.set(42, 42, now + kErrorTTL + std::chrono::milliseconds(1)))
      << "inserted";
  EXPECT_NE(cache.get(42, now), nullptr) << "we found 42 in the cache";
  EXPECT_EQ(cache.size(), 5)
      << "cache should still be full (no excess) but has " << cache.size();
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  folly::init(&argc, &argv);
  return RUN_ALL_TESTS();
}
