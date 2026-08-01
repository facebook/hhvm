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

#include <cstdint>

#include <thrift/lib/cpp2/fast_thrift/frame/FrameType.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/DirectStreamMap.h>

namespace apache::thrift::fast_thrift::rocket {

/**
 * RocketStreamContext - everything the rocket layer must remember about a
 * single stream between its frames.
 *
 * Held (keyed by streamId) in the pipeline-level RocketStreamContexts map and
 * shared across the rocket stream handlers on a connection, replacing the
 * private per-handler maps. The type is direction-agnostic: the same fields
 * describe a stream regardless of which endpoint owns it. Fields are the union
 * of what those handlers need:
 *
 * - streamType: the initiating request's REQUEST_* frame type, captured when
 *   the stream opens and remembered for its lifetime (continuation frames like
 *   REQUEST_N/CANCEL carry only a streamId, so it is re-stamped from here).
 * Lets rocket-layer handlers dispatch by frame type without their own
 * bookkeeping. This is the RSocket frame type, not the Thrift RpcKind (the
 * rocket layer never parses metadata); it is a 1:1 proxy for RpcKind except
 * that REQUEST_CHANNEL covers both sink and bidi, which are separated at the
 * Thrift layer after metadata deserialization, not here.
 * - credits: outstanding REQUEST_N demand for the stream. Streaming, sink, and
 *   bidi share the REQUEST_N credit model, so one field serves all three. Zero
 *   until a demand-tracking per-pattern handler populates it.
 *
 * Deliberately small; concerns that do not belong here become separate
 * pipeline-level state types via PipelineBuilder::addState<T>().
 */
struct RocketStreamContext {
  frame::FrameType streamType{frame::FrameType::RESERVED};
  uint64_t credits{0};
};

/**
 * RocketStreamContexts - the connection's table of per-stream state, one entry
 * per live streamId.
 *
 * Mental model: after the opening frame, continuation and terminal frames
 * (REQUEST_N, CANCEL, PAYLOAD, ...) carry only a streamId, not the RPC kind and
 * not any accrued demand. This table is where a handler recovers what a
 * streamId is (its streamType) and what has accumulated for it (credits).
 *
 * Registered once per connection via
 * PipelineBuilder::addState<RocketStreamContexts>() and reached by handlers
 * through ctx.state<RocketStreamContexts>(). Wraps the streamId-keyed map
 * (DirectStreamMap's default StreamIdIndex) in a struct so that additional
 * connection-scoped stream state can be added alongside it without changing the
 * registered state type.
 */
struct RocketStreamContexts {
  frame::read::DirectStreamMap<RocketStreamContext> streams;
};

} // namespace apache::thrift::fast_thrift::rocket
