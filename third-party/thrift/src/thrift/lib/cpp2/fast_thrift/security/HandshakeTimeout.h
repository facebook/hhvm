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

#include <chrono>
#include <optional>

#include <glog/logging.h>
#include <folly/io/async/AsyncTimeout.h>

namespace apache::thrift::fast_thrift::security {

/**
 * Copyable absolute-deadline holder for multi-phase work sharing a
 * single budget. Construction captures `now + duration` (or leaves the
 * deadline unset for unbounded); copies share the same absolute deadline
 * and can independently check hasExpired() and arm their own
 * folly::AsyncTimeout via schedule().
 *
 * std::nullopt = unbounded: hasExpired() always false, schedule() is a
 * no-op. DO NOT pass std::optional<ms>{0ms} to mean unbounded — that is
 * an immediate deadline and the constructor CHECKs.
 *
 * Recommended usage at each phase:
 *
 *     if (timeout_.hasExpired()) { failFast(...); return; }
 *     timeout_.schedule(*this);
 *
 * Skipping hasExpired() lets a phase race past an already-expired
 * deadline with no timer armed (no enforcement; not immediate fire).
 */
class HandshakeTimeout {
 public:
  // nullopt = unbounded. Any provided value MUST be > 0; passing 0 is a
  // programming error caught at construction.
  explicit HandshakeTimeout(std::optional<std::chrono::milliseconds> timeout) {
    if (timeout) {
      CHECK_GT(timeout->count(), 0)
          << "HandshakeTimeout: pass std::nullopt for unbounded; "
             "optional(0ms) would expire every connection immediately.";
      deadline_ = std::chrono::steady_clock::now() + *timeout;
    }
  }

  ~HandshakeTimeout() = default;
  HandshakeTimeout(const HandshakeTimeout&) = default;
  HandshakeTimeout& operator=(const HandshakeTimeout&) = default;
  HandshakeTimeout(HandshakeTimeout&&) = default;
  HandshakeTimeout& operator=(HandshakeTimeout&&) = default;

  // True iff a deadline was set AND it is already past.
  // Unbounded (no deadline) is NEVER expired.
  bool hasExpired() const {
    return deadline_ && std::chrono::steady_clock::now() >= *deadline_;
  }

  // Schedule `timer` to fire at this object's deadline.
  //   - No deadline (unbounded): no-op.
  //   - Deadline in the future: timer.scheduleTimeout(deadline - now).
  //   - Deadline already past: no-op (caller MUST hasExpired() first
  //     and fail-fast in that case).
  void schedule(folly::AsyncTimeout& timer) const {
    if (!deadline_) {
      return;
    }
    auto now = std::chrono::steady_clock::now();
    if (now >= *deadline_) {
      return;
    }
    timer.scheduleTimeout(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            *deadline_ - now));
  }

 private:
  // nullopt = unbounded; engaged value = absolute deadline.
  std::optional<std::chrono::steady_clock::time_point> deadline_;
};

} // namespace apache::thrift::fast_thrift::security
