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
 * Channel concept - represents a connection with an associated pipeline.
 *
 * The Channel is the top-level abstraction that ties together:
 * - A transport (socket, etc.) — hidden from handlers
 * - A pipeline for processing data
 *
 * The transport details are hidden from handlers. Handlers interact with the
 * pipeline, not the underlying transport directly.
 *
 * Requires folly::DelayedDestructionBase for safe lifecycle management.
 * Objects survive until the current call stack unwinds.
 */
template <typename C, typename P>
concept Channel =
    std::derived_from<C, folly::DelayedDestructionBase> && requires(C c) {
      // Access to the pipeline
      { c.pipeline() } noexcept -> std::same_as<P*>;

      // Connection state
      { c.is_active() } noexcept -> std::same_as<bool>;

      // Lifecycle
      { c.close() } noexcept -> std::same_as<void>;
    };

} // namespace apache::thrift::fast_thrift::channel_pipeline
