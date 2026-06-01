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
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>

namespace apache::thrift::fast_thrift::channel_pipeline {

/**
 * Pipeline concept - defines the external API for channel pipelines.
 *
 * The Pipeline is the container that owns all handlers and contexts.
 * It provides:
 * - Fire to specific handler: send_read/send_write/send_exception by handler ID
 * - Fire from head/tail: fire_read/fire_write to start the chain
 * - Context lookup: context(id) to get a handler's context
 * - Lifecycle: close() to shutdown
 *
 * Requires folly::DelayedDestructionBase for safe lifecycle management.
 * Objects survive until the current call stack unwinds.
 *
 * Pipelines are static after construction (no dynamic add/remove).
 * To change the pipeline, rebuild it entirely.
 */
template <typename P, typename Ctx>
concept Pipeline = std::derived_from<P, folly::DelayedDestructionBase> &&
    requires(P p,
             HandlerId handlerId,
             TypeErasedBox&& t,
             folly::exception_wrapper&& e) {
      // Fire to specific handler (by ID lookup)
      {
        p.send_write(handlerId, std::move(t))
      } noexcept -> std::same_as<Result>;
      { p.send_read(handlerId, std::move(t)) } noexcept -> std::same_as<Result>;
      {
        p.send_exception(handlerId, std::move(e))
      } noexcept -> std::same_as<Result>;

      // Get context by handler ID
      // Returns nullptr if handler not found
      { p.context(handlerId) } noexcept -> std::same_as<Ctx*>;

      // Fire from head/tail of pipeline
      { p.fire_read(std::move(t)) } noexcept -> std::same_as<Result>;
      { p.fire_write(std::move(t)) } noexcept -> std::same_as<Result>;

      // Lifecycle
      { p.close() } noexcept -> std::same_as<void>;
    };

} // namespace apache::thrift::fast_thrift::channel_pipeline
