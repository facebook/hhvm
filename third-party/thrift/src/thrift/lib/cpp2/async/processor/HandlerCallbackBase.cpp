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

#include <folly/ExceptionWrapper.h>
#include <folly/Executor.h>
#include <thrift/lib/cpp/TApplicationException.h>
#include <thrift/lib/cpp/concurrency/ThreadManager.h>
#include <thrift/lib/cpp2/async/AsyncProcessorFactory.h>
#include <thrift/lib/cpp2/async/Interaction.h>
#include <thrift/lib/cpp2/async/RpcTypes.h>
#include <thrift/lib/cpp2/async/ServerSinkBridge.h>
#include <thrift/lib/cpp2/async/ServerStream.h>
#include <thrift/lib/cpp2/async/processor/HandlerCallbackBase.h>
#include <thrift/lib/cpp2/server/LazyDynamicArguments.h>

// Default to ture, so it can be used for killswitch.
THRIFT_FLAG_DEFINE_bool(thrift_enable_streaming_tracking, false);
#if defined(__linux__) && !FOLLY_MOBILE
/**
 * TALK TO THE THRIFT TEAM
 * BEFORE FLIPPING THIS FLAG!
 */
DEFINE_bool(
    EXPERIMENTAL_thrift_enable_streaming_tracking,
    true,
    "Enable Thrift Streaming Tracking.");
#else
static constexpr bool FLAGS_EXPERIMENTAL_thrift_enable_streaming_tracking =
    true;
#endif

namespace {
bool isStreamTrackingEnabled() {
  return FLAGS_EXPERIMENTAL_thrift_enable_streaming_tracking &&
      THRIFT_FLAG(thrift_enable_streaming_tracking);
}
} // namespace

namespace apache::thrift {
const folly::Executor::KeepAlive<>&
HandlerCallbackBase::getInternalKeepAlive() {
  DCHECK(executor_);
  return executor_;
}

HandlerCallbackBase::~HandlerCallbackBase() {
  maybeNotifyComplete();
  // req must be deleted in the eb
  if (req_) {
    if (req_->isActive() && ewp_) {
      // We must call doException() here instead of exception() because the
      // latter may invoke ServiceInterceptor::onResponse.
      doException(
          std::make_exception_ptr(TApplicationException(
              TApplicationException::INTERNAL_ERROR,
              "apache::thrift::HandlerCallback not completed")));
      return;
    }
    if (getEventBase()) {
      releaseRequest(std::move(req_), getEventBase(), std::move(interaction_));
    }
  }
}

void HandlerCallbackBase::releaseRequest(
    ResponseChannelRequest::UniquePtr request,
    folly::EventBase* eb,
    TilePtr interaction) {
  DCHECK(request);
  DCHECK(eb != nullptr);
  if (!eb->inRunningEventBaseThread()) {
    eb->runInEventBaseThread(
        [req = std::move(request), interaction = std::move(interaction)] {});
  }
}

folly::EventBase* HandlerCallbackBase::getEventBase() {
  return eb_;
}

concurrency::ThreadManager* HandlerCallbackBase::getThreadManager_deprecated() {
  if (reqCtx_) {
    if (auto connCtx = reqCtx_->getConnectionContext()) {
      if (auto workerCtx = connCtx->getWorkerContext()) {
        if (auto serverCtx = workerCtx->getServerContext()) {
          return serverCtx->getThreadManager_deprecated().get();
        }
      }
    }
  }
  return nullptr;
}

bool HandlerCallbackBase::isResourcePoolEnabled() {
  if (reqCtx_) {
    if (auto connCtx = reqCtx_->getConnectionContext()) {
      if (auto workerCtx = connCtx->getWorkerContext()) {
        if (auto serverCtx = workerCtx->getServerContext()) {
          return serverCtx->resourcePoolEnabled();
        }
      }
    }
  }
  return false;
}

folly::Executor* HandlerCallbackBase::getThreadManager() {
  return getHandlerExecutor();
}

folly::Executor* HandlerCallbackBase::getHandlerExecutor() {
  if (!executor_) {
    return getThreadManager_deprecated();
  }
  return executor_.get();
}

folly::Optional<uint32_t> HandlerCallbackBase::checksumIfNeeded(
    LegacySerializedResponse& response) {
  folly::Optional<uint32_t> crc32c;
  if (req_->isReplyChecksumNeeded() && response.buffer &&
      !response.buffer->empty()) {
    static folly::once_flag once;
    folly::call_once(once, [] {
      LOG(WARNING) << "WARNING: Response checksum not implemented";
    });
  }
  return crc32c;
}

folly::Optional<uint32_t> HandlerCallbackBase::checksumIfNeeded(
    SerializedResponse& response) {
  folly::Optional<uint32_t> crc32c;
  if (req_->isReplyChecksumNeeded() && response.buffer &&
      !response.buffer->empty()) {
    static folly::once_flag once;
    folly::call_once(once, [] {
      LOG(WARNING) << "WARNING: Response checksum not implemented";
    });
  }
  return crc32c;
}

ResponsePayload HandlerCallbackBase::transform(ResponsePayload&& payload) {
  // Do any compression or other transforms in this thread, the same thread
  // that serialization happens on.
  payload.transform(reqCtx_->getHeader()->getWriteTransforms());
  return std::move(payload);
}

void HandlerCallbackBase::doExceptionWrapped(folly::exception_wrapper ew) {
  if (req_ == nullptr) {
    LOG(ERROR) << ew.what();
  } else {
    callExceptionInEventBaseThread(ewp_, ew);
  }
}

void HandlerCallbackBase::sendReply(SerializedResponse response) {
  this->ctx_.reset();
  folly::Optional<uint32_t> crc32c = checksumIfNeeded(response);
  auto payload = std::move(response).extractPayload(
      req_->includeEnvelope(),
      reqCtx_->getHeader()->getProtocolId(),
      protoSeqId_,
      MessageType::T_REPLY,
      reqCtx_->getMethodName());
  payload = transform(std::move(payload));
  if (getEventBase() && getEventBase()->inRunningEventBaseThread()) {
    QueueReplyInfo(std::move(req_), std::move(payload), crc32c)();
  } else {
    putMessageInReplyQueue(
        std::in_place_type_t<QueueReplyInfo>(),
        std::move(req_),
        std::move(payload),
        crc32c);
  }
}

void HandlerCallbackBase::sendReply(
    ResponseAndServerStreamFactory&& responseAndStream) {
  if (!isStreamTrackingEnabled()) {
    this->ctx_.reset();
  }
  folly::Optional<uint32_t> crc32c =
      checksumIfNeeded(responseAndStream.response);
  auto payload = std::move(responseAndStream.response)
                     .extractPayload(
                         req_->includeEnvelope(),
                         reqCtx_->getHeader()->getProtocolId(),
                         protoSeqId_,
                         MessageType::T_REPLY,
                         reqCtx_->getMethodName());
  payload = transform(std::move(payload));
  auto& stream = responseAndStream.stream;
  stream.setInteraction(std::move(interaction_));
  stream.setContextStack(std::move(this->ctx_));
  if (getEventBase()->isInEventBaseThread()) {
    StreamReplyInfo(
        std::move(req_), std::move(stream), std::move(payload), crc32c)();
  } else {
    putMessageInReplyQueue(
        std::in_place_type_t<StreamReplyInfo>(),
        std::move(req_),
        std::move(stream),
        std::move(payload),
        crc32c);
  }
}

void HandlerCallbackBase::sendReply(
    [[maybe_unused]] std::pair<
        SerializedResponse,
        apache::thrift::detail::ServerSinkFactory>&& responseAndSinkConsumer) {
  if (!isStreamTrackingEnabled()) {
    this->ctx_.reset();
  }
#if FOLLY_HAS_COROUTINES
  folly::Optional<uint32_t> crc32c =
      checksumIfNeeded(responseAndSinkConsumer.first);
  auto payload = std::move(responseAndSinkConsumer.first)
                     .extractPayload(
                         req_->includeEnvelope(),
                         reqCtx_->getHeader()->getProtocolId(),
                         protoSeqId_,
                         MessageType::T_REPLY,
                         reqCtx_->getMethodName());
  payload = transform(std::move(payload));
  auto& sinkFactory = responseAndSinkConsumer.second;
  sinkFactory.setInteraction(std::move(interaction_));
  sinkFactory.setContextStack(std::move(this->ctx_));

  if (getEventBase()->isInEventBaseThread()) {
    SinkConsumerReplyInfo(
        std::move(req_), std::move(sinkFactory), std::move(payload), crc32c)();
  } else {
    putMessageInReplyQueue(
        std::in_place_type_t<SinkConsumerReplyInfo>(),
        std::move(req_),
        std::move(sinkFactory),
        std::move(payload),
        crc32c);
  }
#else
  std::terminate();
#endif
}

void HandlerCallbackBase::sendReply(
    ResponseAndServerBiDiStreamFactory&& responseAndServerBiDiStreamFactory) {
  folly::Optional<uint32_t> crc32c =
      checksumIfNeeded(responseAndServerBiDiStreamFactory.response);
  auto payload = std::move(responseAndServerBiDiStreamFactory.response)
                     .extractPayload(
                         req_->includeEnvelope(),
                         reqCtx_->getHeader()->getProtocolId(),
                         protoSeqId_,
                         MessageType::T_REPLY,
                         reqCtx_->getMethodName());
  payload = transform(std::move(payload));
  auto& bidiStreamFactory = responseAndServerBiDiStreamFactory.bidiStream;
  bidiStreamFactory.setContextStack(std::move(this->ctx_));
  bidiStreamFactory.setInteraction(std::move(interaction_));
  if (getEventBase()->isInEventBaseThread()) {
    BiDiStreamReplyInfo(
        std::move(req_),
        std::move(bidiStreamFactory),
        std::move(payload),
        crc32c)();
  } else {
    putMessageInReplyQueue(
        std::in_place_type_t<BiDiStreamReplyInfo>(),
        std::move(req_),
        std::move(bidiStreamFactory),
        std::move(payload),
        crc32c);
  }
}

bool HandlerCallbackBase::fulfillTilePromise(std::unique_ptr<Tile> ptr) {
  if (!ptr) {
    DLOG(FATAL) << "Nullptr interaction yielded from handler";
    FOLLY_PUSH_WARNING
    FOLLY_CLANG_DISABLE_WARNING("-Wunreachable-code")
    exception(TApplicationException(
        TApplicationException::MISSING_RESULT,
        "Nullptr interaction yielded from handler"));
    FOLLY_POP_WARNING
    return false;
  }

  putMessageInReplyQueue(
      std::in_place_type_t<TilePromiseReplyInfo>(),
      reqCtx_->getConnectionContext(),
      reqCtx_->getInteractionId(),
      std::move(interaction_),
      std::move(ptr),
      getThreadManager_deprecated(),
      eb_,
      executor_,
      isResourcePoolEnabled());
  return true;
}

void HandlerCallbackBase::breakTilePromise() {
  auto fn = [interaction = std::move(interaction_)]() mutable {
    DCHECK(dynamic_cast<TilePromise*>(interaction.get()));
    static_cast<TilePromise&>(*interaction)
        .failWith(
            folly::make_exception_wrapper<TApplicationException>(
                "Interaction constructor failed"),
            kInteractionConstructorErrorErrorCode);
  };

  eb_->runImmediatelyOrRunInEventBaseThread(std::move(fn));
}

#if FOLLY_HAS_COROUTINES
bool HandlerCallbackBase::shouldProcessServiceInterceptorsOnRequest()
    const noexcept {
  // The chain of objects can be null in unit tests when these objects are
  // mocked.
  if (reqCtx_ != nullptr) {
    if (auto connCtx = reqCtx_->getConnectionContext()) {
      if (auto workerCtx = connCtx->getWorkerContext()) {
        if (auto server = workerCtx->getServerContext()) {
          return server->getServiceInterceptors().size() > 0;
        }
      }
    }
  }
  return false;
}

bool HandlerCallbackBase::shouldProcessServiceInterceptorsOnResponse()
    const noexcept {
  if (!shouldProcessServiceInterceptorsOnRequest()) {
    return false;
  }
  if (intrusivePtrControlBlock_.useCount() == 0) {
    // unsafeRelease() was called on IntrusiveSharedPtr(this) that was passed to
    // the user-defined handler implementation. This means that we cannot
    // guarantee that the request outlives ServiceInterceptorBase::onResponse().
    // So the only safe option is to avoid calling it.
    return false;
  }
  return true;
}

folly::coro::Task<void>
HandlerCallbackBase::processServiceInterceptorsOnRequest(
    detail::ServiceInterceptorOnRequestArguments arguments,
    const SerializedRequest& serializedRequest) {
  if (!shouldProcessServiceInterceptorsOnRequest()) {
    co_return;
  }
  const apache::thrift::server::ServerConfigs* server =
      reqCtx_->getConnectionContext()->getWorkerContext()->getServerContext();
  DCHECK(server);
  const std::vector<std::shared_ptr<ServiceInterceptorBase>>&
      serviceInterceptors = server->getServiceInterceptors();
  std::vector<std::pair<std::size_t, std::exception_ptr>> exceptions;

  // Construct LazyDynamicArguments if schema is available
  std::optional<LazyDynamicArguments> lazyArgs;
  if (serializedRequest.buffer) {
    if (auto* fn = reqCtx_->getFunctionNode()) {
      lazyArgs.emplace(
          serializedRequest.buffer.get(),
          fn,
          static_cast<protocol::PROTOCOL_TYPES>(
              reqCtx_->getHeader()->getProtocolId()));
    }
  }

  for (std::size_t i = 0; i < serviceInterceptors.size(); ++i) {
    auto* connectionCtx = reqCtx_->getConnectionContext();
    auto connectionInfo = ServiceInterceptorBase::ConnectionInfo{
        connectionCtx,
        connectionCtx->getStorageForServiceInterceptorOnConnectionByIndex(i)};
    const server::DecoratorData decoratorData = reqCtx_->getDecoratorData();
    auto requestInfo = ServiceInterceptorBase::RequestInfo{
        .context = reqCtx_,
        .storage = reqCtx_->getStorageForServiceInterceptorOnRequestByIndex(i),
        .arguments = arguments,
        .serviceName = methodNameInfo_.serviceName,
        .definingServiceName = methodNameInfo_.definingServiceName,
        .methodName = methodNameInfo_.methodName,
        .qualifiedMethodName = methodNameInfo_.qualifiedMethodName,
        .frameworkMetadata = reqCtx_->getInterceptorFrameworkMetadata(),
        .decoratorData = &decoratorData,
        .serializedRequestBuffer = serializedRequest.buffer.get(),
        .dynamicArguments = lazyArgs.has_value() ? &*lazyArgs : nullptr,
    };
    try {
      co_await serviceInterceptors[i]->internal_onRequest(
          connectionInfo, requestInfo, server->getInterceptorMetricCallback());
    } catch (...) {
      exceptions.emplace_back(i, folly::current_exception());
    }
  }
  if (!exceptions.empty()) {
    std::string message = fmt::format(
        "ServiceInterceptor::onRequest threw exceptions:\n[{}] {}\n",
        serviceInterceptors[exceptions[0].first]->getQualifiedName().get(),
        folly::exceptionStr(exceptions[0].second));
    for (std::size_t i = 1; i < exceptions.size(); ++i) {
      message += fmt::format(
          "[{}] {}\n",
          serviceInterceptors[exceptions[i].first]->getQualifiedName().get(),
          folly::exceptionStr(exceptions[i].second));
    }
    co_yield folly::coro::co_error(TApplicationException(message));
  }
}

folly::coro::Task<void>
HandlerCallbackBase::processServiceInterceptorsOnResponse(
    detail::ServiceInterceptorOnResponseResult resultOrActiveException) {
  if (!shouldProcessServiceInterceptorsOnResponse()) {
    co_return;
  }
  const apache::thrift::server::ServerConfigs* server =
      reqCtx_->getConnectionContext()->getWorkerContext()->getServerContext();
  DCHECK(server);
  const std::vector<std::shared_ptr<ServiceInterceptorBase>>&
      serviceInterceptors = server->getServiceInterceptors();
  std::vector<std::pair<std::size_t, std::exception_ptr>> exceptions;

  for (auto i = std::ptrdiff_t(serviceInterceptors.size()) - 1; i >= 0; --i) {
    auto* connectionCtx = reqCtx_->getConnectionContext();
    auto connectionInfo = ServiceInterceptorBase::ConnectionInfo{
        connectionCtx,
        connectionCtx->getStorageForServiceInterceptorOnConnectionByIndex(i)};
    const server::DecoratorData decoratorData = reqCtx_->getDecoratorData();
    auto responseInfo = ServiceInterceptorBase::ResponseInfo{
        reqCtx_,
        reqCtx_->getStorageForServiceInterceptorOnRequestByIndex(i),
        resultOrActiveException,
        methodNameInfo_.serviceName,
        methodNameInfo_.definingServiceName,
        methodNameInfo_.methodName,
        methodNameInfo_.qualifiedMethodName,
        &decoratorData};
    try {
      co_await serviceInterceptors[i]->internal_onResponse(
          connectionInfo,
          std::move(responseInfo),
          server->getInterceptorMetricCallback());
    } catch (...) {
      exceptions.emplace_back(i, folly::current_exception());
    }
  }

  if (!exceptions.empty()) {
    std::string message = fmt::format(
        "ServiceInterceptor::onResponse threw exceptions:\n[{}] {}\n",
        serviceInterceptors[exceptions[0].first]->getQualifiedName().get(),
        folly::exceptionStr(exceptions[0].second));
    for (std::size_t i = 1; i < exceptions.size(); ++i) {
      message += fmt::format(
          "[{}] {}\n",
          serviceInterceptors[exceptions[i].first]->getQualifiedName().get(),
          folly::exceptionStr(exceptions[i].second));
    }
    co_yield folly::coro::co_error(TApplicationException(message));
  }
}
#endif // FOLLY_HAS_COROUTINES

namespace detail {

#if FOLLY_HAS_COROUTINES
bool shouldProcessServiceInterceptorsOnRequest(
    HandlerCallbackBase& callback) noexcept {
  return callback.shouldProcessServiceInterceptorsOnRequest();
}

folly::coro::Task<void> processServiceInterceptorsOnRequest(
    HandlerCallbackBase& callback,
    detail::ServiceInterceptorOnRequestArguments arguments,
    const SerializedRequest& serializedRequest) {
  try {
    co_await callback.processServiceInterceptorsOnRequest(
        std::move(arguments), serializedRequest);
  } catch (...) {
    callback.exception(folly::current_exception());
    throw;
  }
}
#endif // FOLLY_HAS_COROUTINES
} // namespace detail

} // namespace apache::thrift
