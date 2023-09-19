/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "watchman/root/watchlist.h"
#include <fmt/core.h>
#include <folly/Synchronized.h>
#include <vector>
#include "watchman/QueryableView.h"
#include "watchman/TriggerCommand.h"
#include "watchman/query/Query.h"
#include "watchman/query/QueryContext.h"
#include "watchman/root/Root.h"

namespace watchman {

folly::Synchronized<std::unordered_map<w_string, std::shared_ptr<Root>>>
    watched_roots;
std::atomic<long> live_roots{0};

bool Root::removeFromWatched() {
  auto map = watched_roots.wlock();
  auto it = map->find(root_path);
  if (it == map->end()) {
    return false;
  }
  // it's possible that the root has already been removed and replaced with
  // another, so make sure we're removing the right object
  if (it->second.get() == this) {
    map->erase(it);
    return true;
  }
  return false;
}

bool findEnclosingRoot(
    const w_string& fileName,
    w_string_piece& prefix,
    w_string_piece& relativePath) {
  std::shared_ptr<Root> root;
  auto name = fileName.piece();
  {
    auto map = watched_roots.rlock();
    for (const auto& it : *map) {
      auto root_name = it.first;
      if (name.startsWith(root_name.piece()) &&
          (name.size() == root_name.size() /* exact match */ ||
           is_slash(name[root_name.size()] /* dir container matches */))) {
        root = it.second;
        prefix = root_name.piece();
        if (name.size() == root_name.size()) {
          relativePath = w_string_piece();
        } else {
          relativePath = name;
          relativePath.advance(root_name.size() + 1);
        }
        return true;
      }
    }
  }
  return false;
}

json_ref w_root_stop_watch_all() {
  std::vector<Root*> roots;
  std::vector<json_ref> stopped;

  Root::SaveGlobalStateHook saveGlobalStateHook = nullptr;

  // Funky looking loop because root->cancel() needs to acquire the
  // watched_roots wlock and will invalidate any iterators we might
  // otherwise have held.  Therefore we just loop until the map is
  // empty.
  while (true) {
    std::shared_ptr<Root> root;

    {
      auto map = watched_roots.wlock();
      if (map->empty()) {
        break;
      }

      auto it = map->begin();
      root = it->second;
    }

    root->cancel();
    if (!saveGlobalStateHook) {
      saveGlobalStateHook = root->getSaveGlobalStateHook();
    } else {
      // This assumes there is only one hook per application, rather than
      // independent hooks per root. That's true today because every root holds
      // w_state_save.
      w_assert(
          saveGlobalStateHook == root->getSaveGlobalStateHook(),
          "all roots must contain the same saveGlobalStateHook");
    }
    stopped.push_back(w_string_to_json(root->root_path));
  }

  if (saveGlobalStateHook) {
    saveGlobalStateHook();
  }

  return json_array(std::move(stopped));
}

json_ref w_root_watch_list_to_json() {
  std::vector<json_ref> arr;

  auto map = watched_roots.rlock();
  for (const auto& it : *map) {
    auto root = it.second;
    arr.push_back(w_string_to_json(root->root_path));
  }

  return json_array(std::move(arr));
}

std::vector<RootDebugStatus> Root::getStatusForAllRoots() {
  std::vector<RootDebugStatus> result;

  auto map = watched_roots.rlock();
  result.reserve(map->size());
  for (const auto& [name, root] : *map) {
    result.push_back(root->getStatus());
  }

  return result;
}

RootDebugStatus Root::getStatus() const {
  RootDebugStatus obj;
  auto now = std::chrono::steady_clock::now();

  auto cookie_array = cookies.getOutstandingCookieFileList();

  std::string crawl_status;
  RootRecrawlInfo recrawl_info;
  {
    auto info = recrawlInfo.rlock();
    recrawl_info.count = info->recrawlCount;
    recrawl_info.should_recrawl = info->shouldRecrawl;
    recrawl_info.reason = info->reason;
    if (info->warning) {
      recrawl_info.warning = info->warning;
    }
    std::shared_ptr<std::atomic<size_t>> stat_count = info->statCount;
    if (stat_count) {
      recrawl_info.stat_count = stat_count->load(std::memory_order_acquire);
    }

    int64_t finish_ago = std::chrono::duration_cast<std::chrono::milliseconds>(
                             now - info->crawlFinish)
                             .count();
    int64_t start_ago = std::chrono::duration_cast<std::chrono::milliseconds>(
                            now - info->crawlStart)
                            .count();
    if (!inner.done_initial) {
      recrawl_info.started_at = -start_ago;
      crawl_status = fmt::format(
          "{}crawling for {} ms", info->recrawlCount ? "re-" : "", start_ago);
    } else if (info->shouldRecrawl) {
      recrawl_info.completed_at = -finish_ago;
      crawl_status = fmt::format(
          "needs recrawl: {}. Last crawl was {}ms ago",
          info->warning ? info->warning->view() : std::string_view{},
          finish_ago);
    } else {
      recrawl_info.started_at = -start_ago;
      recrawl_info.completed_at = -finish_ago;
      crawl_status = fmt::format(
          "crawl completed {}ms ago, and took {}ms",
          finish_ago,
          start_ago - finish_ago);
    }
  }

  std::vector<RootQueryInfo> query_info;
  {
    auto locked = queries.rlock();
    for (auto& ctx : *locked) {
      auto elapsed = now - ctx->created;

      const char* queryState = "?";
      switch (ctx->state.load()) {
        case QueryContextState::NotStarted:
          queryState = "NotStarted";
          break;
        case QueryContextState::WaitingForCookieSync:
          queryState = "WaitingForCookieSync";
          break;
        case QueryContextState::WaitingForViewLock:
          queryState = "WaitingForViewLock";
          break;
        case QueryContextState::Generating:
          queryState = "Generating";
          break;
        case QueryContextState::Rendering:
          queryState = "Rendering";
          break;
        case QueryContextState::Completed:
          queryState = "Completed";
          break;
      }

      RootQueryInfo info;
      info.elapsed_milliseconds =
          std::chrono::duration_cast<std::chrono::milliseconds>(elapsed)
              .count();
      info.cookie_sync_duration_milliseconds =
          ctx->cookieSyncDuration.load().count();
      info.generation_duration_milliseconds =
          ctx->generationDuration.load().count();
      info.render_duration_milliseconds = ctx->renderDuration.load().count();
      info.view_lock_wait_duration_milliseconds =
          ctx->viewLockWaitDuration.load().count();
      info.state = queryState;
      info.client_pid = ctx->query->clientPid;
      info.request_id = ctx->query->request_id;
      info.query = ctx->query->query_spec;
      if (ctx->query->subscriptionName) {
        info.subscription_name = ctx->query->subscriptionName;
      }

      query_info.push_back(std::move(info));
    }
  }

  std::vector<w_string> cookiePrefix;
  for (const auto& name : cookies.cookiePrefix()) {
    // If cookiePrefix() returned a std::vector, we could move the string here.
    cookiePrefix.push_back(name);
  }

  std::vector<w_string> cookieDirs;
  for (const auto& dir : cookies.cookieDirs()) {
    // If cookieDirs() returned a std::vector, we could move the string here.
    cookieDirs.push_back(dir);
  }

  obj.path = root_path;
  obj.fstype = fs_type;
  obj.watcher = view_->getName();
  obj.uptime =
      std::chrono::duration_cast<std::chrono::seconds>(now - startTime).count();
  obj.case_sensitive = case_sensitive == CaseSensitivity::CaseSensitive;
  obj.cookie_prefix = cookiePrefix;
  obj.cookie_dir = cookieDirs;
  obj.cookie_list = cookie_array;
  obj.recrawl_info = std::move(recrawl_info);
  obj.queries = std::move(query_info);
  obj.done_initial = inner.done_initial;
  obj.cancelled = inner.cancelled;
  obj.crawl_status = w_string{crawl_status.data(), crawl_status.size()};
  obj.enable_parallel_crawl = enable_parallel_crawl;
  return obj;
}

json_ref Root::triggerListToJson() const {
  std::vector<json_ref> arr;
  {
    auto map = triggers.rlock();
    for (const auto& it : *map) {
      const auto& cmd = it.second;
      arr.push_back(cmd->definition);
    }
  }

  return json_array(std::move(arr));
}

void w_root_free_watched_roots() {
  int last, interval;
  time_t started;
  std::vector<std::shared_ptr<Root>> roots;

  // We want to cancel the list of roots, but need to be careful to avoid
  // deadlock; make a copy of the set of roots under the lock...
  {
    auto map = watched_roots.rlock();
    for (const auto& it : *map) {
      roots.emplace_back(it.second);
    }
  }

  // ... and cancel them outside of the lock
  for (auto& root : roots) {
    if (!root->cancel()) {
      root->stopThreads();
    }
  }

  // release them all so that we don't mess with the number of live_roots
  // in the code below.
  roots.clear();

  last = live_roots;
  time(&started);
  logf(DBG, "waiting for roots to cancel and go away {}\n", last);
  interval = 100;
  for (;;) {
    auto current = live_roots.load();
    if (current == 0) {
      break;
    }
    if (time(NULL) > started + 3) {
      logf(ERR, "{} roots were still live at exit\n", current);
      break;
    }
    if (current != last) {
      logf(DBG, "waiting: {} live\n", current);
      last = current;
    }
    /* sleep override */ std::this_thread::sleep_for(
        std::chrono::microseconds(interval));
    interval = std::min(interval * 2, 1000000);
  }

  logf(DBG, "all roots are gone\n");
}

} // namespace watchman

/* vim:ts=2:sw=2:et:
 */
