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

#include <folly/experimental/coro/Task.h>

namespace apache::thrift {

class ServiceInterceptorBase;

#if FOLLY_HAS_COROUTINES

class Cpp2ConnContext;
class Cpp2RequestContext;
class ContextStack;

class ServiceInterceptorBase {
 public:
  virtual ~ServiceInterceptorBase() = default;

  struct InitParams {};
  virtual folly::coro::Task<void> co_onStartServing(InitParams);

  struct ConnectionInfo {
    const Cpp2ConnContext* context = nullptr;
  };
  virtual void internal_onConnection(ConnectionInfo) = 0;

  struct RequestInfo {
    const Cpp2RequestContext* context = nullptr;
  };
  virtual folly::coro::Task<void> internal_onRequest(
      ConnectionInfo, RequestInfo) = 0;

  struct ResponseInfo {
    const Cpp2RequestContext* context = nullptr;
  };
  virtual folly::coro::Task<void> internal_onResponse(
      ConnectionInfo, ResponseInfo) = 0;
};

#endif // FOLLY_HAS_COROUTINES

} // namespace apache::thrift
