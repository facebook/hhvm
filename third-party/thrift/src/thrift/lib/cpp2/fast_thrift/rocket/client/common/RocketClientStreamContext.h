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

#include <thrift/lib/cpp2/fast_thrift/frame/FrameType.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/DirectStreamMap.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/common/TypeErasedPtr.h>

namespace apache::thrift::fast_thrift::rocket::client {

/**
 * PendingRequestStats - the request-side half of a response's transport stats.
 *
 * These cannot be read off the response the way the response-side sizes can:
 * they are only knowable by correlating the outbound write with the response
 * that answers it, so they are parked on the stream's context until the
 * terminal frame arrives.
 *
 * Written and read exclusively by RocketClientStatsHandler. A pipeline that
 * omits that handler leaves every field at its zero default and never reads a
 * clock; the fields still occupy space in the context, which is the price of
 * sharing one per-stream map instead of keeping a second one.
 */
struct PendingRequestStats {
  /// Steady, not system: these marks only ever produce durations, and a wall
  /// clock stepping backwards would turn one into a negative latency.
  using Clock = std::chrono::steady_clock;

  /// Set when the request passes through on its way out.
  Clock::time_point writeStart;
  /// Set when the transport reports the request's bytes are gone. Stays at the
  /// clock epoch on pipelines that never report write completions — that is
  /// the signal to report neither latency.
  Clock::time_point writeComplete;
  /// Set only for a response the server fragmented, from the event the
  /// defragmentation handler's tracker fires. Stays at the clock epoch for a
  /// response that arrived whole, where the terminal frame this handler sees
  /// *is* the first frame — and on a pipeline whose defragmentation handler
  /// carries no tracker, which makes a fragmented response indistinguishable
  /// from a whole one.
  Clock::time_point firstResponseFrame;
  uint32_t requestWireSizeBytes{0};
  uint32_t requestMetadataAndPayloadSizeBytes{0};
};

/**
 * RocketClientStreamContext - everything the client rocket layer must remember
 * about a single outbound stream between its frames.
 *
 * Held (keyed by streamId) in the pipeline-level RocketClientStreamContexts
 * map and shared across the client rocket handlers on a connection, so they do
 * not each keep a private map of the same streams. Fields are the union of
 * what those handlers need:
 *
 * - requestContext: a type-erased owning handle to the AppAdapter's
 *   heap-allocated per-request context. The rocket layer never dereferences
 *   the pointer; it only routes the handle and runs the deleter the AppAdapter
 *   supplied on any cleanup path (terminal frame, map clear on connection
 *   teardown, ~DirectStreamMap on pipeline destruction), so the map never
 *   holds a stale pointer.
 * - streamType: the initiating request's REQUEST_* frame type, captured when
 *   the stream opens and remembered for its lifetime, so inbound frames — which
 *   carry only a streamId — can be stamped for stateless dispatch downstream.
 * - stats: see PendingRequestStats. Untouched unless the stats handler is
 *   installed.
 *
 * The client counterpart of rocket::RocketStreamContext, kept separate because
 * the two directions need disjoint fields: only the client parks a
 * requestContext and request-side stats, only the server tracks credits.
 */
struct RocketClientStreamContext {
  TypeErasedPtr requestContext;
  frame::FrameType streamType{frame::FrameType::RESERVED};
  PendingRequestStats stats;
};

/**
 * RocketClientStreamContexts - the connection's table of per-stream state, one
 * entry per live streamId.
 *
 * Mental model: a response frame carries only a streamId, not the RPC kind, not
 * the caller's context, and not anything measured about the request that
 * provoked it. This table is where a handler recovers all of that for a
 * streamId.
 *
 * Registered once per connection via
 * PipelineBuilder::addState<RocketClientStreamContexts>() and reached by
 * handlers through ctx.state<RocketClientStreamContexts>().
 * RocketClientStreamStateHandler owns the entry lifecycle (insert on
 * stream-open, erase on terminal); every other handler only reads or updates
 * entries it finds. Wraps the streamId-keyed map (DirectStreamMap's default
 * StreamIdIndex) in a struct so that additional connection-scoped stream state
 * can be added alongside it without changing the registered state type.
 */
struct RocketClientStreamContexts {
  frame::read::DirectStreamMap<RocketClientStreamContext> streams;
};

} // namespace apache::thrift::fast_thrift::rocket::client
