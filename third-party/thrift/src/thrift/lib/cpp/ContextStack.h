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
#include <folly/Try.h>
#include <thrift/lib/cpp/SerializedMessage.h>
#include <thrift/lib/cpp/TProcessorEventHandler.h>
#include <thrift/lib/cpp/protocol/TProtocolTypes.h>
#include <thrift/lib/cpp/transport/THeader.h>
#include <thrift/lib/cpp2/async/ClientInterceptorBase.h>
#include <thrift/lib/cpp2/async/ClientInterceptorStorage.h>
#include <thrift/lib/cpp2/util/AllocationColocator.h>

#include <thrift/lib/cpp/StreamEventHandler.h>

namespace apache::thrift {

class ContextStack;

namespace detail {
/**
 * Internal API for use within thrift library - users should not use this
 * directly.
 */
class ContextStackUnsafeAPI {
 public:
  explicit ContextStackUnsafeAPI(ContextStack&);
  void*& contextAt(size_t index) const;
  std::unique_ptr<folly::IOBuf> getInterceptorFrameworkMetadata(
      const RpcOptions& rpcOptions);

 private:
  ContextStack& contextStack_;
};
} // namespace detail

class ContextStack {
  friend class EventHandlerBase;
  friend class detail::ContextStackUnsafeAPI;

 public:
  using UniquePtr =
      apache::thrift::util::AllocationColocator<ContextStack>::Ptr;

  // Note: factory functions return nullptr if handlers is nullptr or empty.
  static UniquePtr create(
      const std::shared_ptr<
          std::vector<std::shared_ptr<TProcessorEventHandler>>>& handlers,
      std::string_view serviceName,
      std::string_view method,
      TConnectionContext* connectionContext);

  static UniquePtr createWithClientContext(
      const std::shared_ptr<
          std::vector<std::shared_ptr<TProcessorEventHandler>>>& handlers,
      const std::shared_ptr<
          std::vector<std::shared_ptr<ClientInterceptorBase>>>&
          clientInterceptors,
      std::string_view serviceName,
      std::string_view method,
      transport::THeader& header);

  static ContextStack::UniquePtr createWithClientContextCopyNames(
      const std::shared_ptr<
          std::vector<std::shared_ptr<TProcessorEventHandler>>>& handlers,
      const std::shared_ptr<
          std::vector<std::shared_ptr<ClientInterceptorBase>>>&
          clientInterceptors,
      const std::string& serviceName,
      const std::string& methodName,
      transport::THeader& header);

  ContextStack(ContextStack&&) = delete;
  ContextStack& operator=(ContextStack&&) = delete;
  ContextStack(const ContextStack&) = delete;
  ContextStack& operator=(const ContextStack&) = delete;

  ~ContextStack();

  void preWrite();

  void onWriteData(const SerializedMessage& msg);

  void postWrite(uint32_t bytes);

  void preRead();

  void onReadData(const SerializedMessage& msg);

  void postRead(apache::thrift::transport::THeader* header, uint32_t bytes);

  void onInteractionTerminate(int64_t id);

  void handlerErrorWrapped(const folly::exception_wrapper& ew);
  void userExceptionWrapped(bool declared, const folly::exception_wrapper& ew);

  void onStreamSubscribe(const StreamEventHandler::StreamContext&);
  void onStreamNext();
  void onStreamNextSent();
  void onStreamCredit(uint32_t credits);
  void onStreamPause(details::STREAM_PAUSE_REASON reason);
  void onStreamResumeReceive();
  void handleStreamErrorWrapped(const folly::exception_wrapper& ew);
  void onStreamFinally(details::STREAM_ENDING_TYPES endReason);

  void onSinkSubscribe(const StreamEventHandler::StreamContext&);
  void onSinkNext();
  void onSinkConsumed();
  void onSinkCancel();
  void onSinkCredit(uint32_t credits);
  void onSinkFinally(details::SINK_ENDING_TYPES endReason);
  void handleSinkError(const folly::exception_wrapper& ew);

  void resetClientRequestContextHeader();

  const std::shared_ptr<std::vector<std::shared_ptr<ClientInterceptorBase>>>&
  getClientInterceptors() const {
    return clientInterceptors_;
  }

  [[nodiscard]] folly::Try<void> processClientInterceptorsOnRequest(
      ClientInterceptorOnRequestArguments arguments,
      apache::thrift::transport::THeader* headers,
      RpcOptions& options) noexcept;

  // RPC responses may be exceptions or values. If exception_wrapper has
  // a valid exception "result" is ignored. If exception_wrapper is empty
  // "result" is used as the response.
  // This formulation, requiring this method to resolve this, simplifies client
  // stub code generation.
  [[nodiscard]] folly::Try<void> processClientInterceptorsOnResponse(
      const apache::thrift::transport::THeader* headers,
      folly::exception_wrapper exceptionWrapper,
      apache::thrift::util::TypeErasedRef result =
          apache::thrift::util::TypeErasedRef::of<folly::Unit>(
              folly::unit)) noexcept;

  template <typename T>
  [[nodiscard]] folly::Try<void> processClientInterceptorsOnResponse(
      const apache::thrift::transport::THeader* headers,
      folly::exception_wrapper exceptionWrapper,
      T& result) noexcept {
    return processClientInterceptorsOnResponse(
        headers,
        exceptionWrapper,
        apache::thrift::util::TypeErasedRef::of<T>(result));
  }

  template <typename T>
  [[nodiscard]] folly::Try<T> processClientInterceptorsOnResponse(
      const apache::thrift::transport::THeader* headers,
      folly::Try<T>&& result) noexcept {
    folly::exception_wrapper exceptionWrapper;
    if (result.hasException()) {
      exceptionWrapper = result.exception();
    }
    apache::thrift::util::TypeErasedRef resultRef = [&]() {
      if (!result.hasValue()) {
        return apache::thrift::util::TypeErasedRef::of<folly::Unit>(
            folly::unit);
      }
      return apache::thrift::util::TypeErasedRef::of<T>(result.value());
    }();
    auto interceptorTry = processClientInterceptorsOnResponse(
        headers, exceptionWrapper, resultRef);
    if (interceptorTry.hasException()) {
      return folly::Try<T>(interceptorTry.exception());
    }
    return std::move(result);
  }

 private:
  std::shared_ptr<std::vector<std::shared_ptr<TProcessorEventHandler>>>
      handlers_;
  std::shared_ptr<std::vector<std::shared_ptr<ClientInterceptorBase>>>
      clientInterceptors_;
  const std::string_view serviceName_;
  // "{service_name}.{method_name}"
  const std::string_view methodNamePrefixed_;
  // "{method_name}", without the service name prefix
  const std::string_view methodNameUnprefixed_;
  void** serviceContexts_;
  InterceptorFrameworkMetadataStorage clientInterceptorFrameworkMetadata_;
  // While the server-side has a Cpp2RequestContext, the client-side "fakes"
  // it with an embedded version. We can't make it nullptr because this is
  // the API used to read/write headers. The root cause of this limitation
  // is that the TProcessorEventHandler API is shared between the client and
  // the server, but is primarily designed for the server-side use case.
  class EmbeddedClientRequestContext;
  using EmbeddedClientContextPtr =
      apache::thrift::util::AllocationColocator<>::Ptr<
          EmbeddedClientRequestContext>;
  EmbeddedClientContextPtr embeddedClientContext_;
  util::AllocationColocator<>::ArrayPtr<
      detail::ClientInterceptorOnRequestStorage>
      clientInterceptorsStorage_;

  ContextStack(
      const std::shared_ptr<
          std::vector<std::shared_ptr<TProcessorEventHandler>>>& handlers,
      std::string_view serviceName,
      std::string_view method,
      void** serviceContexts,
      TConnectionContext* connectionContext);

  ContextStack(
      const std::shared_ptr<
          std::vector<std::shared_ptr<TProcessorEventHandler>>>& handlers,
      const std::shared_ptr<
          std::vector<std::shared_ptr<ClientInterceptorBase>>>&
          clientInterceptors,
      std::string_view serviceName,
      std::string_view method,
      void** serviceContexts,
      EmbeddedClientContextPtr embeddedClientContext,
      util::AllocationColocator<>::ArrayPtr<
          detail::ClientInterceptorOnRequestStorage> clientInterceptorsStorage);

  void*& contextAt(size_t i);

  detail::ClientInterceptorOnRequestStorage*
  getStorageForClientInterceptorOnRequestByIndex(std::size_t index);

  std::unique_ptr<folly::IOBuf> getInterceptorFrameworkMetadata(
      const RpcOptions& rpcOptions);
};

} // namespace apache::thrift
