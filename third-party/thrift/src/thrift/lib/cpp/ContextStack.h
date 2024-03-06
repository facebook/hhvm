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
  // Customly sized allocation is used for ContextStack, so we can't use default
  // unique_ptr deleter.
  struct Deleter {
    void operator()(ContextStack* ptr) {
      ptr->~ContextStack();
      operator delete (ptr, std::align_val_t{alignof(ContextStack)});
    }
  };
  using UniquePtr = std::unique_ptr<ContextStack, Deleter>;

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
  const bool hasClientRequestContext_{false};

  friend struct std::default_delete<apache::thrift::ContextStack>;

  struct WithEmbeddedClientRequestContext {};

  ContextStack(
      const std::shared_ptr<
          std::vector<std::shared_ptr<TProcessorEventHandler>>>& handlers,
      const char* serviceName,
      const char* method,
      TConnectionContext* connectionContext);

  ContextStack(
      WithEmbeddedClientRequestContext,
      const std::shared_ptr<
          std::vector<std::shared_ptr<TProcessorEventHandler>>>& handlers,
      const char* serviceName,
      const char* method,
      TConnectionContext* connectionContext);

  void*& contextAt(size_t i);
};

} // namespace apache::thrift

namespace std {
template <>
struct default_delete<apache::thrift::ContextStack> {
  void operator()(apache::thrift::ContextStack* cs) const;
};
} // namespace std
