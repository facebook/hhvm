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
 * EndpointHandlerLifecycle concept — base lifecycle methods for all endpoint
 * handlers.
 *
 * All endpoint handlers must implement these lifecycle methods:
 * - handlerAdded(): Called when handler is added to the pipeline (build())
 * - handlerRemoved(): Called when handler is removed (pipeline destroyed)
 * - onPipelineActive(): Called when pipeline becomes active (connection ready)
 * - onPipelineInactive(): Called when pipeline becomes inactive (closing)
 */
template <typename H>
concept EndpointHandlerLifecycle = requires(H h) {
  { h.handlerAdded() } noexcept -> std::same_as<void>;
  { h.handlerRemoved() } noexcept -> std::same_as<void>;
  { h.onPipelineActive() } noexcept -> std::same_as<void>;
  { h.onPipelineInactive() } noexcept -> std::same_as<void>;
};

/**
 * HeadEndpointHandler concept — endpoint at the head of the pipeline.
 *
 * The head receives data via onWrite().
 *
 * Flow: fireWrite() propagates through handlers and arrives at head's
 * onWrite().
 *
 * Required methods:
 * - onWrite(TypeErasedBox&&): Handle data exiting the pipeline
 * - Plus all EndpointHandlerLifecycle methods
 */
template <typename H>
concept HeadEndpointHandler =
    EndpointHandlerLifecycle<H> && requires(H h, TypeErasedBox&& msg) {
      { h.onWrite(std::move(msg)) } noexcept -> std::same_as<Result>;
    };

/**
 * TailEndpointHandler concept — endpoint at the tail of the pipeline.
 *
 * The tail receives data via onRead() and exceptions via onException().
 *
 * Flow: fireRead() propagates through handlers and arrives at tail's onRead().
 *
 * Required methods:
 * - onRead(TypeErasedBox&&): Handle data entering the pipeline
 * - onException(exception_wrapper&&): Handle pipeline exceptions
 * - Plus all EndpointHandlerLifecycle methods
 */
template <typename T>
concept TailEndpointHandler = EndpointHandlerLifecycle<T> &&
    requires(T t, TypeErasedBox&& msg, folly::exception_wrapper&& ex) {
      { t.onRead(std::move(msg)) } noexcept -> std::same_as<Result>;
      { t.onException(std::move(ex)) } noexcept -> std::same_as<void>;
    };

/**
 * ValidEndpointPair concept — validates head/tail endpoint combination.
 *
 * A valid pipeline requires a HeadEndpointHandler at the head and
 * a TailEndpointHandler at the tail.
 */
template <typename Head, typename Tail>
concept ValidEndpointPair =
    HeadEndpointHandler<Head> && TailEndpointHandler<Tail>;

} // namespace apache::thrift::fast_thrift::channel_pipeline
