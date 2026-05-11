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

#pragma once

#include <memory>

#include <folly/container/F14Map.h>
#include <folly/io/async/DelayedDestruction.h>

namespace apache::thrift::fast_thrift::security {

class FizzHandshakeHelper;

/**
 * Owns in-flight FizzHandshakeHelper instances for a single EventBase.
 *
 * Not thread-safe — all access must be from the owning EventBase thread.
 * Helpers self-remove on completion via remove(); cancelAll() is invoked
 * during shutdown to abort any handshakes still in progress by triggering
 * each helper's synchronous terminal-callback path.
 */
class PendingHandshakes {
 public:
  using HelperPtr = std::
      unique_ptr<FizzHandshakeHelper, folly::DelayedDestruction::Destructor>;

  PendingHandshakes() = default;
  ~PendingHandshakes() = default;
  PendingHandshakes(const PendingHandshakes&) = delete;
  PendingHandshakes& operator=(const PendingHandshakes&) = delete;
  PendingHandshakes(PendingHandshakes&&) = delete;
  PendingHandshakes& operator=(PendingHandshakes&&) = delete;

  void add(HelperPtr helper) {
    auto* raw = helper.get();
    helpers_.emplace(raw, std::move(helper));
  }

  // Removes and destroys the helper. Safe to call from the helper's own
  // success/error callback because FizzHandshakeHelper is DelayedDestruction:
  // any DestructorGuard live on the call stack defers actual deletion until
  // the guard pops.
  void remove(FizzHandshakeHelper* helper) { helpers_.erase(helper); }

  // Synchronously cancels all in-flight handshakes. Each helper's cancel()
  // must invoke a terminal callback synchronously, which routes through
  // finish() → remove(this); we drain into a local map first so the
  // callbacks erasing from helpers_ don't perturb iteration.
  void cancelAll();

  size_t size() const { return helpers_.size(); }

 private:
  folly::F14FastMap<FizzHandshakeHelper*, HelperPtr> helpers_;
};

} // namespace apache::thrift::fast_thrift::security
