/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>
#include <unordered_map>
#include "watchman/Clock.h"
#include "watchman/CookieSync.h"
#include "watchman/IgnoreSet.h"
#include "watchman/PendingCollection.h"
#include "watchman/PubSub.h"
#include "watchman/Serde.h"
#include "watchman/WatchmanConfig.h"
#include "watchman/fs/FileSystem.h"
#include "watchman/thirdparty/jansson/jansson.h"
#include "watchman/watchman_string.h"

/* Prune out nodes that were deleted roughly 12-36 hours ago */
#define DEFAULT_GC_AGE (86400 / 2)
#define DEFAULT_GC_INTERVAL 86400

namespace watchman {

class Root;
struct TriggerCommand;
class QueryableView;
struct QueryContext;
class PerfSample;

enum ClientStateDisposition {
  PendingEnter,
  Asserted,
  PendingLeave,
  Done,
};

class ClientStateAssertion {
 public:
  const std::shared_ptr<Root> root; // Holds a ref on the root
  const w_string name;
  // locking: You must hold root->assertedStates lock to access this member
  ClientStateDisposition disposition{PendingEnter};

  // Deferred payload to send when this assertion makes it to the front
  // of the queue.
  // locking: You must hold root->assertedStates lock to access this member.
  std::optional<json_ref> enterPayload;

  ClientStateAssertion(const std::shared_ptr<Root>& root, const w_string& name)
      : root(root), name(name) {}
};

class ClientStateAssertions {
 public:
  /** Returns true if `assertion` is the front instance in the queue
   * of assertions that match assertion->name */
  bool isFront(const std::shared_ptr<ClientStateAssertion>& assertion) const;

  /** Returns true if `assertion` currently has an Asserted disposition */
  bool isStateAsserted(w_string stateName) const;

  /** Add assertion to the queue of assertions for assertion->name.
   * Throws if the named state is already asserted or if there is
   * a pending assertion for that state. */
  void queueAssertion(std::shared_ptr<ClientStateAssertion> assertion);

  /** remove assertion from the queue of assertions for assertion->name.
   * If no more assertions remain in that named queue then the queue is
   * removed.
   * If the removal of an assertion causes the new front of that queue
   * to occupied by an assertion with Asserted disposition, generates a
   * broadcast of its enterPayload.
   */
  bool removeAssertion(const std::shared_ptr<ClientStateAssertion>& assertion);

  /** Returns some diagnostic information that is used by
   * the integration tests. */
  json_ref debugStates() const;

 private:
  /** states_ maps from a state name to a queue of assertions with
   * various dispositions */
  std::
      unordered_map<w_string, std::deque<std::shared_ptr<ClientStateAssertion>>>
          states_;
};

class RootConfig {
 public:
  /* path to root */
  const w_string root_path;
  /* filesystem type name, as returned by w_fstype() */
  const w_string fs_type;
  const CaseSensitivity case_sensitive;
  const IgnoreSet ignore;
  const FileInformation stat;
};

/**
 * Given the ignore_dirs and ignore_vcs configuration arrays, return a
 * configured IgnoreSet.
 *
 * Normally an implementation detail, but exposed for testing.
 */
IgnoreSet computeIgnoreSet(
    const w_string& root_path,
    const Configuration& config);

struct RootRecrawlInfo : serde::Object {
  int64_t count;
  bool should_recrawl;
  std::optional<w_string> warning;
  w_string reason;
  std::optional<int64_t> started_at;
  std::optional<int64_t> completed_at;
  std::optional<int64_t> stat_count;

  template <typename X>
  void map(X& x) {
    x("count", count);
    x("should-recrawl", should_recrawl);
    x("warning", warning);
    x("reason", reason);
    x("started", started_at);
    x("completed", completed_at);
    x("stats", stat_count);
  }
};

struct RootQueryInfo : serde::Object {
  int64_t elapsed_milliseconds;
  int64_t cookie_sync_duration_milliseconds;
  int64_t generation_duration_milliseconds;
  int64_t render_duration_milliseconds;
  int64_t view_lock_wait_duration_milliseconds;
  w_string state;
  int64_t client_pid;
  std::optional<w_string> request_id;
  std::optional<json_ref> query;
  std::optional<w_string> subscription_name;

  template <typename X>
  void map(X& x) {
    x("elapsed-milliseconds", elapsed_milliseconds);
    x("cookie-sync-duration-milliseconds", cookie_sync_duration_milliseconds);
    x("generation-duration-milliseconds", generation_duration_milliseconds);
    x("render-duration-milliseconds", render_duration_milliseconds);
    x("view-lock-wait-duration-milliseconds",
      view_lock_wait_duration_milliseconds);
    x("state", state);
    x("client-pid", client_pid);
    x("request-id", request_id);
    x.skip_if("query", query, [](const auto& query_2) {
      return !query_2.has_value();
    });
  }
};

struct RootDebugStatus : serde::Object {
  w_string path;
  w_string fstype;
  w_string watcher;
  int64_t uptime;
  bool case_sensitive;
  std::vector<w_string> cookie_prefix;
  std::vector<w_string> cookie_dir;
  std::vector<w_string> cookie_list;
  RootRecrawlInfo recrawl_info;
  std::vector<RootQueryInfo> queries;
  bool done_initial;
  bool cancelled;
  bool enable_parallel_crawl;
  w_string crawl_status;

  template <typename X>
  void map(X& x) {
    x("path", path);
    x("fstype", fstype);
    x("watcher", watcher);
    x("uptime", uptime);
    x("case_sensitive", case_sensitive);
    x("cookie_prefix", cookie_prefix);
    x("cookie_dir", cookie_dir);
    x("cookie_list", cookie_list);
    x("recrawl_info", recrawl_info);
    x("queries", queries);
    x("done_initial", done_initial);
    x("cancelled", cancelled);
    x("crawl-status", crawl_status);
    x("enable_parallel_crawl", enable_parallel_crawl);
  }
};

/**
 * Represents a watched root directory.
 *
 * Lifetime is managed by a shared_ptr. The root is tracked in `watched_roots`
 * until it is cancelled. The count of live roots is tracked by `live_roots`.
 *
 * Watcher threads are detached, and thus cannot be joined. However, they
 * maintain strong references to roots, so we know the threads are stopped when
 * the root becomes unreferenced.
 */
class Root : public RootConfig, public std::enable_shared_from_this<Root> {
 public:
  using SaveGlobalStateHook = void (*)();

  /* map of rule id => struct TriggerCommand */
  folly::Synchronized<
      std::unordered_map<w_string, std::unique_ptr<TriggerCommand>>>
      triggers;

  const std::chrono::steady_clock::time_point startTime =
      std::chrono::steady_clock::now();

  CookieSync cookies;

  /* mutable config items */
  std::atomic<bool> enable_parallel_crawl;

  /* config options loaded via json file */
  std::optional<json_ref> config_file;
  Configuration config;

  const std::chrono::milliseconds trigger_settle{0};
  /**
   * Don't GC more often than this.
   *
   * If zero, then never age out.
   */
  const std::chrono::seconds gc_interval{DEFAULT_GC_INTERVAL};
  /**
   * When GCing, age out files older than this.
   */
  const std::chrono::seconds gc_age{DEFAULT_GC_AGE};
  const std::chrono::seconds idle_reap_age{0};

  const bool allow_crawling_other_mounts;

  // Stream of broadcast unilateral items emitted by this root
  std::shared_ptr<Publisher> unilateralResponses;

  struct RecrawlInfo {
    /* how many times we've had to recrawl */
    uint64_t recrawlCount = 0;
    /* if true, we've decided that we should re-crawl the root
     * for the sake of ensuring consistency */
    bool shouldRecrawl = true;
    // Last recrawl reason
    w_string reason{"startup"};
    // Last ad-hoc warning message
    std::optional<w_string> warning;
    std::chrono::steady_clock::time_point crawlStart;
    std::chrono::steady_clock::time_point crawlFinish;
    // Number of statPath() called during recrawl
    std::shared_ptr<std::atomic<size_t>> statCount;
  };
  folly::Synchronized<RecrawlInfo> recrawlInfo;

  // Why we failed to watch
  std::optional<w_string> failure_reason;

  // State transition counter to allow identification of concurrent state
  // transitions
  std::atomic<uint32_t> stateTransCount{0};
  folly::Synchronized<ClientStateAssertions> assertedStates;

  struct Inner {
    /**
     * Initially false and set to false by the iothread after scheduleRecrawl.
     * Set true after fullCrawl is done.
     *
     * Primarily used by the iothread but this is atomic because other threads
     * sometimes read it to produce log messages.
     */
    std::atomic<bool> done_initial{false};

    /**
     * Set if cancel() has been called. Once true, is never set to false.
     */
    std::atomic<bool> cancelled{false};

    /* map of cursor name => last observed tick value */
    folly::Synchronized<std::unordered_map<w_string, ClockTicks>> cursors;

    /// Set by connection threads and read on the iothread.
    std::atomic<std::chrono::steady_clock::time_point> last_cmd_timestamp{
        std::chrono::steady_clock::time_point{}};

    /// Only accessed on the iothread.
    std::chrono::steady_clock::time_point last_reap_timestamp;
  } inner;

  // For debugging and diagnostic purposes, this set references
  // all outstanding query contexts that are executing against this root.
  // If is only safe to read the query contexts while the queries.rlock()
  // is held, and even then it is only really safe to read fields that
  // are not changed by the query exection.
  folly::Synchronized<std::unordered_set<QueryContext*>> queries;

  /**
   * Returns the view with which this Root was constructed.
   */
  std::shared_ptr<QueryableView> view() const {
    return view_;
  }

  Root(
      FileSystem& fileSystem,
      const w_string& root_path,
      const w_string& fs_type,
      std::optional<json_ref> config_file,
      Configuration config,
      std::shared_ptr<QueryableView> view,
      SaveGlobalStateHook saveGlobalStateHook);
  ~Root();

  void considerAgeOut();
  void performAgeOut(std::chrono::seconds min_age);
  folly::SemiFuture<folly::Unit> waitForSettle(
      std::chrono::milliseconds settle_period);
  CookieSync::SyncResult syncToNow(std::chrono::milliseconds timeout);
  void scheduleRecrawl(const char* why);
  void recrawlTriggered(const char* why);

  // Requests cancellation of the root.
  // Returns true if this request caused the root cancellation, false
  // if it was already in the process of being cancelled.
  bool cancel();

  // Returns true if the caller should stop the watch.
  bool considerReap();
  bool removeFromWatched();
  void stopThreads();
  bool stopWatch();
  json_ref triggerListToJson() const;

  static std::vector<RootDebugStatus> getStatusForAllRoots();
  RootDebugStatus getStatus() const;

  // Annotate the sample with some standard metadata taken from a root.
  void addPerfSampleMetadata(PerfSample& sample) const;

  SaveGlobalStateHook getSaveGlobalStateHook() const {
    return saveGlobalStateHook_;
  }

 private:
  const std::shared_ptr<QueryableView> view_;

  /// A hook that allows saving Watchman's state after key operations. Usually
  /// holds w_state_save.
  SaveGlobalStateHook saveGlobalStateHook_;
};

} // namespace watchman
