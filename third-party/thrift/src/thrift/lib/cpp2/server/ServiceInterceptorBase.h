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

#include <folly/ExceptionWrapper.h>
#include <folly/coro/Task.h>
#include <folly/io/IOBuf.h>
#include <folly/memory/not_null.h>

#include <thrift/lib/cpp/StreamEventHandler.h>
#include <thrift/lib/cpp2/schema/SyntaxGraph.h>
#include <thrift/lib/cpp2/server/DecoratorData.h>
#include <thrift/lib/cpp2/server/DecoratorDataRuntime.h>
#include <thrift/lib/cpp2/server/LazyDynamicArguments.h>
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
    using ServiceSchema =
        std::vector<folly::not_null<const syntax_graph::ServiceNode*>>;
    ServiceSchema serviceSchema;
    server::DecoratorDataHandleFactory* decoratorDataHandleFactory = nullptr;
  };

  virtual folly::coro::Task<void> co_onStartServing(InitParams);

  struct ConnectionInfo {
    Cpp2ConnContext* context = nullptr;
    detail::ServiceInterceptorOnConnectionStorage* storage = nullptr;
  };
  virtual void internal_onConnectionAttempted(
      ConnectionInfo, InterceptorMetricCallback&) = 0;
  virtual void internal_onConnectionEstablished(
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

    /**
     * This is the raw bytes buffer of the serialized request. Deserialized
     * arguments are already available via `RequestInfo.arguments`. However,
     * this is made available for logging purposes.
     */
    const folly::IOBuf* serializedRequestBuffer = nullptr;

    /**
     * Provides lazy access to method arguments as DynamicValue objects.
     * Arguments are deserialized on-demand from the serialized request buffer
     * using schema information. This allows interceptors to inspect arguments
     * dynamically without knowing compile-time types.
     *
     * Returns nullptr if schema information is not available for this method.
     */
    const LazyDynamicArguments* dynamicArguments = nullptr;
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

  // ============ Streaming Interceptor Types ============

  enum class StreamDirection {
    ServerStream,
    ClientSink,
    BiDirectional,
  };

  /**
   * Information provided when a stream begins.
   * Interceptors can use this to initialize per-stream state.
   */
  struct StreamInfo {
    detail::StreamId streamId = 0;
    detail::ServiceInterceptorOnRequestStorage* requestStorage = nullptr;
    StreamDirection direction = StreamDirection::ServerStream;
    std::string_view serviceName = "";
    std::string_view methodName = "";
  };

  /**
   * Information provided for each streaming payload.
   * Called BEFORE serialization with access to the typed payload.
   */
  struct StreamPayloadInfo {
    detail::StreamId streamId = 0;
    detail::ServiceInterceptorOnRequestStorage* requestStorage = nullptr;
    /**
     * The typed payload before serialization.
     * Use TypeErasedRef::value<T>() to access the concrete type.
     */
    util::TypeErasedRef payload;
    uint64_t sequenceNumber = 0;
  };

  /**
   * Information provided when a stream ends.
   * Interceptors should clean up per-stream state here.
   */
  struct StreamEndInfo {
    detail::StreamId streamId = 0;
    detail::ServiceInterceptorOnRequestStorage* requestStorage = nullptr;
    details::STREAM_ENDING_TYPES reason =
        details::STREAM_ENDING_TYPES::COMPLETE;
    folly::exception_wrapper error;
    uint64_t totalPayloads = 0;
  };

  // ============ Streaming Interceptor Methods ============
  // These methods have default no-op implementations for backward
  // compatibility with existing interceptors that don't need streaming.

  /**
   * Called when a stream is established (after first response sent).
   * Interceptors can initialize per-stream state here.
   */
  virtual folly::coro::Task<void> internal_onStreamBegin(
      ConnectionInfo, StreamInfo, InterceptorMetricCallback&) {
    co_return;
  }

  /**
   * Called for each typed payload BEFORE serialization.
   * This is where interceptors can inspect/process streaming data.
   *
   * PERFORMANCE NOTE: This is on the hot path. Implementations should
   * be as lightweight as possible.
   */
  virtual folly::coro::Task<void> internal_onStreamPayload(
      ConnectionInfo, StreamPayloadInfo, InterceptorMetricCallback&) {
    co_return;
  }

  /**
   * Called when stream ends (complete, error, or cancelled).
   * Interceptors should clean up per-stream state here.
   */
  virtual folly::coro::Task<void> internal_onStreamEnd(
      ConnectionInfo, StreamEndInfo, InterceptorMetricCallback&) {
    co_return;
  }

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
