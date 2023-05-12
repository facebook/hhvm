/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <fmt/core.h>
#include <folly/String.h>
#include "watchman/Logging.h"
#include "watchman/QueryableView.h"
#include "watchman/TriggerCommand.h"
#include "watchman/fs/DirHandle.h"
#include "watchman/fs/FSDetect.h"
#include "watchman/root/Root.h"
#include "watchman/root/watchlist.h"

namespace watchman {

namespace {
/// Idle out watches that haven't had activity in several days
inline constexpr json_int_t kDefaultReapAge = 86400 * 5;
inline constexpr json_int_t kDefaultSettlePeriod = 20;
} // namespace

void ClientStateAssertions::queueAssertion(
    std::shared_ptr<ClientStateAssertion> assertion) {
  // Check to see if someone else has or had a pending claim for this
  // state and reject the attempt in that case
  auto state_q = states_.find(assertion->name);
  if (state_q != states_.end() && !state_q->second.empty()) {
    auto disp = state_q->second.back()->disposition;
    if (disp == ClientStateDisposition::PendingEnter ||
        disp == ClientStateDisposition::Asserted) {
      throw std::runtime_error(fmt::format(
          "state {} is already Asserted or PendingEnter", assertion->name));
    }
  }
  states_[assertion->name].push_back(assertion);
}

json_ref ClientStateAssertions::debugStates() const {
  std::vector<json_ref> states;
  for (const auto& state_q : states_) {
    for (const auto& state : state_q.second) {
      auto obj = json_object();
      obj.set("name", w_string_to_json(state->name));
      switch (state->disposition) {
        case ClientStateDisposition::PendingEnter:
          obj.set("state", w_string_to_json("PendingEnter"));
          break;
        case ClientStateDisposition::Asserted:
          obj.set("state", w_string_to_json("Asserted"));
          break;
        case ClientStateDisposition::PendingLeave:
          obj.set("state", w_string_to_json("PendingLeave"));
          break;
        case ClientStateDisposition::Done:
          obj.set("state", w_string_to_json("Done"));
          break;
      }
      states.push_back(std::move(obj));
    }
  }
  return json_array(std::move(states));
}

bool ClientStateAssertions::removeAssertion(
    const std::shared_ptr<ClientStateAssertion>& assertion) {
  auto it = states_.find(assertion->name);
  if (it == states_.end()) {
    return false;
  }

  auto& queue = it->second;
  for (auto assertionIter = queue.begin(); assertionIter != queue.end();
       ++assertionIter) {
    if (*assertionIter == assertion) {
      assertion->disposition = ClientStateDisposition::Done;
      queue.erase(assertionIter);

      // If there are no more entries queued with this name, remove
      // the name from the states map.
      if (queue.empty()) {
        states_.erase(it);
      } else {
        // Now check to see who is at the front of the queue.  If
        // they are set to asserted and have a payload assigned, they
        // are a state-enter that is pending broadcast of the assertion.
        // We couldn't send it earlier without risking out of order
        // delivery wrt. vacating states.
        auto front = queue.front();
        if (front->disposition == ClientStateDisposition::Asserted &&
            front->enterPayload) {
          front->root->unilateralResponses->enqueue(
              std::move(*front->enterPayload));
          front->enterPayload = std::nullopt;
        }
      }
      return true;
    }
  }

  return false;
}

bool ClientStateAssertions::isFront(
    const std::shared_ptr<ClientStateAssertion>& assertion) const {
  auto it = states_.find(assertion->name);
  if (it == states_.end()) {
    return false;
  }
  auto& queue = it->second;
  if (queue.empty()) {
    return false;
  }
  return queue.front() == assertion;
}

bool ClientStateAssertions::isStateAsserted(w_string stateName) const {
  auto it = states_.find(stateName);
  if (it == states_.end()) {
    return false;
  }
  auto& queue = it->second;
  for (auto& state : queue) {
    if (state->disposition == Asserted) {
      return true;
    }
  }
  return false;
}

namespace {

json_ref getIgnoreVcs(const Configuration& config) {
  std::optional<json_ref> ignores = config.get("ignore_vcs");
  if (!ignores) {
    // default to a well-known set of vcs's
    return json_array(
        {typed_string_to_json(".git"),
         typed_string_to_json(".svn"),
         typed_string_to_json(".hg")});
  }

  if (!ignores->isArray()) {
    throw std::runtime_error("ignore_vcs must be an array of strings");
  }

  return *ignores;
}

/**
 * Returns which directory should be used for cookies. Returns the first
 * directory in ignore_vcs that exists. Otherwise, returns the root_path.
 */
w_string computeCookieDir(
    const w_string& root_path,
    const Configuration& config,
    CaseSensitivity case_sensitive,
    const IgnoreSet& ignore) {
  auto ignores = getIgnoreVcs(config);
  for (auto& jignore : ignores.array()) {
    if (!jignore.isString()) {
      throw std::runtime_error("ignore_vcs must be an array of strings");
    }

    auto fullname = w_string::pathCat({root_path, json_to_w_string(jignore)});
    // if we are completely ignoring this dir, we have nothing more to
    // do here
    if (ignore.isIgnoreDir(fullname)) {
      continue;
    }

    FileInformation info;
    try {
      info = getFileInformation(fullname.c_str(), case_sensitive);
    } catch (const std::exception&) {
      continue;
    }

    if (info.isDir()) {
      // root/{.hg,.git,.svn}
      return fullname;
    }
  }

  return root_path;
}

} // namespace

IgnoreSet computeIgnoreSet(
    const w_string& root_path,
    const Configuration& config) {
  IgnoreSet result;

  if (auto ignores = config.get("ignore_dirs")) {
    if (!ignores->isArray()) {
      logf(ERR, "ignore_dirs must be an array of strings\n");
    } else {
      auto& arr = ignores->array();
      for (size_t i = 0; i < arr.size(); i++) {
        auto& jignore = arr[i];

        if (!jignore.isString()) {
          logf(ERR, "ignore_dirs must be an array of strings\n");
          continue;
        }

        auto name = json_to_w_string(jignore);
        auto fullname = w_string::pathCat({root_path, name});
        result.add(fullname, false);
        logf(DBG, "ignoring {} recursively\n", fullname);
      }
    }
  }

  auto ignores = getIgnoreVcs(config);
  for (auto& jignore : ignores.array()) {
    if (!jignore.isString()) {
      throw std::runtime_error("ignore_vcs must be an array of strings");
    }

    auto fullname = w_string::pathCat({root_path, json_to_w_string(jignore)});

    // if we are completely ignoring this dir, we have nothing more to
    // do here
    if (result.isIgnoreDir(fullname)) {
      continue;
    }
    result.add(fullname, true);
  }

  return result;
}

Root::Root(
    FileSystem& fileSystem,
    const w_string& root_path,
    const w_string& fs_type,
    std::optional<json_ref> config_file,
    Configuration config_,
    std::shared_ptr<QueryableView> view,
    SaveGlobalStateHook saveGlobalStateHook)
    : RootConfig{
          root_path,
          fs_type,
          getCaseSensitivityForPath(root_path.c_str()),
          computeIgnoreSet(root_path, config_),
          fileSystem.getFileInformation(root_path.c_str())},
      cookies(
          fileSystem,
          computeCookieDir(root_path, config_, case_sensitive, ignore)),
      enable_parallel_crawl{config_.getBool("enable_parallel_crawl", false)},
      config_file(std::move(config_file)),
      config(std::move(config_)),
      trigger_settle(int(config.getInt("settle", kDefaultSettlePeriod))),
      gc_interval(
          int(config.getInt("gc_interval_seconds", DEFAULT_GC_INTERVAL))),
      gc_age(int(config.getInt("gc_age_seconds", DEFAULT_GC_AGE))),
      idle_reap_age(
          int(config.getInt("idle_reap_age_seconds", kDefaultReapAge))),
      allow_crawling_other_mounts{config_.getBool("allow_crawling_other_mounts", false)},
      unilateralResponses(std::make_shared<Publisher>()),
      view_{std::move(view)},
      saveGlobalStateHook_{std::move(saveGlobalStateHook)} {
  // This just opens and releases the dir.  If an exception is thrown
  // it will bubble up.
  fileSystem.openDir(root_path.c_str());

  // TODO: This is only exception-safe because the rest of the function is
  // unlikely to throw. Switch to some sort of RAII handle instead.
  ++live_roots;

  inner.last_cmd_timestamp = std::chrono::steady_clock::now();

  if (!view_->requiresCrawl) {
    // This watcher can resolve queries without needing a crawl.
    inner.done_initial = true;
    auto crawlInfo = recrawlInfo.wlock();
    crawlInfo->shouldRecrawl = false;
    crawlInfo->crawlStart = std::chrono::steady_clock::now();
    crawlInfo->crawlFinish = crawlInfo->crawlStart;
  }
}

Root::~Root() {
  logf(DBG, "root: final ref on {}\n", root_path);
  --live_roots;
}

void Root::addPerfSampleMetadata(PerfSample& sample) const {
  // Note: if the root lock isn't held, we may read inaccurate numbers for
  // some of these properties.  We're ok with that, and don't want to force
  // the root lock to be re-acquired just for this.
  auto meta = json_object(
      {{"path", w_string_to_json(root_path)},
       {"recrawl_count", json_integer(recrawlInfo.rlock()->recrawlCount)},
       {"case_sensitive",
        json_boolean(case_sensitive == CaseSensitivity::CaseSensitive)}});

  // During recrawl, the view may be re-assigned.  Protect against
  // reading a nullptr.
  auto view = this->view();
  if (view) {
    meta.set({{"watcher", w_string_to_json(view->getName())}});
  }

  sample.add_meta("root", std::move(meta));
}

} // namespace watchman
