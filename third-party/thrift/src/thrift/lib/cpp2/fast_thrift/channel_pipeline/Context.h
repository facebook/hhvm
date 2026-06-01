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

#include <folly/io/async/DelayedDestruction.h>
#include <folly/io/async/EventBase.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>

namespace apache::thrift::fast_thrift::channel_pipeline {

/**
 * ContextApi concept - defines the external API for handler contexts.
 *
 * The Context is a handler's view into the pipeline. Each handler has its own
 * context which provides:
 * - Identity: handlerId() for this context's handler ID
 * - Fire down the line: fireRead/fireWrite/fireException to next handler
 * - Pipeline access: pipeline() to look up other handlers
 * - Buffer allocation: allocate() to get buffers from pipeline's allocator
 * - Event loop: eventBase() to access the EventBase this pipeline runs on
 * - Lifecycle: close() to shutdown
 *
 * Requires folly::DelayedDestructionBase for safe lifecycle management.
 * Objects survive until the current call stack unwinds.
 */
template <typename Ctx, typename P>
concept ContextApi = std::derived_from<Ctx, folly::DelayedDestructionBase> &&
    requires(Ctx ctx,
             TypeErasedBox&& message,
             folly::exception_wrapper&& e,
             size_t size) {
      // Identity
      { ctx.handlerId() } noexcept -> std::same_as<HandlerId>;

      // Fire down the line (to next handler)
      { ctx.fireRead(std::move(message)) } noexcept -> std::same_as<Result>;
      { ctx.fireWrite(std::move(message)) } noexcept -> std::same_as<Result>;
      { ctx.fireException(std::move(e)) } noexcept -> std::same_as<void>;

      // Access to pipeline
      // Guaranteed non-null between handlerAdded() and handlerRemoved()
      { ctx.pipeline() } noexcept -> std::convertible_to<P*>;

      // Buffer allocation using pipeline's allocator
      { ctx.allocate(size) } noexcept -> std::same_as<BytesPtr>;

      // Event loop this pipeline runs on
      { ctx.eventBase() } noexcept -> std::same_as<folly::EventBase*>;

      // Lifecycle
      { ctx.close() } noexcept -> std::same_as<void>;
    };

} // namespace apache::thrift::fast_thrift::channel_pipeline
