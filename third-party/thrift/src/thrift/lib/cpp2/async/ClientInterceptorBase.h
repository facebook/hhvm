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

#include <stdexcept>
#include <vector>

#include <folly/ExceptionWrapper.h>

#include <thrift/lib/cpp2/async/ClientInterceptorStorage.h>
#include <thrift/lib/cpp2/async/InterceptorFrameworkMetadata.h>

namespace apache::thrift {

namespace transport {
class THeader;
}

class ClientInterceptorBase {
 public:
  virtual ~ClientInterceptorBase() = default;

  virtual std::string getName() const = 0;

  struct RequestInfo {
    detail::ClientInterceptorOnRequestStorage* storage = nullptr;
    ClientInterceptorOnRequestArguments arguments;
    apache::thrift::transport::THeader* headers = nullptr;
    /**
     * The name of the service definition as specified in Thrift IDL.
     */
    std::string_view serviceName = "";
    /**
     * The name of the method as specified in Thrift IDL. This does NOT include
     * the service name. If the method is an interaction method, then it will be
     * in the format `{interaction_name}.{method_name}`.
     */
    std::string_view methodName = "";
    /**
     * Mutable access to interceptor framework metadata storage - this is
     * typically initialized by the framework and will be passed along the
     * the channel as part of RpcMetadata.
     */
    InterceptorFrameworkMetadataStorage* frameworkMetadata = nullptr;
    /**
     * RpcOptions that were applied to this request
     */
    RpcOptions* rpcOptions = nullptr;
  };
  virtual void internal_onRequest(RequestInfo) = 0;

  struct ResponseInfo {
    detail::ClientInterceptorOnRequestStorage* storage = nullptr;
    const apache::thrift::transport::THeader* headers = nullptr;
    /**
     * The name of the service definition as specified in Thrift IDL.
     */
    std::string_view serviceName = "";
    /**
     * The name of the method as specified in Thrift IDL. This does NOT include
     * the service name. If the method is an interaction method, then it will be
     * in the format `{interaction_name}.{method_name}`.
     */
    std::string_view methodName = "";
  };
  virtual void internal_onResponse(ResponseInfo) = 0;

  static constexpr std::size_t kMaxRequestStateSize =
      detail::ClientInterceptorOnRequestStorage::max_size();
};

class ClientInterceptorException : public std::runtime_error {
 public:
  enum class CallbackKind { ON_REQUEST, ON_RESPONSE };

  struct SingleExceptionInfo {
    std::string sourceInterceptorName;
    folly::exception_wrapper cause;
  };

  ClientInterceptorException(CallbackKind, std::vector<SingleExceptionInfo>);
  ClientInterceptorException(const ClientInterceptorException&) = default;
  ClientInterceptorException& operator=(const ClientInterceptorException&) =
      default;

  const std::vector<SingleExceptionInfo>& causes() const noexcept {
    return causes_;
  }

 private:
  std::vector<SingleExceptionInfo> causes_;
};

} // namespace apache::thrift
