/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Synchronized.h>
#include <folly/futures/Promise.h>
#include <chrono>
#include <condition_variable>
#include "eden/common/utils/OptionSet.h"
#include "watchman/thirdparty/libart/src/art.h"
#include "watchman/watchman_string.h"

struct watchman_dir;

namespace watchman {

struct PendingFlags : facebook::eden::OptionSet<PendingFlags, uint8_t> {
  using OptionSet::OptionSet;
  static const NameTable table;
};

/**
 * Set when this change requires a recursive scan of its children.
 *
 * If an entry is recursive, then the IO thread will stat its children too.
 *
 * PendingCollection uses this to prune unnecessary notifications: if a parent
 * entry is already flagged as requiring a recursive scan, then children can be
 * pruned.
 */
constexpr inline auto W_PENDING_RECURSIVE = PendingFlags::raw(1);

/**
 * Set when this change requires a non-recursive scan of its children.
 *
 * Some watchers, notably FSEvents in its default mode, only report changes to
 * directories and expect Watchman to enumerate and stat their children.
 *
 * This flag indicates to the IO thread that it must do such a scan.
 */
constexpr inline auto W_PENDING_NONRECURSIVE_SCAN = PendingFlags::raw(2);

/**
 * This change event came from a watcher.
 *
 * Crawler uses this to distinguish between crawler-originated events and
 * watcher-originated events.
 *
 * iothread uses this flag to detect whether cookie events were discovered via a
 * crawl or watcher.
 */
constexpr inline auto W_PENDING_VIA_NOTIFY = PendingFlags::raw(4);

/**
 * Set by the IO thread when it adds new pending paths while crawling.
 *
 * Crawl-only paths do not cause PendingCollection pruning. Also affects cookie
 * discovery.
 *
 * Sort of exclusive with VIA_NOTIFY...
 */
constexpr inline auto W_PENDING_CRAWL_ONLY = PendingFlags::raw(8);

/**
 * Set when the watcher is desynced and may have missed filesystem events. The
 * watcher is no longer guaranteed to report every file or directory, which
 * could prevent cookies from being observed. W_PENDING_RECURSIVE flag should
 * also be set alongside it to force an recrawl of the passed in directory.
 * Cookies will not be considered when this flag is set.
 */
constexpr inline auto W_PENDING_IS_DESYNCED = PendingFlags::raw(16);

/**
 * Set when the processPath() is triggered by recursive parallel walk.
 * processPath() -> statPath() should avoid appending to PendingChanges.
 * Missing pre_stat can be treated as deletion without extra stat().
 */
constexpr inline auto W_PENDING_VIA_PWALK = PendingFlags::raw(32);

/**
 * Represents a change notification from the Watcher.
 */
struct PendingChange {
  w_string path;
  std::chrono::system_clock::time_point now;
  PendingFlags flags;
};

struct watchman_pending_fs : watchman::PendingChange {
  // We own the next entry and will destroy that chain when we
  // are destroyed.
  std::shared_ptr<watchman_pending_fs> next;

  watchman_pending_fs(
      w_string path,
      std::chrono::system_clock::time_point now,
      PendingFlags flags)
      : PendingChange{std::move(path), now, flags} {}

 private:
  // Only used for unlinking during pruning.
  std::weak_ptr<watchman_pending_fs> prev;
  friend class PendingChanges;
};

/**
 * Holds a linked list of watchman_pending_fs instances and a trie that
 * efficiently prunes redundant changes.
 *
 * PendingChanges is only intended to be accessed by one thread at a time.
 * If you would like to use a single pending changes object accross
 * threads, you should use PendingCollection which puts a lock around
 * accesses to the unerlying PendingChanges object. If you only intend to
 * use the object on one thread, then you can use PendingChanges directly.
 */
class PendingChanges {
 public:
  PendingChanges() = default;
  PendingChanges(PendingChanges&&) = delete;
  PendingChanges& operator=(PendingChanges&&) = delete;

  /**
   * Erase all elements from the collection.
   *
   * Any pending syncs will be fulfilled with a BrokenPromise error.
   */
  void clear();

  /**
   * Add a pending entry.  Will consolidate an existing entry with the same
   * name. The caller must own the collection lock.
   */
  void add(
      const w_string& path,
      std::chrono::system_clock::time_point now,
      PendingFlags flags);
  void add(
      watchman_dir* dir,
      const char* name,
      std::chrono::system_clock::time_point now,
      PendingFlags flags);

  /**
   * Add a sync request. The consumer of this sync should fulfill it after
   * processing all of the pending items.
   */
  void addSync(folly::Promise<folly::Unit> promise);

  /**
   * Merge the full contents of `chain` into this collection. They are usually
   * from a stealItems() call.
   *
   * `chain` is consumed -- the links are broken.
   */
  void append(
      std::shared_ptr<watchman_pending_fs> chain,
      std::vector<folly::Promise<folly::Unit>> syncs);

  /* Moves the head of the chain of items to the caller.
   * The tree is cleared and the caller owns the whole chain */
  std::shared_ptr<watchman_pending_fs> stealItems();

  std::vector<folly::Promise<folly::Unit>> stealSyncs();

  /**
   * Returns true if there are no items or syncs.
   */
  bool empty() const;

  /**
   * Returns the number of unique pending items in the collection. Does not
   * include sync requests.
   */
  uint32_t getPendingItemCount() const;

  void startRefusingSyncs(std::string_view reason);

 protected:
  art_tree<std::shared_ptr<watchman_pending_fs>, w_string> tree_;
  std::shared_ptr<watchman_pending_fs> pending_;
  std::vector<folly::Promise<folly::Unit>> syncs_;
  bool refuseSyncs_{false}; // true if we should refuse to add any more syncs
  std::string refuseSyncsReason_{};

 private:
  void maybePruneObsoletedChildren(w_string path, PendingFlags flags);
  inline void consolidateItem(watchman_pending_fs* p, PendingFlags flags);
  bool isObsoletedByContainingDir(const w_string& path);
  inline void linkHead(std::shared_ptr<watchman_pending_fs>&& p);
  inline void unlinkItem(std::shared_ptr<watchman_pending_fs>& p);
};

class PendingCollectionBase : public PendingChanges {
 public:
  explicit PendingCollectionBase(std::condition_variable& cond);
  PendingCollectionBase(PendingCollectionBase&&) = delete;
  PendingCollectionBase& operator=(PendingCollectionBase&&) = delete;

  /**
   * Sets the pinged flag to true and wakes any waiting threads.
   */
  void ping();

  /**
   * Sets the pinged flag to false.
   * Returns true if previously pinged or PendingChanges is non-empty.
   */
  bool checkAndResetPinged();

 private:
  std::condition_variable& cond_;
  bool pinged_{false};
};

class PendingCollection
    : public folly::Synchronized<PendingCollectionBase, std::mutex> {
 public:
  PendingCollection();

  /**
   * If previously pinged or non-empty, returns a locked PendingCollectionBase.
   * Otherwise, waits up to timeoutms (or indefinitely if -1 ms) for a ping().
   *
   * The internal pinged state is always false after this call.
   */
  LockedPtr lockAndWait(std::chrono::milliseconds timeoutms);

 private:
  // Notified on ping().
  std::condition_variable cond_;
};

// Since the tree has no internal knowledge about path structures, when we
// search for "foo/bar" it may return a prefix match for an existing node
// with the key "foo/bard".  We use this function to test whether the string
// exactly matches the input ("foo/bar") or whether it has a slash as the next
// character after the common prefix ("foo/bar/" as a prefix).
bool is_path_prefix(
    const char* path,
    size_t path_len,
    const char* other,
    size_t common_prefix);

inline bool is_path_prefix(const w_string& key, const w_string& root) {
  return is_path_prefix(key.data(), key.size(), root.data(), root.size());
}

} // namespace watchman
