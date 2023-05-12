/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include <folly/Synchronized.h>
#include <map>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include "watchman/ContentHash.h"
#include "watchman/CookieSync.h"
#include "watchman/PendingCollection.h"
#include "watchman/PerfSample.h"
#include "watchman/QueryableView.h"
#include "watchman/Result.h"
#include "watchman/RingBuffer.h"
#include "watchman/SymlinkTargets.h"
#include "watchman/WatchmanConfig.h"
#include "watchman/fs/DirHandle.h"
#include "watchman/query/FileResult.h"
#include "watchman/watchman_string.h"
#include "watchman/watchman_system.h"

struct watchman_file;

namespace watchman {

class FileSystem;
class RootConfig;
struct GlobTree;
class Watcher;

// Helper struct to hold caches used by the InMemoryView
struct InMemoryViewCaches {
  ContentHashCache contentHashCache;
  SymlinkTargetCache symlinkTargetCache;

  InMemoryViewCaches(
      const w_string& rootPath,
      size_t maxHashes,
      size_t maxSymlinks,
      std::chrono::milliseconds errorTTL);
};

class InMemoryFileResult final : public FileResult {
 public:
  InMemoryFileResult(const watchman_file* file, InMemoryViewCaches& caches);
  std::optional<FileInformation> stat() override;
  std::optional<struct timespec> accessedTime() override;
  std::optional<struct timespec> modifiedTime() override;
  std::optional<struct timespec> changedTime() override;
  std::optional<size_t> size() override;
  w_string_piece baseName() override;
  w_string_piece dirName() override;
  std::optional<bool> exists() override;
  std::optional<ResolvedSymlink> readLink() override;
  std::optional<ClockStamp> ctime() override;
  std::optional<ClockStamp> otime() override;
  std::optional<FileResult::ContentHash> getContentSha1() override;
  void batchFetchProperties(
      const std::vector<std::unique_ptr<FileResult>>& files) override;

 private:
  const watchman_file* file_;
  std::optional<w_string> dirName_;
  InMemoryViewCaches& caches_;
  std::optional<ResolvedSymlink> symlinkTarget_;
  Result<FileResult::ContentHash> contentSha1_;
};

/**
 * In-memory data structure representing Watchman's understanding of the watched
 * root. Files are ordered in a linked recency index as well as hierarchically
 * from the root.
 */
class ViewDatabase {
 public:
  explicit ViewDatabase(const w_string& root_path);

  watchman_file* getLatestFile() const {
    return latestFile_;
  }

  ino_t getRootInode() const {
    return rootInode_;
  }

  void setRootInode(ino_t ino) {
    rootInode_ = ino;
  }

  watchman_dir* resolveDir(const w_string& dirname, bool create);

  const watchman_dir* resolveDir(const w_string& dirname) const;

  /**
   * Returns the direct child file named name if it already exists, else creates
   * that entry and returns it.
   */
  watchman_file* getOrCreateChildFile(
      watchman_dir* dir,
      const w_string& file_name,
      ClockStamp ctime);

  /**
   * Updates the otime for the file and bubbles it to the front of recency
   * index.
   */
  void markFileChanged(watchman_file* file, ClockStamp otime);

  /**
   * Mark a directory as being removed from the view. Marks the contained set of
   * files as deleted. If recursive is true, is recursively invoked on child
   * dirs.
   */
  void markDirDeleted(watchman_dir* dir, ClockStamp otime, bool recursive);

 private:
  void insertAtHeadOfFileList(struct watchman_file* file);

  const w_string rootPath_;

  /* the most recently changed file */
  watchman_file* latestFile_ = nullptr;

  std::unique_ptr<watchman_dir> rootDir_;

  // Inode number for the root dir.  This is used to detect what should
  // be impossible situations, but is needed in practice to workaround
  // eg: BTRFS not delivering all events for subvolumes
  ino_t rootInode_{0};
};

/**
 * Keeps track of the state of the filesystem in-memory and drives a notify
 * thread which consumes events from the watcher.
 */
class InMemoryView final : public QueryableView {
 public:
  InMemoryView(
      FileSystem& fileSystem,
      const w_string& root_path,
      Configuration config,
      std::shared_ptr<Watcher> watcher);
  ~InMemoryView() override;

  InMemoryView(InMemoryView&&) = delete;
  InMemoryView& operator=(InMemoryView&&) = delete;

  ClockPosition getMostRecentRootNumberAndTickValue() const override;
  ClockTicks getLastAgeOutTickValue() const override;
  std::chrono::system_clock::time_point getLastAgeOutTimeStamp() const override;
  w_string getCurrentClockString() const override;

  ClockStamp getClock(std::chrono::system_clock::time_point now) const {
    return ClockStamp{
        mostRecentTick_.load(std::memory_order_acquire),
        std::chrono::system_clock::to_time_t(now),
    };
  }

  void ageOut(PerfSample& sample, std::chrono::seconds minAge) override;

  folly::SemiFuture<folly::Unit> waitForSettle(
      std::chrono::milliseconds settle_period) override;
  CookieSync::SyncResult syncToNow(
      const std::shared_ptr<Root>& root,
      std::chrono::milliseconds timeout) override;

  /**
   * Write cookies to the working copy and wait to see them.
   *
   * The returned future will complete when all the cookies written to the
   * working copy have been noticed by the underlying watcher.
   */
  folly::SemiFuture<CookieSync::SyncResult> sync(
      const std::shared_ptr<Root>& root) override;

  bool doAnyOfTheseFilesExist(
      const std::vector<w_string>& fileNames) const override;

  void timeGenerator(const Query* query, QueryContext* ctx) const override;

  void pathGenerator(const Query* query, QueryContext* ctx) const override;

  void globGenerator(const Query* query, QueryContext* ctx) const override;

  void allFilesGenerator(const Query* query, QueryContext* ctx) const override;

  /**
   * Returns a SemiFuture that completes when any pending recrawls are
   * completed. The primary use of this is so that "watch-project" doesn't send
   * its return PDU to the client until after the initial crawl is complete.
   * Note that a recrawl can happen at any point, so this is a bit of a weak
   * promise that a query can be immediately executed, but is good enough
   * assuming that the system isn't in a perpetual state of recrawl.
   */
  folly::SemiFuture<folly::Unit> waitUntilReadyToQuery() override;

  void startThreads(const std::shared_ptr<Root>& root) override;
  void stopThreads() override;
  void wakeThreads() override;
  void clientModeCrawl(const std::shared_ptr<Root>& root);

  const w_string& getName() const override;
  const std::shared_ptr<Watcher>& getWatcher() const;
  json_ref getWatcherDebugInfo() const override;
  void clearWatcherDebugInfo() override;
  json_ref getViewDebugInfo() const;
  void clearViewDebugInfo();

  // If content cache warming is configured, do the warm up now
  void warmContentCache();

  InMemoryViewCaches& debugAccessCaches() const {
    return caches_;
  }

 private:
  CookieSync::SyncResult syncToNowCookies(
      const std::shared_ptr<Root>& root,
      std::chrono::milliseconds timeout);

  // Returns the erased file's otime.
  ClockStamp ageOutFile(
      std::unordered_set<w_string>& dirs_to_erase,
      watchman_file* file);

  // When a watcher is desynced, it sets the W_PENDING_IS_DESYNCED flag, and the
  // crawler will set these recursively. If one of these flag is set,
  // processPending will return IsDesynced::Yes and it is expected that the
  // caller will abort all pending cookies after processAllPending returns.
  enum class IsDesynced { Yes, No };

  /** Recursively walks files under a specified dir */
  void dirGenerator(
      const Query* query,
      QueryContext* ctx,
      const watchman_dir* dir,
      uint32_t depth) const;
  void globGeneratorTree(
      QueryContext* ctx,
      const GlobTree* node,
      const struct watchman_dir* dir) const;
  void globGeneratorDoublestar(
      QueryContext* ctx,
      const struct watchman_dir* dir,
      const GlobTree* node,
      const char* dir_name,
      uint32_t dir_name_len) const;

  void notifyThread(const std::shared_ptr<Root>& root);

  // BEGIN IOTHREAD

  void ioThread(const std::shared_ptr<Root>& root);

  // Consume entries from `pending` and apply them to the InMemoryView. Any new
  // pending paths generated by processPath will be crawled before
  // processAllPending returns.
  IsDesynced processAllPending(
      const std::shared_ptr<Root>& root,
      ViewDatabase& view,
      PendingChanges& pending);

  void processPath(
      const std::shared_ptr<Root>& root,
      ViewDatabase& view,
      PendingChanges& coll,
      const PendingChange& pending,
      const FileInformation* pre_stat,
      std::vector<w_string>& pendingCookies);

  /**
   * Crawl the given directory. Any cookies discovered during the crawl are
   * appended to pendingCookies.
   *
   * Allowed flags:
   *  - W_PENDING_RECURSIVE: the directory will be recursively crawled,
   *  - W_PENDING_VIA_NOTIFY when the watcher only supports directory
   *    notification (W_PENDING_NONRECURSIVE_SCAN), this will stat all
   *    the files and directories contained in the passed in directory and stop.
   */
  void crawler(
      const std::shared_ptr<Root>& root,
      ViewDatabase& view,
      PendingChanges& coll,
      const PendingChange& pending,
      std::vector<w_string>& pendingCookies);

  /**
   * Crawl the given directory recursively using ParallelWalker.
   *
   * W_PENDING_RECURSIVE must be set.
   */
  void crawlerParallel(
      const std::shared_ptr<Root>& root,
      ViewDatabase& view,
      PendingChanges& coll,
      const PendingChange& pending,
      std::vector<w_string>& pendingCookies);

  /**
   * Called on the IO thread. If `pending` is not in the ignored directory list,
   * lstat() the file and update the InMemoryView. This may insert work into
   * `coll` if a directory needs to be rescanned.
   */
  void statPath(
      const Root& root,
      const CookieSync& cookies,
      ViewDatabase& view,
      PendingChanges& coll,
      const PendingChange& pending,
      const FileInformation* pre_stat);

  // END IOTHREAD

 public:
  // This is public so the IO thread state machine can be driven from a test.
  // TODO: Move this logic into a separate class with its own state.

  enum class Continue {
    Stop,
    Continue,
  };

  struct IoThreadState {
    explicit IoThreadState(std::chrono::milliseconds biggestTimeout)
        : biggestTimeout{biggestTimeout} {}

    const std::chrono::milliseconds biggestTimeout;

    PendingChanges localPending;
    std::chrono::milliseconds currentTimeout;

    // When the iothread last processed a pending event from the Watcher.
    std::optional<std::chrono::steady_clock::time_point> lastUnsettle;
  };

  // Returns a reference to the ViewDatabase without synchronizing on the mutex.
  // DO NOT USE OUTSIDE OF SINGLE-THREADED TESTS.
  ViewDatabase& unsafeAccessViewDatabase() {
    return view_.unsafeGetUnlocked();
  }

  // Used by tests to inject events into the iothread.
  PendingCollection& unsafeAccessPendingFromWatcher() {
    return pendingFromWatcher_;
  }

  // Returns whether IO thread should stop.
  Continue stepIoThread(
      const std::shared_ptr<Root>& root,
      IoThreadState& state,
      PendingCollection& pendingFromWatcher);

 private:
  void fullCrawl(
      const std::shared_ptr<Root>& root,
      PendingCollection& pendingFromWatcher,
      PendingChanges& localPending);

  // Performs settle-time actions.
  // Returns whether the root was reaped and the IO thread should terminate.
  Continue doSettleThings(Root& root, IoThreadState& state);

  FileSystem& fileSystem_;
  const Configuration config_;

  folly::Synchronized<ViewDatabase> view_;
  // The most recently observed tick value of an item in the view
  // Only incremented by the iothread, but may be read by other threads.
  std::atomic<ClockTicks> mostRecentTick_{1};
  const ClockRoot rootNumber_{0};
  const w_string rootPath_;

  ClockTicks lastAgeOutTick_{0};
  // This is system_clock instead of steady_clock because it's compared with a
  // file's otime.
  std::chrono::system_clock::time_point lastAgeOutTimestamp_{};

  using PendingSettles =
      std::multimap<std::chrono::milliseconds, folly::Promise<folly::Unit>>;

  /**
   * Holds promises that are fulfilled when the IO thread has settled for the
   * desired amount of time.
   *
   * Sorted by settle period.
   */
  folly::Synchronized<PendingSettles> pendingSettles_;

  /*
   * Queue of items that we need to stat/process.
   *
   * Populated by both the IO thread (fullCrawl), the notify thread (from the
   * watcher), and anything that calls waitUntilReadyToQuery.
   */
  PendingCollection pendingFromWatcher_;

  std::atomic<bool> stopThreads_{false};
  std::shared_ptr<Watcher> watcher_;

  // mutable because we pass a reference to other things from inside
  // const methods
  mutable InMemoryViewCaches caches_;

  // Should we warm the cache when we settle?
  bool enableContentCacheWarming_{false};
  // How many of the most recent files to warm up when settling?
  size_t maxFilesToWarmInContentCache_{1024};
  // Do not warm up files whose size is greater than this. A size of 0 is
  // equivalent to unlimited.
  int64_t maxFileSizeToWarmInContentCache_{10 * 1024 * 1024};
  // If true, we will wait for the items to be hashed before
  // dispatching the settle to watchman clients
  bool syncContentCacheWarming_{false};
  // Remember what we've already warmed up
  uint32_t lastWarmedTick_{0};

  struct PendingChangeLogEntry {
    PendingChangeLogEntry() noexcept {
      // time_point is not noexcept so this can't be defaulted.
    }
    explicit PendingChangeLogEntry(
        const PendingChange& pc,
        std::error_code errcode,
        const FileInformation& st) noexcept;

    json_ref asJsonValue() const;

    // 55 should cover many filenames.
    static constexpr size_t kPathLength = 55;

    // fields from PendingChange
    std::chrono::system_clock::time_point now;
    PendingFlags::UnderlyingType pending_flags;
    char path_tail[kPathLength];

    // results of calling getFileInformation
    int32_t errcode;
    mode_t mode;
    off_t size;
    time_t mtime;
  };

  static_assert(88 == sizeof(PendingChangeLogEntry));

  // If set, paths processed by processPending are logged here.
  std::unique_ptr<RingBuffer<PendingChangeLogEntry>> processedPaths_;

  // Track statPath() count during fullCrawl(). Used to report progress.
  std::shared_ptr<std::atomic<size_t>> fullCrawlStatCount_;
};

} // namespace watchman
