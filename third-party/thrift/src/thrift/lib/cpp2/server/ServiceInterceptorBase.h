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
#include <folly/memory/not_null.h>

#include <thrift/lib/cpp2/schema/SyntaxGraph.h>
#include <thrift/lib/cpp2/server/DecoratorData.h>
#include <thrift/lib/cpp2/server/DecoratorDataRuntime.h>
#include <thrift/lib/cpp2/server/ServiceInterceptorStorage.h>
#include <thrift/lib/cpp2/server/metrics/InterceptorMetricCallback.h>

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

  /**
   * This method returns the name of the interceptor itself. This is use as
   * part of the interceptor's qualified name.
   */
  virtual std::string getName() const = 0;

  /**
   * At runtime, ServiceInterceptors are uniquely identified by
   * {module name}.{interceptor name}. This methods returns
   * ServiceInterceptorQualifiedName which encapsulates this identifier. If
   * this is called prior to the module being set, this will fatal
   */
  virtual const ServiceInterceptorQualifiedName& getQualifiedName() const = 0;

  struct InitParams {
#ifdef THRIFT_SCHEMA_AVAILABLE
    using ServiceSchema =
        std::vector<folly::not_null<const syntax_graph::ServiceNode*>>;
    ServiceSchema serviceSchema;
#endif
    server::DecoratorDataHandleFactory* decoratorDataHandleFactory = nullptr;
  };

  virtual folly::coro::Task<void> co_onStartServing(InitParams);

  struct ConnectionInfo {
    Cpp2ConnContext* context = nullptr;
    detail::ServiceInterceptorOnConnectionStorage* storage = nullptr;
  };
  virtual void internal_onConnection(
      ConnectionInfo, InterceptorMetricCallback&) = 0;
  virtual void internal_onConnectionClosed(
      ConnectionInfo, InterceptorMetricCallback&) noexcept = 0;

  struct RequestInfo {
    Cpp2RequestContext* context = nullptr;
    detail::ServiceInterceptorOnRequestStorage* storage = nullptr;
    detail::ServiceInterceptorOnRequestArguments arguments;
    /**
     * The name of the service definition as specified in Thrift IDL.
     */
    std::string_view serviceName = "";
    /**
     * The name of the service definition (where the corresponding method name)
     * as specified in Thrift IDL. In most cases, this will be the same as the
     * service name. However, if method name is pulled in via service
     * inheritance, then this name will match the base class that defines the
     * method.
     */
    std::string_view definingServiceName = "";
    /**
     * The name of the method as specified in Thrift IDL. This does NOT include
     * the service name. If the method is an interaction method, then it will be
     * in the format `{interaction_name}.{method_name}`.
     */
    std::string_view methodName = "";
    /**
     * The name of the method as specified in Thrift IDL. This includes the
     * service name, i.e it is in the format `{service_name}.{method_name}`. If
     * the method is an interaction method, then it will be in the format
     * `{service_name}.{interaction_name}.{method_name}`.
     */
    std::string_view qualifiedMethodName = "";
    /**
     * This is a pointer to the deserialized frameworkMetadata buffer sent as
     * part of the request. InterceptorFrameworkMetadataStorage may be empty
     * even if the pointer here is not null - it should always be checked with
     * has_value()
     */
    const InterceptorFrameworkMetadataStorage* frameworkMetadata = nullptr;
    /**
     * Interceptors have access to decorator data on request through this
     * pointer. In order to read DecoratorData, DecoratorDataHandles need to be
     * initialized from the userDecoratorData() function.
     */
    const server::DecoratorData* decoratorData = nullptr;
  };
  virtual folly::coro::Task<void> internal_onRequest(
      ConnectionInfo, RequestInfo, InterceptorMetricCallback&) = 0;

  struct ResponseInfo {
    const Cpp2RequestContext* context = nullptr;
    detail::ServiceInterceptorOnRequestStorage* storage = nullptr;
    detail::ServiceInterceptorOnResponseResult resultOrActiveException;
    /**
     * The name of the service definition as specified in Thrift IDL.
     */
    std::string_view serviceName = "";
    /**
     * The name of the service definition (where the corresponding method name)
     * as specified in Thrift IDL. In most cases, this will be the same as the
     * service name. However, if method name is pulled in via service
     * inheritance, then this name will match the base class that defines the
     * method.
     */
    std::string_view definingServiceName = "";
    /**
     * The name of the method as specified in Thrift IDL. This does NOT include
     * the service name. If the method is an interaction method, then it will be
     * in the format `{interaction_name}.{method_name}`.
     */
    std::string_view methodName = "";
    /**
     * The name of the method as specified in Thrift IDL. This includes the
     * service name, i.e it is in the format `{service_name}.{method_name}`. If
     * the method is an interaction method, then it will be in the format
     * `{service_name}.{interaction_name}.{method_name}`.
     */
    std::string_view qualifiedMethodName = "";
    /**
     * Interceptors have access to decorator data on response through this
     * pointer. In order to read DecoratorData, DecoratorDataHandles need to be
     * initialized from the userDecoratorData() function.
     */
    const server::DecoratorData* decoratorData = nullptr;
  };
  virtual folly::coro::Task<void> internal_onResponse(
      ConnectionInfo, ResponseInfo, InterceptorMetricCallback&) = 0;

  /**
   * This methods is called by ThriftServer to set the module name for the
   * interceptor. This method is expected to be called prior to to the
   * the interceptor methods being executed on any request, as it performs
   * various secondary intitialization requiring module name to be known.
   */
  virtual void setModuleName(const std::string& moduleName) = 0;

  static constexpr std::size_t kMaxRequestStateSize =
      detail::ServiceInterceptorOnRequestStorage::max_size();
  static constexpr std::size_t kMaxConnectionStateSize =
      detail::ServiceInterceptorOnConnectionStorage::max_size();
};

#endif // FOLLY_HAS_COROUTINES

} // namespace apache::thrift
