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

ContextStack::ContextStack(
    const std::shared_ptr<std::vector<std::shared_ptr<TProcessorEventHandler>>>&
        handlers,
    const char* serviceName,
    const char* method,
    TConnectionContext* connectionContext)
    : handlers_(handlers), serviceName_(serviceName), method_(method) {
  CHECK(handlers_ && !handlers_->empty());
  for (size_t i = 0; i < handlers_->size(); ++i) {
    contextAt(i) = (*handlers_)[i]->getServiceContext(
        serviceName_, method_, connectionContext);
  }
}

ContextStack::ContextStack(
    WithEmbeddedClientRequestContext,
    const std::shared_ptr<std::vector<std::shared_ptr<TProcessorEventHandler>>>&
        handlers,
    const char* serviceName,
    const char* method,
    TConnectionContext* connectionContext)
    : handlers_(handlers),
      serviceName_(serviceName),
      method_(method),
      hasClientRequestContext_(true) {
  CHECK(handlers_ && !handlers_->empty());
  for (size_t i = 0; i < handlers_->size(); ++i) {
    contextAt(i) = (*handlers_)[i]->getServiceContext(
        serviceName_, method_, connectionContext);
  }
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

  const size_t nbytes = sizeof(ContextStack) + handlers->size() * sizeof(void*);
  auto* storage = static_cast<ContextStack*>(operator new (
      nbytes, std::align_val_t{alignof(ContextStack)}));
  auto* object = new (storage)
      ContextStack(handlers, serviceName, method, connectionContext);

  return ContextStack::UniquePtr(object);
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

  const size_t nbytes = sizeof(ContextStack) +
      sizeof(Cpp2ClientRequestContext) + handlers->size() * sizeof(void*);
  auto* storage = static_cast<ContextStack*>(operator new (
      nbytes, std::align_val_t{alignof(ContextStack)}));

  auto* connectionContext = new (storage + 1) Cpp2ClientRequestContext(&header);

  auto* object = new (storage) ContextStack(
      WithEmbeddedClientRequestContext(),
      handlers,
      serviceName,
      method,
      connectionContext);

  return ContextStack::UniquePtr(object);
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

  size_t serviceNameBytes = serviceName.size() + 1;
  size_t methodNameBytes = serviceName.size() + 1 + methodName.size() + 1;

  const size_t nbytes = sizeof(ContextStack) +
      sizeof(Cpp2ClientRequestContext) + handlers->size() * sizeof(void*) +
      serviceNameBytes + methodNameBytes;
  auto* storage = static_cast<ContextStack*>(operator new (
      nbytes, std::align_val_t{alignof(ContextStack)}));

  auto* connectionContext = new (storage + 1) Cpp2ClientRequestContext(&header);
  auto serviceNameStorage = reinterpret_cast<char*>(storage) + nbytes -
      serviceNameBytes - methodNameBytes;
  auto methodNameStorage = serviceNameStorage + serviceNameBytes;
  snprintf(serviceNameStorage, serviceNameBytes, "%s", serviceName.c_str());
  snprintf(
      methodNameStorage,
      methodNameBytes,
      "%s.%s",
      serviceName.c_str(),
      methodName.c_str());

  auto* object = new (storage) ContextStack(
      WithEmbeddedClientRequestContext(),
      handlers,
      serviceNameStorage,
      methodNameStorage,
      connectionContext);

  return ContextStack::UniquePtr(object);
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
  if (!hasClientRequestContext_) {
    return;
  }

  auto* connectionContext =
      reinterpret_cast<Cpp2ClientRequestContext*>(this + 1);
  connectionContext->setRequestHeader(nullptr);
}

void*& ContextStack::contextAt(size_t i) {
  void** start = reinterpret_cast<void**>(this + 1);
  if (hasClientRequestContext_) {
    start = reinterpret_cast<void**>(
        reinterpret_cast<Cpp2ClientRequestContext*>(start) + 1);
  }
  return start[i];
}

} // namespace thrift
} // namespace apache

namespace std {
void default_delete<apache::thrift::ContextStack>::operator()(
    apache::thrift::ContextStack* cs) const {
  if (cs) {
    const size_t nbytes = sizeof(apache::thrift::ContextStack) +
        (cs->hasClientRequestContext_
             ? sizeof(apache::thrift::Cpp2ClientRequestContext)
             : 0) +
        cs->handlers_->size() * sizeof(void*);

    auto* connectionContext = cs->hasClientRequestContext_
        ? reinterpret_cast<apache::thrift::Cpp2ClientRequestContext*>(cs + 1)
        : nullptr;

    cs->~ContextStack();
    if (connectionContext) {
      connectionContext->~Cpp2ClientRequestContext();
    }

    operator delete (
        cs, nbytes, std::align_val_t{alignof(apache::thrift::ContextStack)});
  }
}
} // namespace std
