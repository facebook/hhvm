/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "watchman/Shutdown.h"
#include <atomic>
#include <vector>
#include "watchman/watchman_stream.h"

namespace {

static std::vector<std::shared_ptr<watchman_event>> listener_thread_events;
static std::atomic<bool> stopping = false;

} // namespace

bool w_is_stopping() {
  return stopping.load(std::memory_order_relaxed);
}

void w_request_shutdown() {
  stopping.store(true, std::memory_order_relaxed);
  // Knock listener thread out of poll/accept
  for (auto& evt : listener_thread_events) {
    evt->notify();
  }
}

void w_push_listener_thread_event(std::shared_ptr<watchman_event> event) {
  listener_thread_events.push_back(std::move(event));
}
