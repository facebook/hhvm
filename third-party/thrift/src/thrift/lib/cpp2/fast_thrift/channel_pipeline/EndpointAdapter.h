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

#include <folly/ExceptionWrapper.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>

namespace apache::thrift::fast_thrift::channel_pipeline {

/**
 * EndpointHandler concept — terminal of a pipeline (head or tail).
 *
 * Both endpoints receive messages via onMessage() and exceptions via
 * onException().  The pipeline routes reads and writes to whichever
 * endpoint sits at the exit of each direction, controlled by
 * HeadToTailOp.
 */
template <typename E>
concept EndpointHandler =
    requires(E e, TypeErasedBox&& msg, folly::exception_wrapper&& ex) {
      { e.onMessage(std::move(msg)) } noexcept -> std::same_as<Result>;
      { e.onException(std::move(ex)) } noexcept -> std::same_as<void>;
    };

/**
 * EndpointLifecycleHook provides optional lifecycle notifications for
 * endpoint adapters.
 *
 * Adapters that want to receive activate/deactivate notifications embed
 * this hook as a public member named `lifecycleHook_`.  The hook is
 * automatically detected at compile time when the endpoint adapter is set
 * in PipelineBuilder::build() via `if constexpr`.
 *
 * Usage:
 *   class MyAppHandler {
 *    public:
 *     EndpointLifecycleHook lifecycleHook_{
 *         .onActivated = [](void* s) noexcept {
 *             static_cast<MyAppHandler*>(s)->onActivated();
 *         },
 *         .onDeactivated = [](void* s) noexcept {
 *             static_cast<MyAppHandler*>(s)->onDeactivated();
 *         },
 *         .self = this,
 *     };
 *
 *     Result onMessage(TypeErasedBox&& msg) noexcept { ... }
 *     void onException(folly::exception_wrapper&& e) noexcept { ... }
 *   };
 */
struct EndpointLifecycleHook {
  void (*onActivated)(void* self) noexcept {nullptr};
  void (*onDeactivated)(void* self) noexcept {nullptr};
  void* self{nullptr};
};

} // namespace apache::thrift::fast_thrift::channel_pipeline
