/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "watchman/QueryableView.h"
#include "watchman/TriggerCommand.h"
#include "watchman/root/Root.h"

using namespace watchman;

void Root::recrawlTriggered(const char* why) {
  recrawlInfo.wlock()->recrawlCount++;

  log(ERR, root_path, ": ", why, ": tree recrawl triggered\n");
}

void Root::scheduleRecrawl(const char* why) {
  {
    auto info = recrawlInfo.wlock();

    if (!info->shouldRecrawl) {
      info->recrawlCount++;
      info->reason = why;
      if (!config.getBool("suppress_recrawl_warnings", false)) {
        info->warning = w_string::build(
            "Recrawled this watch ",
            info->recrawlCount,
            " time",
            info->recrawlCount != 1 ? "s" : "",
            ", most recently because:\n",
            why,
            "To resolve, please review the information on\n",
            cfg_get_trouble_url(),
            "#recrawl");
      }

      log(ERR, root_path, ": ", why, ": scheduling a tree recrawl\n");
    }
    info->shouldRecrawl = true;
  }
  view()->wakeThreads();
}

void Root::stopThreads() {
  view()->stopThreads();
}

// Cancels a watch.
bool Root::cancel() {
  if (inner.cancelled.exchange(true, std::memory_order_acq_rel)) {
    // Already cancelled. Return false.
    return false;
  }

  log(DBG, "marked ", root_path, " cancelled\n");

  // The client will fan this out to all matching subscriptions.
  // This happens in listener.cpp.
  unilateralResponses->enqueue(json_object(
      {{"root", w_string_to_json(root_path)}, {"canceled", json_true()}}));

  stopThreads();
  removeFromWatched();

  {
    auto map = triggers.rlock();
    for (auto& it : *map) {
      it.second->stop();
    }
  }

  return true;
}

bool Root::stopWatch() {
  bool stopped = removeFromWatched();

  if (stopped) {
    cancel();
    saveGlobalStateHook_();
  }
  stopThreads();

  return stopped;
}

/* vim:ts=2:sw=2:et:
 */
