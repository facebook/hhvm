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

#include <folly/Executor.h>
#include <folly/io/async/EventBase.h>
#include <thrift/lib/cpp/concurrency/ThreadManager.h>
#include <thrift/lib/cpp2/server/Cpp2ConnContext.h>

namespace apache::thrift {

/**
 * This struct encapsulates the various thrift control information of interest
 * to request handlers; the executor on which we expect them to execute, the
 * Cpp2RequestContext of the incoming request struct, etc.
 */
class RequestParams {
 public:
  RequestParams(
      Cpp2RequestContext* requestContext,
      concurrency::ThreadManager* threadManager,
      folly::EventBase* eventBase,
      folly::Executor* handlerExecutor = nullptr)
      : requestContext_(requestContext),
        threadManager_(threadManager),
        handlerExecutor_(handlerExecutor),
        eventBase_(eventBase) {}
  RequestParams() = default;

  Cpp2RequestContext* getRequestContext() const { return requestContext_; }

  // For cases where caller only needs the folly::Executor* interface.
  // These calls can be replaced with getHandlerExecutor.
  [[deprecated("Use getHandlerExecutor()")]] folly::Executor* getThreadManager()
      const {
    return getHandlerExecutor();
  }

  // For cases where the caller needs the ThreadManager interface. Caller
  // needs to be refactored to replace these calls with getHandlerExecutor.
  [[deprecated("Use getHandlerExecutor()")]] concurrency::ThreadManager*
  getThreadManager_deprecated() const {
    return threadManager_;
  }

  folly::EventBase* getEventBase() const { return eventBase_; }
  folly::Executor* getHandlerExecutor() const {
    if (threadManager_) {
      return threadManager_;
    } else {
      return handlerExecutor_;
    }
  }

 private:
  friend class ServerInterface;

  Cpp2RequestContext* requestContext_{nullptr};
  concurrency::ThreadManager* threadManager_{nullptr};
  folly::Executor* handlerExecutor_{nullptr};
  folly::EventBase* eventBase_{nullptr};
};

} // namespace apache::thrift
