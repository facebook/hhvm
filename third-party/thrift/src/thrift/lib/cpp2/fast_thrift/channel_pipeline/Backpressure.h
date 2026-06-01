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
 * WriteReadyHook provides intrusive list membership for write backpressure.
 *
 * Handlers that want to receive onWriteReady() notifications embed this hook
 * as a public member named `writeReadyHook_`. The hook is automatically
 * detected at compile time when the handler is added to the pipeline via
 * makeHandlerNode.
 *
 * Usage:
 *   class MyHandler {
 *    public:
 *     WriteReadyHook writeReadyHook_;  // Detected by makeHandlerNode
 *
 *     Result onWrite(ContextImpl& ctx, TypeErasedBox&& msg) noexcept {
 *       auto result = ctx.fireWrite(std::move(msg));
 *       if (result == Result::Backpressure) {
 *         pending_.push_back(std::move(msg));
 *         ctx.awaitWriteReady();  // Register for callback
 *         return Result::Backpressure;
 *       }
 *       return result;
 *     }
 *
 *     void onWriteReady(ContextImpl& ctx) noexcept {
 *       // Retry pending messages...
 *       ctx.cancelAwaitWriteReady();  // Unregister when done
 *     }
 *   };
 *
 * Note: Read backpressure is handled at the transport level via TCP flow
 * control. When the read path returns Result::Backpressure, the transport
 * adapter should call pauseRead() on the socket. There is no handler-to-handler
 * read backpressure signaling - this follows the Netty model.
 */
struct WriteReadyHook {
  folly::IntrusiveListHook hook;
  size_t handlerIndex{0}; // Set by PipelineImpl during initialization
};

/**
 * List of handlers awaiting write ready notifications.
 * Maintained by PipelineImpl, walked when onWriteReady() is called.
 */
using WriteReadyList =
    folly::IntrusiveList<WriteReadyHook, &WriteReadyHook::hook>;

} // namespace apache::thrift::fast_thrift::channel_pipeline
