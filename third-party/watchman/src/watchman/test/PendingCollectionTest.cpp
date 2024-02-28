/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "watchman/PendingCollection.h"
#include "watchman/Logging.h"

#include <folly/logging/xlog.h>
#include <folly/portability/GTest.h>
#include <chrono>

using namespace watchman;

namespace watchman {
[[maybe_unused]] static std::ostream& operator<<(
    std::ostream& out,
    PendingChanges const& item) {
  return out << (const void*)&item;
}
[[maybe_unused]] static std::ostream& operator<<(
    std::ostream& out,
    watchman_pending_fs const& item) {
  return out << (const void*)&item;
}
} // namespace watchman

namespace {

void build_list(
    std::vector<watchman_pending_fs>* list,
    std::chrono::system_clock::time_point now,
    const w_string& parent_name,
    size_t depth,
    size_t num_files,
    size_t num_dirs) {
  size_t i;
  for (i = 0; i < num_files; i++) {
    list->emplace_back(
        w_string::build(parent_name.view(), "/file", i),
        now,
        W_PENDING_VIA_NOTIFY);
  }

  for (i = 0; i < num_dirs; i++) {
    list->emplace_back(
        w_string::build(parent_name.view(), "/dir", i),
        now,
        W_PENDING_RECURSIVE);

    if (depth > 0) {
      build_list(list, now, list->back().path, depth - 1, num_files, num_dirs);
    }
  }
}

size_t process_items(PendingCollection::LockedPtr& coll) {
  size_t drained = 0;

  auto item = coll->stealItems();
  while (item) {
    drained++;
    item = std::move(item->next);
  }
  return drained;
}

} // namespace

// Simulate
TEST(Pending, bench) {
  // These parameters give us 262140 items to track
  const size_t tree_depth = 7;
  const size_t num_files_per_dir = 8;
  const size_t num_dirs_per_dir = 4;
  w_string root_name("/some/path", W_STRING_BYTE);
  std::vector<watchman_pending_fs> list;
  const size_t alloc_size = 280000;

  list.reserve(alloc_size);

  // Build a list ordered from the root (top) down to the leaves.
  auto build_start = std::chrono::system_clock::now();
  build_list(
      &list,
      build_start,
      root_name,
      tree_depth,
      num_files_per_dir,
      num_dirs_per_dir);
  XLOG(ERR) << "built list with " << list.size() << " items";

  // Benchmark insertion in top-down order.
  {
    PendingCollection coll;
    size_t drained = 0;
    auto lock = coll.lock();

    auto start = std::chrono::steady_clock::now();
    for (auto& item : list) {
      lock->add(item.path, item.now, item.flags);
    }
    drained = process_items(lock);

    auto end = std::chrono::steady_clock::now();
    XLOG(ERR) << "took " << std::chrono::duration<double>(end - start).count()
              << "s to insert " << drained << " items into pending coll";
  }

  // and now in reverse order; this is from the leaves of the filesystem
  // tree up to the root, or bottom-up.  This simulates the workload of
  // a recursive delete of a filesystem tree.
  {
    PendingCollection coll;
    size_t drained = 0;
    auto lock = coll.lock();

    auto start = std::chrono::steady_clock::now();
    for (auto it = list.rbegin(); it != list.rend(); ++it) {
      auto& item = *it;
      lock->add(item.path, item.now, item.flags);
    }

    drained = process_items(lock);

    auto end = std::chrono::steady_clock::now();
    XLOG(ERR) << "took " << std::chrono::duration<double>(end - start).count()
              << "s to reverse insert " << drained
              << " items into pending coll";
  }
}

namespace {

template <typename Collection>
class PendingCollectionFixture : public testing::Test {
 public:
  Collection coll;
  std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
};

class WrappedPendingCollection : public PendingCollectionBase {
 public:
  WrappedPendingCollection() : PendingCollectionBase{cond} {}

  std::condition_variable cond;
};

/**
 * A naive, low-performance implementation of PendingCollection so they can be
 * fuzzed in relation to each other.
 *
 * This type is intended to be as unclever as possible.
 */
class NaivePendingCollection {
 public:
  void add(
      const w_string& path,
      std::chrono::system_clock::time_point now,
      PendingFlags flags) {
    for (std::shared_ptr<watchman_pending_fs> p = head_; p; p = p->next) {
      if (path.piece().startsWith(p->path) &&
          watchman::is_path_prefix(path, p->path)) {
        if ((p->flags & (W_PENDING_RECURSIVE | W_PENDING_CRAWL_ONLY)) ==
            W_PENDING_RECURSIVE) {
          return;
        }
      }
    }

    for (std::shared_ptr<watchman_pending_fs> p = head_; p; p = p->next) {
      if (p->path == path) {
        // consolidateItem
        p->flags.set(
            flags &
            (W_PENDING_CRAWL_ONLY | W_PENDING_RECURSIVE |
             W_PENDING_IS_DESYNCED));
        // TODO: should prune here
        return;
      }
    }

    // maybePruneObsoletedChildren
    if ((flags & (W_PENDING_RECURSIVE | W_PENDING_CRAWL_ONLY)) ==
        W_PENDING_RECURSIVE) {
      std::shared_ptr<watchman_pending_fs>* prev = &head_;
      auto p = head_;
      while (p) {
        if (watchman::is_path_prefix(p->path, path)) {
          (*prev) = p->next;
          p = p->next;
        } else {
          prev = &(*prev)->next;
          p = p->next;
        }
      }
    }

    auto p = std::make_shared<watchman_pending_fs>(path, now, flags);
    p->next = head_;
    head_ = p;
  }

  size_t getPendingItemCount() const {
    size_t i = 0;
    for (auto p = head_; p; p = p->next) {
      ++i;
    }
    return i;
  }

  std::shared_ptr<watchman_pending_fs> stealItems() {
    return std::exchange(head_, nullptr);
  }

 private:
  std::shared_ptr<watchman_pending_fs> head_;
};

using PCTypes = ::testing::Types<PendingChanges, NaivePendingCollection>;

} // namespace

TYPED_TEST_SUITE(PendingCollectionFixture, PCTypes);

TYPED_TEST(PendingCollectionFixture, add_one_item) {
  w_string path{"foo/bar"};
  PendingFlags flags;

  this->coll.add(path, this->now, flags);

  auto item = this->coll.stealItems();
  ASSERT_NE(nullptr, item);
  EXPECT_EQ(nullptr, item->next);
  EXPECT_EQ(w_string{"foo/bar"}, item->path);
  EXPECT_EQ(PendingFlags{}, item->flags);
}

TYPED_TEST(PendingCollectionFixture, add_two_items) {
  PendingFlags flags;

  this->coll.add(w_string{"foo/bar"}, this->now, flags);
  this->coll.add(w_string{"foo/baz"}, this->now, flags);

  auto item = this->coll.stealItems();
  ASSERT_NE(nullptr, item);
  EXPECT_NE(nullptr, item->next);
  EXPECT_EQ(w_string{"foo/baz"}, item->path);
  EXPECT_EQ(PendingFlags{}, item->flags);

  item = item->next;
  ASSERT_NE(nullptr, item);
  EXPECT_EQ(nullptr, item->next);
  EXPECT_EQ(w_string{"foo/bar"}, item->path);
  EXPECT_EQ(PendingFlags{}, item->flags);
}

TYPED_TEST(PendingCollectionFixture, same_item_consolidates) {
  PendingFlags flags;
  this->coll.add(w_string{"foo/bar"}, this->now, flags);
  this->coll.add(w_string{"foo/bar"}, this->now, flags);

  auto item = this->coll.stealItems();
  ASSERT_NE(nullptr, item);
  EXPECT_EQ(nullptr, item->next);
  EXPECT_EQ(w_string{"foo/bar"}, item->path);
  EXPECT_EQ(PendingFlags{}, item->flags);
}

TYPED_TEST(PendingCollectionFixture, prune_obsoleted_children) {
  this->coll.add(w_string{"foo/bar"}, this->now, 0);
  this->coll.add(w_string{"foo"}, this->now, W_PENDING_RECURSIVE);

  auto item = this->coll.stealItems();
  ASSERT_NE(nullptr, item);
  EXPECT_EQ(nullptr, item->next);
  EXPECT_EQ(w_string{"foo"}, item->path);
  EXPECT_EQ(W_PENDING_RECURSIVE, item->flags);
}

TYPED_TEST(
    PendingCollectionFixture,
    recursive_parents_prevent_children_from_being_added) {
  this->coll.add(w_string{"foo"}, this->now, W_PENDING_RECURSIVE);
  this->coll.add(w_string{"foo/bar"}, this->now, 0);

  auto item = this->coll.stealItems();
  ASSERT_NE(nullptr, item);
  EXPECT_EQ(nullptr, item->next);
  EXPECT_EQ(w_string{"foo"}, item->path);
  EXPECT_EQ(W_PENDING_RECURSIVE, item->flags);
}

TYPED_TEST(
    PendingCollectionFixture,
    mixed_addition_is_cleared_by_recursive_parents) {
  this->coll.add(w_string{"foo/bar"}, this->now, 0);
  this->coll.add(w_string{"foo/bar/baz"}, this->now, 0);
  this->coll.add(w_string{"foo"}, this->now, W_PENDING_RECURSIVE);
  this->coll.add(w_string{"foo/bar/baz/qux"}, this->now, 0);

  auto item = this->coll.stealItems();
  ASSERT_NE(nullptr, item);
  EXPECT_EQ(nullptr, item->next);
  EXPECT_EQ(w_string{"foo"}, item->path);
  EXPECT_EQ(W_PENDING_RECURSIVE, item->flags);
}

TYPED_TEST(PendingCollectionFixture, unrelated_prefixes_dont_prune) {
  PendingFlags flags;
  this->coll.add(w_string{"foo/bar"}, this->now, flags);
  this->coll.add(w_string{"f"}, this->now, W_PENDING_RECURSIVE);

  auto item = this->coll.stealItems();
  ASSERT_NE(nullptr, item);
  EXPECT_NE(nullptr, item->next);
  EXPECT_EQ(w_string{"f"}, item->path);
  EXPECT_EQ(W_PENDING_RECURSIVE, item->flags);

  item = item->next;
  ASSERT_NE(nullptr, item);
  EXPECT_EQ(nullptr, item->next);
  EXPECT_EQ(w_string{"foo/bar"}, item->path);
  EXPECT_EQ(PendingFlags{}, item->flags);
}

TYPED_TEST(PendingCollectionFixture, queue_crawl_then_notify) {
  this->coll.add(
      w_string{"foo"}, this->now, W_PENDING_CRAWL_ONLY | W_PENDING_RECURSIVE);
  this->coll.add(w_string{"foo/bar"}, this->now, W_PENDING_VIA_NOTIFY);

  // TODO: Why are these paths not returned bottom-up?

  auto item = this->coll.stealItems();
  ASSERT_NE(nullptr, item);
  EXPECT_NE(nullptr, item->next);
  EXPECT_EQ(w_string{"foo/bar"}, item->path);
  EXPECT_EQ(W_PENDING_VIA_NOTIFY, item->flags);

  item = item->next;
  EXPECT_EQ(nullptr, item->next);
  EXPECT_EQ(w_string{"foo"}, item->path);
  EXPECT_EQ(W_PENDING_CRAWL_ONLY | W_PENDING_RECURSIVE, item->flags);
}

TYPED_TEST(PendingCollectionFixture, queue_notify_then_crawl) {
  this->coll.add(w_string{"foo/bar"}, this->now, W_PENDING_VIA_NOTIFY);
  this->coll.add(
      w_string{"foo"}, this->now, W_PENDING_CRAWL_ONLY | W_PENDING_RECURSIVE);

  auto item = this->coll.stealItems();
  ASSERT_NE(nullptr, item);
  EXPECT_NE(nullptr, item->next);
  EXPECT_EQ(w_string{"foo"}, item->path);
  EXPECT_EQ(W_PENDING_CRAWL_ONLY | W_PENDING_RECURSIVE, item->flags);

  item = item->next;
  EXPECT_EQ(nullptr, item->next);
  EXPECT_EQ(w_string{"foo/bar"}, item->path);
  EXPECT_EQ(W_PENDING_VIA_NOTIFY, item->flags);
}

TYPED_TEST(PendingCollectionFixture, real_example) {
  this->coll.add(
      w_string{"/home/chadaustin/tmp/watchmanroots/test-root/foo/baz"},
      this->now,
      W_PENDING_NONRECURSIVE_SCAN | W_PENDING_RECURSIVE);
  this->coll.add(
      w_string{"/home/chadaustin/tmp/watchmanroots/test-root/foo/baz"},
      this->now,
      W_PENDING_NONRECURSIVE_SCAN);
  this->coll.add(
      w_string{"/home/chadaustin/tmp/watchmanroots/test-root/foo/baq"},
      this->now,
      W_PENDING_NONRECURSIVE_SCAN);
  this->coll.add(
      w_string{"/home/chadaustin/tmp/watchmanroots/test-root/f"},
      this->now,
      W_PENDING_NONRECURSIVE_SCAN | W_PENDING_RECURSIVE);

  EXPECT_EQ(3, this->coll.getPendingItemCount());

  auto item = this->coll.stealItems();
  ASSERT_NE(nullptr, item);
  EXPECT_NE(nullptr, item->next);

  item = item->next;
  ASSERT_NE(nullptr, item);
  EXPECT_NE(nullptr, item->next);

  item = item->next;
  ASSERT_NE(nullptr, item);
  EXPECT_EQ(nullptr, item->next);
}
