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
#include <type_traits>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Backpressure.h>

namespace apache::thrift::fast_thrift::frame::write::handler {

/**
 * Composable backpressure policy for outbound write-path handlers.
 *
 * A handler that can absorb downstream saturation inherits its policy
 * publicly. The policy owns the two pieces of state that participation
 * requires — the pipeline's write-ready hook and the "downstream is
 * saturated" flag — so that declining to participate costs nothing rather
 * than costing an unused member plus a branch.
 *
 * Selection is compile time. `kBackpressureEnabled` gates every touchpoint in
 * the handler via `if constexpr`, and the presence of `writeReadyHook_` is
 * what the pipeline itself keys on: `makeHandlerNode` captures a hook only
 * under `if constexpr (requires(H& h) { h.writeReadyHook_; })`. With the
 * disabled policy the member does not exist, so no hook is recorded and the
 * handler can never be linked into `PipelineImpl::writeReadyList_` — the
 * pipeline cannot dispatch `onWriteReady` to it even in principle.
 *
 * Because the policy is a dependent base, handlers must reach its members
 * through `this->`.
 */
template <typename P>
concept BackpressurePolicy = requires {
  { P::kBackpressureEnabled } -> std::convertible_to<bool>;
};

/**
 * Whether T exposes the member `makeHandlerNode` keys on when deciding to
 * register a handler for write-ready notification.
 *
 * This must be a named concept rather than an inline `requires` expression:
 * the member access is only a substitution failure when its type is a template
 * parameter. Naming a concrete type inline makes it a hard error instead.
 */
template <typename T>
concept HasWriteReadyHook = requires(T& t) { t.writeReadyHook_; };

/**
 * Participates in write backpressure: buffers while downstream is saturated,
 * registers for `onWriteReady`, and drains on resume. The default.
 */
struct BackpressureEnabled {
  static constexpr bool kBackpressureEnabled = true;

  // Detected by makeHandlerNode; the pipeline drives onWriteReady through it.
  channel_pipeline::WriteReadyHook writeReadyHook_;

  // Set when a downstream write reported saturation, cleared on resume.
  bool backpressured_{false};
};

/**
 * Declines to participate. Empty, so an inheriting handler pays zero bytes
 * for it, and hook-less, so the pipeline never links or notifies the handler.
 * The handler keeps doing its primary work — a batcher still batches, a
 * fragmenter still fragments — it simply never reports or absorbs saturation.
 */
struct BackpressureDisabled {
  static constexpr bool kBackpressureEnabled = false;
};

static_assert(
    BackpressurePolicy<BackpressureEnabled>,
    "BackpressureEnabled must satisfy BackpressurePolicy concept");
static_assert(
    BackpressurePolicy<BackpressureDisabled>,
    "BackpressureDisabled must satisfy BackpressurePolicy concept");

// The zero-cost claim, enforced at compile time: the disabled policy must stay
// empty so empty-base optimization removes it entirely, and must not expose a
// writeReadyHook_ or makeHandlerNode would register the handler anyway.
static_assert(
    std::is_empty_v<BackpressureDisabled>,
    "BackpressureDisabled must be empty so EBO removes it from handlers");
static_assert(
    HasWriteReadyHook<BackpressureEnabled>,
    "BackpressureEnabled must expose writeReadyHook_ so makeHandlerNode "
    "registers the handler for write-ready notification");
static_assert(
    !HasWriteReadyHook<BackpressureDisabled>,
    "BackpressureDisabled must not expose writeReadyHook_; makeHandlerNode "
    "keys on that member to decide whether to register the handler");

} // namespace apache::thrift::fast_thrift::frame::write::handler
