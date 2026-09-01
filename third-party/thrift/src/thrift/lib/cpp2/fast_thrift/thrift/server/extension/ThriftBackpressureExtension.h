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

#include <concepts>
#include <memory>
#include <utility>

#include <folly/io/async/EventBase.h>

#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/context/ThriftConnContext.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/extension/ThriftConnectionExtension.h>

namespace apache::thrift::fast_thrift::thrift {

/**
 * Per-connection admission control for FastThriftServer extensions.
 *
 * A service whose cost per request is not bounded by the wire — work queued
 * against a database, say — cannot rely on transport backpressure to protect
 * it: the socket can be completely idle while the service is far behind. This
 * family lets such a service stop admitting requests on a connection based on
 * its own state, and resume when it has caught up.
 *
 * The signal it hangs off is egress going idle: the point at which the server
 * has finished writing everything it owed this client. That is a natural place
 * to reassess a connection, and it costs nothing to observe because the
 * write-completion event is already travelling up the pipeline.
 *
 * Pausing itself is expressed on the request path, via
 * RequestVerdict::backpressure(): that is the one point where the decision can
 * still reach the transport and stop the socket before the next request
 * arrives. This family supplies the other half — the resumer that lifts it, and
 * a notification each time the server catches up, which is when a consumer
 * would reassess and resume.
 *
 * Ordering, per connection:
 *   onBackpressureAttached — once, before any request. Hands over the resumer.
 *   onEgressDrained        — each time the connection's egress goes idle.
 *
 * Declaring onEgressDrained is how an extension opts in; it composes freely
 * with the request/response and connection-lifecycle callbacks on the same
 * extension, and one instance per connection sees all of them.
 */

namespace backpressure_detail {

/**
 * Shared between the adapter and every resumer it handed out. The adapter
 * clears `owner` when its pipeline goes inactive, which is what makes a resume
 * arriving after teardown a no-op rather than a use-after-free. The indirection
 * through a function pointer keeps ReadResumer a concrete type an extension can
 * store as a plain member.
 *
 * Unsynchronized by design: both sides run on the connection's EventBase, so
 * the clear and the read of `owner` are ordered by that thread alone.
 */
struct ResumeControl {
  void (*resumeFn)(void* owner) noexcept {nullptr};
  void* owner{nullptr};
};

} // namespace backpressure_detail

/**
 * Resumes a connection paused by onEgressDrained.
 *
 * Must be called on the connection's EventBase — the thread every extension
 * callback already runs on. Neither this resumer nor the connection state it
 * reaches is synchronized, so an extension that decides to resume from
 * elsewhere has to hop back onto that EventBase to do it.
 *
 * Safe to hold for the life of the extension, and safe to call once the
 * connection has closed — a resume with nothing left to resume does nothing.
 * That matters because the natural place to resume is a later turn of the
 * event loop (see below), which can land after teardown.
 *
 * Resuming from inside onEgressDrained is legal but rarely what you want: that
 * callback runs synchronously from the socket's write-completion path, so
 * resuming there restarts reads underneath the write that triggered it and can
 * begin the next request inside the tail of the previous one's reply. Defer to
 * the next loop iteration — eventBase()->runInLoop(...) — unless the handler is
 * known to tolerate it.
 */
class ReadResumer {
 public:
  ReadResumer() = default;

  explicit ReadResumer(
      std::shared_ptr<backpressure_detail::ResumeControl> control,
      folly::EventBase* eventBase) noexcept
      : control_(std::move(control)), eventBase_(eventBase) {}

  void resume() noexcept {
    if (control_ == nullptr) {
      return;
    }
    if (void* owner = control_->owner; owner != nullptr) {
      control_->resumeFn(owner);
    }
  }

  // False once the connection this resumer belongs to has gone away.
  explicit operator bool() const noexcept {
    return control_ != nullptr && control_->owner != nullptr;
  }

  /**
   * The connection's EventBase — the one resume() may be called on, and the
   * one a deferred resume must be scheduled against. Null only on a
   * default-constructed resumer.
   *
   * Outlives the connection, so it stays usable for as long as the resumer
   * itself does. The thread-local EventBase an extension could reach for
   * instead is not necessarily this one: a server driven by an
   * IOThreadPoolExecutor built over a private EventBaseManager registers its
   * loops nowhere the global manager can see them, and asking that manager on
   * such a thread hands back a fresh EventBase that nothing ever loops.
   */
  folly::EventBase* eventBase() const noexcept { return eventBase_; }

 private:
  std::shared_ptr<backpressure_detail::ResumeControl> control_;
  folly::EventBase* eventBase_{nullptr};
};

// === Concepts ===

template <typename H>
concept HasEgressDrainedCallback =
    requires(H& h, const ThriftConnectionView& v) {
      { h.onEgressDrained(v) } noexcept -> std::same_as<void>;
    };

template <typename H>
concept HasBackpressureAttachedCallback = requires(H& h, ReadResumer resumer) {
  {
    h.onBackpressureAttached(std::move(resumer))
  } noexcept -> std::same_as<void>;
};

/**
 * True iff H does per-connection admission control. Such an extension observes
 * per-connection state, so the server requires enableRequestContext when one is
 * registered.
 */
template <typename H>
concept ThriftBackpressureExtensionHandler = HasEgressDrainedCallback<H>;

} // namespace apache::thrift::fast_thrift::thrift
