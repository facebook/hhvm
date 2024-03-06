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

#include <cstdio>
#include <thrift/lib/cpp/ContextStack.h>

#include <folly/tracing/StaticTracepoint.h>

#include <thrift/lib/cpp2/detail/EventHandlerRuntime.h>
#include <thrift/lib/cpp2/server/Cpp2ConnContext.h>

namespace apache {
namespace thrift {

using util::AllocationColocator;

class ContextStack::EmbeddedClientRequestContext
    : public apache::thrift::server::TConnectionContext {
 public:
  explicit EmbeddedClientRequestContext(transport::THeader* header)
      : TConnectionContext(header) {}

  void resetRequestHeader() { header_ = nullptr; }
};

ContextStack::ContextStack(
    const std::shared_ptr<std::vector<std::shared_ptr<TProcessorEventHandler>>>&
        handlers,
    const char* serviceName,
    const char* method,
    void** serviceContexts,
    TConnectionContext* connectionContext)
    : handlers_(handlers),
      serviceName_(serviceName),
      method_(method),
      serviceContexts_(serviceContexts) {
  CHECK(handlers_ && !handlers_->empty());
  for (size_t i = 0; i < handlers_->size(); ++i) {
    contextAt(i) = (*handlers_)[i]->getServiceContext(
        serviceName_, method_, connectionContext);
  }
}

ContextStack::ContextStack(
    const std::shared_ptr<std::vector<std::shared_ptr<TProcessorEventHandler>>>&
        handlers,
    const char* serviceName,
    const char* method,
    void** serviceContexts,
    EmbeddedClientContextPtr embeddedClientContext)
    : ContextStack(
          handlers,
          serviceName,
          method,
          serviceContexts,
          embeddedClientContext.get()) {
  embeddedClientContext_ = std::move(embeddedClientContext);
}

ContextStack::~ContextStack() {
  for (size_t i = 0; i < handlers_->size(); i++) {
    (*handlers_)[i]->freeContext(contextAt(i), method_);
  }
}

ContextStack::UniquePtr ContextStack::create(
    const std::shared_ptr<std::vector<std::shared_ptr<TProcessorEventHandler>>>&
        handlers,
    const char* serviceName,
    const char* method,
    TConnectionContext* connectionContext) {
  if (!handlers || handlers->empty()) {
    return nullptr;
  }

  AllocationColocator<ContextStack> alloc;
  return alloc.allocate(
      [&, contexts = alloc.array<void*>(handlers->size())](auto make) mutable {
        return ContextStack(
            handlers,
            serviceName,
            method,
            make(std::move(contexts)),
            connectionContext);
      });
}

ContextStack::UniquePtr ContextStack::createWithClientContext(
    const std::shared_ptr<std::vector<std::shared_ptr<TProcessorEventHandler>>>&
        handlers,
    const char* serviceName,
    const char* method,
    transport::THeader& header) {
  if (!handlers || handlers->empty()) {
    return nullptr;
  }
  if (apache::thrift::detail::EventHandlerRuntime::isClientMethodBypassed(
          serviceName, method)) {
    return nullptr;
  }

  AllocationColocator<ContextStack> alloc;
  return alloc.allocate(
      [&,
       embeddedClientContext = alloc.object<EmbeddedClientRequestContext>(),
       contexts = alloc.array<void*>(handlers->size())](auto make) mutable {
        return ContextStack(
            handlers,
            serviceName,
            method,
            make(std::move(contexts)),
            EmbeddedClientContextPtr(
                make(std::move(embeddedClientContext), &header)));
      });
}

ContextStack::UniquePtr ContextStack::createWithClientContextCopyNames(
    const std::shared_ptr<std::vector<std::shared_ptr<TProcessorEventHandler>>>&
        handlers,
    const std::string& serviceName,
    const std::string& methodName,
    transport::THeader& header) {
  if (!handlers || handlers->empty()) {
    return nullptr;
  }
  if (apache::thrift::detail::EventHandlerRuntime::isClientMethodBypassed(
          serviceName, methodName)) {
    return nullptr;
  }

  AllocationColocator<ContextStack> alloc;
  return alloc.allocate(
      [&,
       embeddedClientContext = alloc.object<EmbeddedClientRequestContext>(),
       contexts = alloc.array<void*>(handlers->size()),
       serviceNameStorage = alloc.string(serviceName.size()),
       methodNameStorage =
           alloc.string(serviceName.size() + 1 /* dot */ + methodName.size())](
          auto make) mutable {
        // Unlike C++, thrift-python (whose implementation also requires
        // extending string lifetimes) does not prefix the method name with
        // "Service.". So we are formatting here for consistency.
        const std::size_t methodNameBytes = methodNameStorage.length + 1;
        auto methodNamePtr = reinterpret_cast<char*>(
            make.uninitialized(std::move(methodNameStorage)));
        std::snprintf(
            methodNamePtr,
            methodNameBytes,
            "%s.%s",
            serviceName.c_str(),
            methodName.c_str());

        return ContextStack(
            handlers,
            make(std::move(serviceNameStorage), serviceName).data(),
            methodNamePtr,
            make(std::move(contexts)),
            EmbeddedClientContextPtr(
                make(std::move(embeddedClientContext), &header)));
      });
}

void ContextStack::preWrite() {
  FOLLY_SDT(thrift, thrift_context_stack_pre_write, serviceName_, method_);

  if (handlers_) {
    for (size_t i = 0; i < handlers_->size(); i++) {
      (*handlers_)[i]->preWrite(contextAt(i), method_);
    }
  }
}

void ContextStack::onWriteData(const SerializedMessage& msg) {
  FOLLY_SDT(thrift, thrift_context_stack_on_write_data, serviceName_, method_);

  for (size_t i = 0; i < handlers_->size(); i++) {
    (*handlers_)[i]->onWriteData(contextAt(i), method_, msg);
  }
}

void ContextStack::postWrite(uint32_t bytes) {
  FOLLY_SDT(
      thrift, thrift_context_stack_post_write, serviceName_, method_, bytes);

  for (size_t i = 0; i < handlers_->size(); i++) {
    (*handlers_)[i]->postWrite(contextAt(i), method_, bytes);
  }
}

void ContextStack::preRead() {
  FOLLY_SDT(thrift, thrift_context_stack_pre_read, serviceName_, method_);

  for (size_t i = 0; i < handlers_->size(); i++) {
    (*handlers_)[i]->preRead(contextAt(i), method_);
  }
}

void ContextStack::onReadData(const SerializedMessage& msg) {
  FOLLY_SDT(thrift, thrift_context_stack_on_read_data, serviceName_, method_);

  for (size_t i = 0; i < handlers_->size(); i++) {
    (*handlers_)[i]->onReadData(contextAt(i), method_, msg);
  }
}

void ContextStack::postRead(
    apache::thrift::transport::THeader* header, uint32_t bytes) {
  FOLLY_SDT(
      thrift, thrift_context_stack_post_read, serviceName_, method_, bytes);

  for (size_t i = 0; i < handlers_->size(); i++) {
    (*handlers_)[i]->postRead(contextAt(i), method_, header, bytes);
  }
}

void ContextStack::onInteractionTerminate(int64_t id) {
  FOLLY_SDT(
      thrift, thrift_context_stack_on_interaction_terminate, serviceName_, id);

  for (size_t i = 0; i < handlers_->size(); i++) {
    (*handlers_)[i]->onInteractionTerminate(contextAt(i), id);
  }
}

void ContextStack::handlerErrorWrapped(const folly::exception_wrapper& ew) {
  FOLLY_SDT(
      thrift,
      thrift_context_stack_handler_error_wrapped,
      serviceName_,
      method_);

  for (size_t i = 0; i < handlers_->size(); i++) {
    (*handlers_)[i]->handlerErrorWrapped(contextAt(i), method_, ew);
  }
}

void ContextStack::userExceptionWrapped(
    bool declared, const folly::exception_wrapper& ew) {
  FOLLY_SDT(
      thrift,
      thrift_context_stack_user_exception_wrapped,
      serviceName_,
      method_);

  for (size_t i = 0; i < handlers_->size(); i++) {
    (*handlers_)[i]->userExceptionWrapped(contextAt(i), method_, declared, ew);
  }
}

void ContextStack::resetClientRequestContextHeader() {
  if (embeddedClientContext_ == nullptr) {
    return;
  }
  auto* connectionContext =
      static_cast<EmbeddedClientRequestContext*>(embeddedClientContext_.get());
  connectionContext->resetRequestHeader();
}

void*& ContextStack::contextAt(size_t i) {
  return serviceContexts_[i];
}

namespace detail {
/* static */ void*& ContextStackInternals::contextAt(
    ContextStack& contextStack, size_t index) {
  return contextStack.contextAt(index);
}
} // namespace detail

} // namespace thrift
} // namespace apache
