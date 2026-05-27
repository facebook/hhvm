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

#include <folly/IntrusiveList.h>

namespace apache::thrift::fast_thrift::channel_pipeline {

/**
 * EventHook provides intrusive list membership for user-event broadcast.
 *
 * Unlike Write/ReadReadyHook, this registration is **static** — handlers
 * are linked once at pipeline construction (based on whether they
 * implement `onEvent`) and stay linked for the pipeline's lifetime. No
 * `await/cancel` API: the link reflects "implements onEvent", not a
 * transient backpressure state.
 *
 * Lives in ContextImpl (stable addresses), not in the handler itself —
 * users implement `onEvent(ctx, const TypeErasedBox&)` and the
 * framework wires the hook up automatically.
 */
struct EventHook {
  folly::IntrusiveListHook hook;
  size_t handlerIndex{0}; // Set by PipelineImpl during initialization
};

/**
 * List of handlers that implement `onEvent`. Maintained by PipelineImpl,
 * walked when `fireEvent` is called. Sparse — only the handlers that
 * opted in are iterated.
 */
using EventList = folly::IntrusiveList<EventHook, &EventHook::hook>;

} // namespace apache::thrift::fast_thrift::channel_pipeline
