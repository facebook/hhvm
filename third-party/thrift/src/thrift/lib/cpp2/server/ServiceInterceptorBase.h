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

#include <variant>

#include <folly/coro/Task.h>

#include <thrift/lib/cpp2/server/ServiceInterceptorStorage.h>

namespace apache::thrift {

class ServiceInterceptorBase;

#if FOLLY_HAS_COROUTINES

class Cpp2ConnContext;
class Cpp2RequestContext;
class ContextStack;

namespace detail {
using ServiceInterceptorOnResponseResult =
    std::variant<folly::exception_wrapper, apache::thrift::util::TypeErasedRef>;
}

class ServiceInterceptorBase {
 public:
  virtual ~ServiceInterceptorBase() = default;

  virtual std::string getName() const = 0;

  struct InitParams {};
  virtual folly::coro::Task<void> co_onStartServing(InitParams);

  struct ConnectionInfo {
    Cpp2ConnContext* context = nullptr;
    detail::ServiceInterceptorOnConnectionStorage* storage = nullptr;
  };
  virtual void internal_onConnection(ConnectionInfo) = 0;
  virtual void internal_onConnectionClosed(ConnectionInfo) noexcept = 0;

  struct RequestInfo {
    Cpp2RequestContext* context = nullptr;
    detail::ServiceInterceptorOnRequestStorage* storage = nullptr;
    detail::ServiceInterceptorOnRequestArguments arguments;
    /**
     * The name of the service definition as specified in Thrift IDL.
     */
    const char* serviceName = nullptr;
    /**
     * The name of the service definition (where the corresponding method name)
     * as specified in Thrift IDL. In most cases, this will be the same as the
     * service name. However, if method name is pulled in via service
     * inheritance, then this name will match the base class that defines the
     * method.
     */
    const char* definingServiceName = nullptr;
    /**
     * The name of the method as specified in Thrift IDL. This does NOT include
     * the service name. If the method is an interaction method, then it will be
     * in the format `{interaction_name}.{method_name}`.
     */
    const char* methodName = nullptr;
  };
  virtual folly::coro::Task<void> internal_onRequest(
      ConnectionInfo, RequestInfo) = 0;

  struct ResponseInfo {
    const Cpp2RequestContext* context = nullptr;
    detail::ServiceInterceptorOnRequestStorage* storage = nullptr;
    detail::ServiceInterceptorOnResponseResult resultOrActiveException;
    /**
     * The name of the service definition as specified in Thrift IDL.
     */
    const char* serviceName = nullptr;
    /**
     * The name of the service definition (where the corresponding method name)
     * as specified in Thrift IDL. In most cases, this will be the same as the
     * service name. However, if method name is pulled in via service
     * inheritance, then this name will match the base class that defines the
     * method.
     */
    const char* definingServiceName = nullptr;
    /**
     * The name of the method as specified in Thrift IDL. This does NOT include
     * the service name. If the method is an interaction method, then it will be
     * in the format `{interaction_name}.{method_name}`.
     */
    const char* methodName = nullptr;
  };
  virtual folly::coro::Task<void> internal_onResponse(
      ConnectionInfo, ResponseInfo) = 0;

  static constexpr std::size_t kMaxRequestStateSize =
      detail::ServiceInterceptorOnRequestStorage::max_size();
  static constexpr std::size_t kMaxConnectionStateSize =
      detail::ServiceInterceptorOnConnectionStorage::max_size();
};

#endif // FOLLY_HAS_COROUTINES

} // namespace apache::thrift
