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

#include <thrift/lib/cpp/StreamEventHandler.h>
#include <thrift/lib/cpp2/async/InterceptorFlags.h>
#include <thrift/lib/cpp2/detail/EventHandlerRuntime.h>
#include <thrift/lib/cpp2/server/Cpp2ConnContext.h>

namespace apache::thrift {

namespace {
std::string_view stripServiceNamePrefix(
    std::string_view method, std::string_view serviceName) {
  if (method.substr(0, serviceName.size()) != serviceName) {
    return method;
  }
  if (method.size() > serviceName.size() && method[serviceName.size()] == '.') {
    return method.substr(serviceName.size() + 1);
  }
  return method;
}
} // namespace

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
    std::string_view serviceName,
    std::string_view method,
    void** serviceContexts,
    TConnectionContext* connectionContext)
    : handlers_(handlers),
      serviceName_(serviceName),
      methodNamePrefixed_(method),
      methodNameUnprefixed_(
          stripServiceNamePrefix(methodNamePrefixed_, serviceName_)),
      serviceContexts_(serviceContexts),
      clientInterceptorFrameworkMetadata_{
          detail::initializeInterceptorFrameworkMetadataStorage()} {
  if (!handlers_ || handlers_->empty()) {
    return;
  }
  for (size_t i = 0; i < handlers_->size(); ++i) {
    auto* handler = (*handlers_)[i].get();
    CHECK(handler != nullptr);
    contextAt(i) = handler->getServiceContext(
        serviceName_, methodNamePrefixed_, connectionContext);
  }
}

ContextStack::ContextStack(
    const std::shared_ptr<std::vector<std::shared_ptr<TProcessorEventHandler>>>&
        handlers,
    const std::shared_ptr<std::vector<std::shared_ptr<ClientInterceptorBase>>>&
        clientInterceptors,
    std::string_view serviceName,
    std::string_view method,
    void** serviceContexts,
    EmbeddedClientContextPtr embeddedClientContext,
    util::AllocationColocator<>::ArrayPtr<
        detail::ClientInterceptorOnRequestStorage> clientInterceptorsStorage)
    : ContextStack(
          handlers,
          serviceName,
          method,
          serviceContexts,
          embeddedClientContext.get()) {
  clientInterceptors_ = clientInterceptors;
  embeddedClientContext_ = std::move(embeddedClientContext);
  clientInterceptorsStorage_ = std::move(clientInterceptorsStorage);
}

ContextStack::~ContextStack() {
  if (handlers_) {
    for (size_t i = 0; i < handlers_->size(); i++) {
      (*handlers_)[i]->freeContext(contextAt(i), methodNamePrefixed_);
    }
  }
}

ContextStack::UniquePtr ContextStack::create(
    const std::shared_ptr<std::vector<std::shared_ptr<TProcessorEventHandler>>>&
        handlers,
    std::string_view serviceName,
    std::string_view method,
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
    const std::shared_ptr<std::vector<std::shared_ptr<ClientInterceptorBase>>>&
        clientInterceptors,
    std::string_view serviceName,
    std::string_view method,
    transport::THeader& header) {
  if ((!handlers || handlers->empty()) &&
      (!clientInterceptors || clientInterceptors->empty())) {
    return nullptr;
  }
  if (apache::thrift::detail::EventHandlerRuntime::isClientMethodBypassed(
          serviceName, method)) {
    return nullptr;
  }
  const auto numLegacyEventHandlers =
      handlers == nullptr ? 0 : handlers->size();
  const auto numClientInterceptors =
      clientInterceptors == nullptr ? 0 : clientInterceptors->size();

  AllocationColocator<ContextStack> alloc;
  return alloc.allocate(
      [&,
       embeddedClientContext = alloc.object<EmbeddedClientRequestContext>(),
       contexts = alloc.array<void*>(numLegacyEventHandlers),
       clientInterceptorsStorage =
           alloc.array<detail::ClientInterceptorOnRequestStorage>(
               numClientInterceptors)](auto make) mutable {
        return ContextStack(
            handlers,
            clientInterceptors,
            serviceName,
            method,
            make(std::move(contexts)),
            EmbeddedClientContextPtr(
                make(std::move(embeddedClientContext), &header)),
            make(std::move(clientInterceptorsStorage), [] {
              return detail::ClientInterceptorOnRequestStorage();
            }));
      });
}

ContextStack::UniquePtr ContextStack::createWithClientContextCopyNames(
    const std::shared_ptr<std::vector<std::shared_ptr<TProcessorEventHandler>>>&
        handlers,
    const std::shared_ptr<std::vector<std::shared_ptr<ClientInterceptorBase>>>&
        clientInterceptors,
    const std::string& serviceName,
    const std::string& methodName,
    transport::THeader& header) {
  if ((!handlers || handlers->empty()) &&
      (!clientInterceptors || clientInterceptors->empty())) {
    return nullptr;
  }
  if (apache::thrift::detail::EventHandlerRuntime::isClientMethodBypassed(
          serviceName, methodName)) {
    return nullptr;
  }
  const auto numLegacyEventHandlers =
      handlers == nullptr ? 0 : handlers->size();
  const auto numClientInterceptors =
      clientInterceptors == nullptr ? 0 : clientInterceptors->size();

  AllocationColocator<ContextStack> alloc;
  return alloc.allocate(
      [&,
       embeddedClientContext = alloc.object<EmbeddedClientRequestContext>(),
       contexts = alloc.array<void*>(numLegacyEventHandlers),
       clientInterceptorsStorage =
           alloc.array<detail::ClientInterceptorOnRequestStorage>(
               numClientInterceptors),
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
            clientInterceptors,
            make(std::move(serviceNameStorage), serviceName).data(),
            methodNamePtr,
            make(std::move(contexts)),
            EmbeddedClientContextPtr(
                make(std::move(embeddedClientContext), &header)),
            make(std::move(clientInterceptorsStorage), [] {
              return detail::ClientInterceptorOnRequestStorage();
            }));
      });
}

void ContextStack::preWrite() {
  FOLLY_SDT(
      thrift,
      thrift_context_stack_pre_write,
      serviceName_,
      methodNamePrefixed_);

  if (handlers_) {
    for (size_t i = 0; i < handlers_->size(); i++) {
      (*handlers_)[i]->preWrite(contextAt(i), methodNamePrefixed_);
    }
  }
}

void ContextStack::onWriteData(const SerializedMessage& msg) {
  FOLLY_SDT(
      thrift,
      thrift_context_stack_on_write_data,
      serviceName_,
      methodNamePrefixed_);

  if (handlers_) {
    for (size_t i = 0; i < handlers_->size(); i++) {
      (*handlers_)[i]->onWriteData(contextAt(i), methodNamePrefixed_, msg);
    }
  }
}

void ContextStack::postWrite(uint32_t bytes) {
  FOLLY_SDT(
      thrift,
      thrift_context_stack_post_write,
      serviceName_,
      methodNamePrefixed_,
      bytes);

  if (handlers_) {
    for (size_t i = 0; i < handlers_->size(); i++) {
      (*handlers_)[i]->postWrite(contextAt(i), methodNamePrefixed_, bytes);
    }
  }
}

void ContextStack::preRead() {
  FOLLY_SDT(
      thrift, thrift_context_stack_pre_read, serviceName_, methodNamePrefixed_);

  if (handlers_) {
    for (size_t i = 0; i < handlers_->size(); i++) {
      (*handlers_)[i]->preRead(contextAt(i), methodNamePrefixed_);
    }
  }
}

void ContextStack::onReadData(const SerializedMessage& msg) {
  FOLLY_SDT(
      thrift,
      thrift_context_stack_on_read_data,
      serviceName_,
      methodNamePrefixed_);

  if (handlers_) {
    for (size_t i = 0; i < handlers_->size(); i++) {
      (*handlers_)[i]->onReadData(contextAt(i), methodNamePrefixed_, msg);
    }
  }
}

void ContextStack::postRead(
    apache::thrift::transport::THeader* header, uint32_t bytes) {
  FOLLY_SDT(
      thrift,
      thrift_context_stack_post_read,
      serviceName_,
      methodNamePrefixed_,
      bytes);

  if (handlers_) {
    for (size_t i = 0; i < handlers_->size(); i++) {
      (*handlers_)[i]->postRead(
          contextAt(i), methodNamePrefixed_, header, bytes);
    }
  }
}

void ContextStack::onInteractionTerminate(int64_t id) {
  FOLLY_SDT(
      thrift, thrift_context_stack_on_interaction_terminate, serviceName_, id);

  if (handlers_) {
    for (size_t i = 0; i < handlers_->size(); i++) {
      (*handlers_)[i]->onInteractionTerminate(contextAt(i), id);
    }
  }
}

void ContextStack::handlerErrorWrapped(const folly::exception_wrapper& ew) {
  FOLLY_SDT(
      thrift,
      thrift_context_stack_handler_error_wrapped,
      serviceName_,
      methodNamePrefixed_);

  if (handlers_) {
    for (size_t i = 0; i < handlers_->size(); i++) {
      (*handlers_)[i]->handlerErrorWrapped(
          contextAt(i), methodNamePrefixed_, ew);
    }
  }
}

void ContextStack::userExceptionWrapped(
    bool declared, const folly::exception_wrapper& ew) {
  FOLLY_SDT(
      thrift,
      thrift_context_stack_user_exception_wrapped,
      serviceName_,
      methodNamePrefixed_);

  if (handlers_) {
    for (size_t i = 0; i < handlers_->size(); i++) {
      (*handlers_)[i]->userExceptionWrapped(
          contextAt(i), methodNamePrefixed_, declared, ew);
    }
  }
}

void ContextStack::onStreamSubscribe() {
  FOLLY_SDT(
      thrift,
      thrift_context_stack_on_stream_subscribe,
      serviceName_,
      methodNamePrefixed_);

  if (handlers_) {
    for (size_t i = 0; i < handlers_->size(); i++) {
      if (auto* streamEventHandler = (*handlers_)[i]->getStreamEventHandler()) {
        streamEventHandler->onStreamSubscribe(contextAt(i));
      }
    }
  }
}

void ContextStack::onStreamCredit(uint32_t credits) {
  FOLLY_SDT(
      thrift,
      thrift_context_stack_on_credit_received,
      serviceName_,
      methodNamePrefixed_);
  if (handlers_) {
    for (size_t i = 0; i < handlers_->size(); i++) {
      if (auto* streamEventHandler = (*handlers_)[i]->getStreamEventHandler()) {
        streamEventHandler->onStreamCredit(contextAt(i), credits);
      }
    }
  }
}

void ContextStack::onStreamNext() {
  FOLLY_SDT(
      thrift,
      thrift_context_stack_on_stream_item,
      serviceName_,
      methodNamePrefixed_);
  if (handlers_) {
    for (size_t i = 0; i < handlers_->size(); i++) {
      if (auto* streamEventHandler = (*handlers_)[i]->getStreamEventHandler()) {
        streamEventHandler->onStreamNext(contextAt(i));
      }
    }
  }
}

void ContextStack::onStreamPauseReceive() {
  FOLLY_SDT(
      thrift,
      thrift_context_stack_on_stream_pause_receive,
      serviceName_,
      methodNamePrefixed_);

  if (handlers_) {
    for (size_t i = 0; i < handlers_->size(); i++) {
      if (auto* streamEventHandler = (*handlers_)[i]->getStreamEventHandler()) {
        streamEventHandler->onStreamPauseReceive(contextAt(i));
      }
    }
  }
}

void ContextStack::onStreamResumeReceive() {
  FOLLY_SDT(
      thrift,
      thrift_context_stack_on_stream_resume_receive,
      serviceName_,
      methodNamePrefixed_);

  if (handlers_) {
    for (size_t i = 0; i < handlers_->size(); i++) {
      if (auto* streamEventHandler = (*handlers_)[i]->getStreamEventHandler()) {
        streamEventHandler->onStreamResumeReceive(contextAt(i));
      }
    }
  }
}

void ContextStack::handleStreamErrorWrapped(
    const folly::exception_wrapper& ew) {
  FOLLY_SDT(
      thrift,
      thrift_context_stack_on_stream_error,
      serviceName_,
      methodNamePrefixed_);

  if (handlers_) {
    for (size_t i = 0; i < handlers_->size(); i++) {
      if (auto* streamEventHandler = (*handlers_)[i]->getStreamEventHandler()) {
        streamEventHandler->handleStreamErrorWrapped(contextAt(i), ew);
      }
    }
  }
}

void ContextStack::onStreamFinally(details::STREAM_ENDING_TYPES endReason) {
  FOLLY_SDT(
      thrift,
      thrift_context_stack_on_stream_finally,
      serviceName_,
      methodNamePrefixed_);

  if (handlers_) {
    for (size_t i = 0; i < handlers_->size(); i++) {
      if (auto* streamEventHandler = (*handlers_)[i]->getStreamEventHandler()) {
        streamEventHandler->onStreamFinally(contextAt(i), endReason);
      }
    }
  }
}

void ContextStack::onSinkSubscribe(
    const StreamEventHandler::StreamContext& streamCtx) {
  FOLLY_SDT(
      thrift,
      thrift_context_stack_on_sink_established,
      serviceName_,
      methodNamePrefixed_);
  if (handlers_) {
    for (size_t i = 0; i < handlers_->size(); i++) {
      if (auto* streamEventHandler = (*handlers_)[i]->getStreamEventHandler()) {
        streamEventHandler->onSinkSubscribe(contextAt(i), streamCtx);
      }
    }
  }
}

void ContextStack::onSinkNext() {
  FOLLY_SDT(
      thrift,
      thrift_context_stack_on_sink_item,
      serviceName_,
      methodNamePrefixed_);
  if (handlers_) {
    for (size_t i = 0; i < handlers_->size(); i++) {
      if (auto* streamEventHandler = (*handlers_)[i]->getStreamEventHandler()) {
        streamEventHandler->onSinkNext(contextAt(i));
      }
    }
  }
}

void ContextStack::onSinkConsumed() {
  FOLLY_SDT(
      thrift,
      thrift_context_stack_on_sink_item,
      serviceName_,
      methodNamePrefixed_);
  if (handlers_) {
    for (size_t i = 0; i < handlers_->size(); i++) {
      if (auto* streamEventHandler = (*handlers_)[i]->getStreamEventHandler()) {
        streamEventHandler->onSinkConsumed(contextAt(i));
      }
    }
  }
}

void ContextStack::onSinkCancel() {
  FOLLY_SDT(
      thrift,
      thrift_context_stack_on_sink_cancel,
      serviceName_,
      methodNamePrefixed_);
  if (handlers_) {
    for (size_t i = 0; i < handlers_->size(); i++) {
      if (auto* streamEventHandler = (*handlers_)[i]->getStreamEventHandler()) {
        streamEventHandler->onSinkCancel(contextAt(i));
      }
    }
  }
}

void ContextStack::onSinkCredit(uint32_t credits) {
  FOLLY_SDT(
      thrift,
      thrift_context_stack_on_credit_received,
      serviceName_,
      methodNamePrefixed_);
  if (handlers_) {
    for (size_t i = 0; i < handlers_->size(); i++) {
      if (auto* streamEventHandler = (*handlers_)[i]->getStreamEventHandler()) {
        streamEventHandler->onSinkCredit(contextAt(i), credits);
      }
    }
  }
}

void ContextStack::onSinkFinally(details::SINK_ENDING_TYPES endReason) {
  FOLLY_SDT(
      thrift,
      thrift_context_stack_on_sink_finally,
      serviceName_,
      methodNamePrefixed_);

  if (handlers_) {
    for (size_t i = 0; i < handlers_->size(); i++) {
      if (auto* streamEventHandler = (*handlers_)[i]->getStreamEventHandler()) {
        streamEventHandler->onSinkFinally(contextAt(i), endReason);
      }
    }
  }
}

void ContextStack::handleSinkError(const folly::exception_wrapper& ew) {
  FOLLY_SDT(
      thrift,
      thrift_context_stack_on_sink_error,
      serviceName_,
      methodNamePrefixed_);

  if (handlers_) {
    for (size_t i = 0; i < handlers_->size(); i++) {
      if (auto* streamEventHandler = (*handlers_)[i]->getStreamEventHandler()) {
        streamEventHandler->handleSinkError(contextAt(i), ew);
      }
    }
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

folly::Try<void> ContextStack::processClientInterceptorsOnRequest(
    ClientInterceptorOnRequestArguments arguments,
    apache::thrift::transport::THeader* headers,
    RpcOptions& options) noexcept {
  if (clientInterceptors_ == nullptr) {
    return {};
  }
  std::vector<ClientInterceptorException::SingleExceptionInfo> exceptions;
  for (std::size_t i = 0; i < clientInterceptors_->size(); ++i) {
    const auto& clientInterceptor = (*clientInterceptors_)[i];
    ClientInterceptorBase::RequestInfo requestInfo{
        getStorageForClientInterceptorOnRequestByIndex(i),
        arguments,
        headers,
        serviceName_,
        methodNameUnprefixed_,
        &clientInterceptorFrameworkMetadata_,
        &options};
    try {
      clientInterceptor->internal_onRequest(std::move(requestInfo));
    } catch (...) {
      exceptions.emplace_back(ClientInterceptorException::SingleExceptionInfo{
          clientInterceptor->getName(),
          folly::exception_wrapper(folly::current_exception())});
    }
  }

  if (!exceptions.empty()) {
    return folly::Try<void>(
        folly::make_exception_wrapper<ClientInterceptorException>(
            ClientInterceptorException::CallbackKind::ON_REQUEST,
            std::move(exceptions)));
  }
  return {};
}

folly::Try<void> ContextStack::processClientInterceptorsOnResponse(
    const apache::thrift::transport::THeader* headers) noexcept {
  if (clientInterceptors_ == nullptr) {
    return {};
  }
  std::vector<ClientInterceptorException::SingleExceptionInfo> exceptions;
  for (auto i = std::ptrdiff_t(clientInterceptors_->size()) - 1; i >= 0; --i) {
    const auto& clientInterceptor = (*clientInterceptors_)[i];
    ClientInterceptorBase::ResponseInfo responseInfo{
        getStorageForClientInterceptorOnRequestByIndex(i),
        headers,
        serviceName_,
        methodNameUnprefixed_};
    try {
      clientInterceptor->internal_onResponse(std::move(responseInfo));
    } catch (...) {
      exceptions.emplace_back(ClientInterceptorException::SingleExceptionInfo{
          clientInterceptor->getName(),
          folly::exception_wrapper(folly::current_exception())});
    }
  }

  if (!exceptions.empty()) {
    return folly::Try<void>(
        folly::make_exception_wrapper<ClientInterceptorException>(
            ClientInterceptorException::CallbackKind::ON_RESPONSE,
            std::move(exceptions)));
  }
  return {};
}

void*& ContextStack::contextAt(size_t i) {
  return serviceContexts_[i];
}

detail::ClientInterceptorOnRequestStorage*
ContextStack::getStorageForClientInterceptorOnRequestByIndex(
    std::size_t index) {
  DCHECK(clientInterceptors_);
  DCHECK_LT(index, clientInterceptors_->size());
  return &clientInterceptorsStorage_[index];
}

std::unique_ptr<folly::IOBuf> ContextStack::getInterceptorFrameworkMetadata(
    const RpcOptions& rpcOptions) {
  detail::postProcessFrameworkMetadata(
      clientInterceptorFrameworkMetadata_, rpcOptions);
  return detail::serializeFrameworkMetadata(
      std::move(clientInterceptorFrameworkMetadata_));
}

namespace detail {
ContextStackUnsafeAPI::ContextStackUnsafeAPI(ContextStack& contextStack)
    : contextStack_{contextStack} {}

void*& ContextStackUnsafeAPI::contextAt(size_t index) const {
  return contextStack_.contextAt(index);
}

std::unique_ptr<folly::IOBuf>
ContextStackUnsafeAPI::getInterceptorFrameworkMetadata(
    const RpcOptions& rpcOptions) {
  if (!THRIFT_FLAG(enable_client_interceptor_framework_metadata)) {
    return nullptr;
  }
  return contextStack_.getInterceptorFrameworkMetadata(rpcOptions);
}
} // namespace detail

} // namespace apache::thrift
