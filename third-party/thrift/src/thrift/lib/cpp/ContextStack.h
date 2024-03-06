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

#include <thrift/lib/cpp/SerializedMessage.h>
#include <thrift/lib/cpp/TProcessorEventHandler.h>
#include <thrift/lib/cpp/protocol/TProtocolTypes.h>
#include <thrift/lib/cpp/transport/THeader.h>
#include <thrift/lib/cpp2/util/AllocationColocator.h>

namespace apache::thrift {

class ContextStack;

namespace detail {
class ContextStackInternals {
 public:
  static void*& contextAt(ContextStack&, size_t index);
};
} // namespace detail

class ContextStack {
  friend class EventHandlerBase;
  friend class detail::ContextStackInternals;

 public:
  using UniquePtr =
      apache::thrift::util::AllocationColocator<ContextStack>::Ptr;

  // Note: factory functions return nullptr if handlers is nullptr or empty.
  static UniquePtr create(
      const std::shared_ptr<
          std::vector<std::shared_ptr<TProcessorEventHandler>>>& handlers,
      const char* serviceName,
      const char* method,
      TConnectionContext* connectionContext);

  static UniquePtr createWithClientContext(
      const std::shared_ptr<
          std::vector<std::shared_ptr<TProcessorEventHandler>>>& handlers,
      const char* serviceName,
      const char* method,
      transport::THeader& header);

  static ContextStack::UniquePtr createWithClientContextCopyNames(
      const std::shared_ptr<
          std::vector<std::shared_ptr<TProcessorEventHandler>>>& handlers,
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

  void resetClientRequestContextHeader();

 private:
  std::shared_ptr<std::vector<std::shared_ptr<TProcessorEventHandler>>>
      handlers_;
  const char* const serviceName_;
  const char* const method_;
  void** serviceContexts_;
  // While the server-side has a Cpp2RequestContext, the client-side "fakes" it
  // with an embedded version. We can't make it nullptr because this is the API
  // used to read/write headers. The root cause of this limitation is that the
  // TProcessorEventHandler API is shared between the client and the server, but
  // is primarily designed for the server-side use case.
  class EmbeddedClientRequestContext;
  using EmbeddedClientContextPtr =
      apache::thrift::util::AllocationColocator<>::Ptr<
          EmbeddedClientRequestContext>;
  EmbeddedClientContextPtr embeddedClientContext_;

  ContextStack(
      const std::shared_ptr<
          std::vector<std::shared_ptr<TProcessorEventHandler>>>& handlers,
      const char* serviceName,
      const char* method,
      void** serviceContexts,
      TConnectionContext* connectionContext);

  ContextStack(
      const std::shared_ptr<
          std::vector<std::shared_ptr<TProcessorEventHandler>>>& handlers,
      const char* serviceName,
      const char* method,
      void** serviceContexts,
      EmbeddedClientContextPtr embeddedClientContext);

  void*& contextAt(size_t i);
};

} // namespace apache::thrift
