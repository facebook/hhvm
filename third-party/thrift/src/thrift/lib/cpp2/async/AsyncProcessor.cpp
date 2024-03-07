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

#include <thrift/lib/cpp2/async/AsyncProcessor.h>

#include <folly/io/async/EventBaseAtomicNotificationQueue.h>

#include <thrift/lib/cpp2/async/ReplyInfo.h>
#include <thrift/lib/cpp2/server/IResourcePoolAcceptor.h>

namespace apache {
namespace thrift {

thread_local RequestParams ServerInterface::requestParams_;

EventTask::~EventTask() {
  expired();
}

void EventTask::expired() {
  // only expire req_ once
  if (!req_.request()) {
    return;
  }
  failWith(
      TApplicationException{"Task expired without processing"},
      kTaskExpiredErrorCode);
}

void EventTask::failWith(folly::exception_wrapper ex, std::string exCode) {
  auto cleanUp = [oneway = oneway_,
                  req = apache::thrift::detail::ServerRequestHelper::request(
                      std::move(req_)),
                  ex = std::move(ex),
                  exCode = std::move(exCode)]() mutable {
    // if oneway, skip sending back anything
    if (oneway) {
      return;
    }
    req->sendErrorWrapped(std::move(ex), std::move(exCode));
  };

  auto eb = apache::thrift::detail::ServerRequestHelper::eventBase(req_);

  if (eb->inRunningEventBaseThread()) {
    cleanUp();
  } else {
    eb->runInEventBaseThread(std::move(cleanUp));
  }
}

void EventTask::setTile(TilePtr&& tile) {
  req_.requestContext()->setTile(std::move(tile));
}

ServerRequestTask::~ServerRequestTask() {
  // only expire req_ once
  if (!req_.request()) {
    return;
  }
  failWith(
      TApplicationException{"Task expired without processing"},
      kTaskExpiredErrorCode);
}

void ServerRequestTask::failWith(
    folly::exception_wrapper ex, std::string exCode) {
  auto cleanUp = [req = apache::thrift::detail::ServerRequestHelper::request(
                      std::move(req_)),
                  ex = std::move(ex),
                  exCode = std::move(exCode)]() mutable {
    req->sendErrorWrapped(std::move(ex), std::move(exCode));
  };

  auto eb = apache::thrift::detail::ServerRequestHelper::eventBase(req_);

  if (eb->inRunningEventBaseThread()) {
    cleanUp();
  } else {
    eb->runInEventBaseThread(std::move(cleanUp));
  }
}

void ServerRequestTask::setTile(TilePtr&& tile) {
  req_.requestContext()->setTile(std::move(tile));
}

void ServerRequestTask::acceptIntoResourcePool(int8_t priority) {
  detail::ServerRequestHelper::setInternalPriority(req_, priority);
  detail::ServerRequestHelper::resourcePool(req_)->accept(std::move(req_));
}

const char* AsyncProcessor::getServiceName() {
  return "NoServiceNameSet";
}

void AsyncProcessor::terminateInteraction(
    int64_t, Cpp2ConnContext&, folly::EventBase&) noexcept {
  LOG(DFATAL) << "This processor doesn't support interactions";
}

void AsyncProcessor::destroyAllInteractions(
    Cpp2ConnContext&, folly::EventBase&) noexcept {}

void AsyncProcessor::executeRequest(
    ServerRequest&&, const AsyncProcessorFactory::MethodMetadata&) {
  LOG(FATAL) << "Unimplemented executeRequest called";
}

void AsyncProcessor::coalesceWithServerScopedLegacyEventHandlers(
    const apache::thrift::server::ServerConfigs& server) {
  const auto& serverScopedEventHandlers = server.getLegacyEventHandlers();
  if (!serverScopedEventHandlers.empty()) {
    std::shared_lock lock{getRWMutex()};
    for (const auto& eventHandler : serverScopedEventHandlers) {
      addEventHandler(eventHandler);
    }
  }
}

void GeneratedAsyncProcessorBase::processInteraction(ServerRequest&& req) {
  if (!setUpRequestProcessing(req)) {
    return;
  }
  auto ctx = req.requestContext();

  Tile* tile = nullptr;
  if (auto interactionId = ctx->getInteractionId()) { // includes create
    try {
      tile = &ctx->getConnectionContext()->getTile(interactionId);
    } catch (const std::out_of_range&) {
      req.request()->sendErrorWrapped(
          TApplicationException(
              "Invalid interaction id " + std::to_string(interactionId)),
          kInteractionIdUnknownErrorCode);
      return;
    }
  }

  auto scope = ctx->getRequestExecutionScope();
  auto task = std::make_unique<ServerRequestTask>(std::move(req));
  if (tile) {
    task->setTile(
        {tile,
         ctx->getConnectionContext()
             ->getWorkerContext()
             ->getWorkerEventBase()});
  }
  auto& request = task->req_;
  std::unique_ptr<concurrency::Runnable> runnableTask = std::move(task);

  if (tile && tile->maybeEnqueue(std::move(runnableTask), scope)) {
    return;
  }

  // If the task was not enqueued, accessing `request` is still valid
  auto source = tile && !ctx->getInteractionCreate() ? folly::Executor::MID_PRI
                                                     : folly::Executor::LO_PRI;

  apache::thrift::detail::ServerRequestHelper::setInternalPriority(
      request, source);

  auto&& resourcePool =
      apache::thrift::detail::ServerRequestHelper::resourcePool(request);
  resourcePool->accept(std::move(request));
}

bool GeneratedAsyncProcessorBase::createInteraction(ServerRequest& req) {
  auto& eb = *apache::thrift::detail::ServerRequestHelper::eventBase(req);
  eb.dcheckIsInEventBaseThread();

  auto nullthrows = [](std::unique_ptr<Tile> tile) {
    if (!tile) {
      DLOG(FATAL) << "Nullptr returned from interaction constructor";
      throw std::runtime_error("Nullptr returned from interaction constructor");
    }
    return tile;
  };
  auto& conn = *req.requestContext()->getConnectionContext();
  bool isFactoryFunction = req.methodMetadata()->createsInteraction;
  auto interactionCreate = req.requestContext()->getInteractionCreate();
  auto isEbMethod =
      req.methodMetadata()->executorType == MethodMetadata::ExecutorType::EVB;
  auto id = req.requestContext()->getInteractionId();

  // In the eb model with old-style constructor we create the interaction
  // inline.
  if (isEbMethod && !isFactoryFunction) {
    auto tile = folly::makeTryWith([&] {
      return nullthrows(createInteractionImpl(
          std::move(*interactionCreate->interactionName_ref()).str()));
    });
    if (tile.hasException()) {
      req.request()->sendErrorWrapped(
          folly::make_exception_wrapper<TApplicationException>(
              "Interaction constructor failed with " +
              tile.exception().what().toStdString()),
          kInteractionConstructorErrorErrorCode);
      return true; // Not a duplicate; caller will see missing tile.
    }
    return conn.addTile(id, {tile->release(), &eb});
  }

  // Otherwise we use a promise.
  auto promisePtr =
      new TilePromise(isFactoryFunction); // freed by RefGuard on next line
  if (!conn.addTile(id, {promisePtr, &eb})) {
    return false;
  }

  auto executor = apache::thrift::detail::ServerRequestHelper::executor(req);

  // Old-style constructor + tm : schedule constructor and return
  if (!isFactoryFunction) {
    apache::thrift::detail::ServerRequestHelper::executor(req)->add(
        [=, &eb, &conn] {
          std::exception_ptr ex;
          try {
            auto tilePtr = nullthrows(createInteractionImpl(
                std::move(*interactionCreate->interactionName_ref()).str()));
            eb.add([=, &conn, &eb, t = std::move(tilePtr)]() mutable {
              TilePtr tile{t.release(), &eb};
              promisePtr->fulfill(*tile, executor, eb);
              conn.tryReplaceTile(id, std::move(tile));
            });
            return;
          } catch (...) {
            ex = std::current_exception();
          }
          DCHECK(ex);
          eb.add([promisePtr, ex = std::move(ex)]() {
            promisePtr->failWith(
                folly::make_exception_wrapper<TApplicationException>(
                    folly::to<std::string>(
                        "Interaction constructor failed with ",
                        folly::exceptionStr(ex))),
                kInteractionConstructorErrorErrorCode);
          });
        });
    return true;
  }
  // Factory function: the handler method will fulfill the promise
  return true;
}

bool GeneratedAsyncProcessorBase::createInteraction(
    const ResponseChannelRequest::UniquePtr& req,
    int64_t id,
    std::string&& name,
    Cpp2RequestContext& ctx,
    concurrency::ThreadManager* tm,
    folly::EventBase& eb,
    ServerInterface* si,
    bool isFactoryFunction) {
  eb.dcheckIsInEventBaseThread();

  auto nullthrows = [](std::unique_ptr<Tile> tile) {
    if (!tile) {
      DLOG(FATAL) << "Nullptr returned from interaction constructor";
      throw std::runtime_error("Nullptr returned from interaction constructor");
    }
    return tile;
  };
  auto& conn = *ctx.getConnectionContext();

  // In the eb model with old-style constructor we create the interaction
  // inline.
  if (!tm && !isFactoryFunction) {
    si->setEventBase(&eb);
    si->setRequestContext(&ctx);
    auto tile = folly::makeTryWith(
        [&] { return nullthrows(createInteractionImpl(name)); });
    if (tile.hasException()) {
      req->sendErrorWrapped(
          folly::make_exception_wrapper<TApplicationException>(
              "Interaction constructor failed with " +
              tile.exception().what().toStdString()),
          kInteractionConstructorErrorErrorCode);
      return true; // Not a duplicate; caller will see missing tile.
    }
    return conn.addTile(id, {tile->release(), &eb});
  }

  // Otherwise we use a promise.
  auto promisePtr =
      new TilePromise(isFactoryFunction); // freed by RefGuard on next line
  if (!conn.addTile(id, {promisePtr, &eb})) {
    return false;
  }

  // Old-style constructor + tm : schedule constructor and return
  if (!isFactoryFunction) {
    tm->add([=, &eb, &ctx, name = std::move(name), &conn] {
      si->setEventBase(&eb);
      si->setThreadManager(tm);
      si->setRequestContext(&ctx);

      std::exception_ptr ex;
      try {
        auto tilePtr = nullthrows(createInteractionImpl(name));
        eb.add([=, &conn, &eb, t = std::move(tilePtr)]() mutable {
          TilePtr tile{t.release(), &eb};
          promisePtr->fulfill(*tile, tm, eb);
          conn.tryReplaceTile(id, std::move(tile));
        });
        return;
      } catch (...) {
        ex = std::current_exception();
      }
      DCHECK(ex);
      eb.add([promisePtr, ex = std::move(ex)]() {
        promisePtr->failWith(
            folly::make_exception_wrapper<TApplicationException>(
                folly::to<std::string>(
                    "Interaction constructor failed with ",
                    folly::exceptionStr(ex))),
            kInteractionConstructorErrorErrorCode);
      });
    });
    return true;
  }

  // Factory function: the handler method will fulfill the promise
  return true;
}

std::unique_ptr<Tile> GeneratedAsyncProcessorBase::createInteractionImpl(
    const std::string&) {
  return nullptr;
}

namespace {
/**
 * Call this version to only invoke handlers that explicitly okays
 * callbacks from non-per-request contexts.
 * @see TProcessorEventHandler::wantNonPerRequestCallbacks
 */
ContextStack::UniquePtr getContextStackForNonPerRequestCallbacks(
    const std::shared_ptr<std::vector<std::shared_ptr<TProcessorEventHandler>>>&
        allHandlers,
    const char* serviceName,
    const char* method,
    TConnectionContext* connectionContext) {
  if (!allHandlers || allHandlers->empty()) {
    return nullptr;
  }
  auto handlersForNonPerRequestCallbacks =
      std::make_shared<std::vector<std::shared_ptr<TProcessorEventHandler>>>();
  std::copy_if(
      allHandlers->begin(),
      allHandlers->end(),
      std::back_inserter(*handlersForNonPerRequestCallbacks),
      [](const auto& handler) {
        return handler->wantNonPerRequestCallbacks();
      });
  return ContextStack::create(
      handlersForNonPerRequestCallbacks,
      serviceName,
      method,
      connectionContext);
}
} // namespace

void GeneratedAsyncProcessorBase::terminateInteraction(
    int64_t id, Cpp2ConnContext& conn, folly::EventBase& eb) noexcept {
  eb.dcheckIsInEventBaseThread();

  if (auto tile = conn.removeTile(id)) {
    Tile::onTermination(std::move(tile), eb);
    auto ctxStack = getContextStackForNonPerRequestCallbacks(
        handlers_, getServiceName(), "#terminateInteraction", &conn);
    if (ctxStack) {
      ctxStack->onInteractionTerminate(id);
    }
  }
}

void GeneratedAsyncProcessorBase::destroyAllInteractions(
    Cpp2ConnContext& conn, folly::EventBase& eb) noexcept {
  eb.dcheckIsInEventBaseThread();

  if (conn.tiles_.empty()) {
    return;
  }

  std::vector<int64_t> ids;
  ids.reserve(conn.tiles_.size());
  for (auto& [id, tile] : conn.tiles_) {
    ids.push_back(id);
  }
  auto ctxStack = getContextStackForNonPerRequestCallbacks(
      handlers_, getServiceName(), "#terminateInteraction", &conn);
  for (auto id : ids) {
    if (conn.removeTile(id) && ctxStack) {
      ctxStack->onInteractionTerminate(id);
    }
  }
}

bool GeneratedAsyncProcessorBase::validateRpcKind(
    const ResponseChannelRequest::UniquePtr& req, RpcKind kind) {
  switch (kind) {
    case RpcKind::SINGLE_REQUEST_NO_RESPONSE:
      switch (req->rpcKind()) {
        case RpcKind::SINGLE_REQUEST_NO_RESPONSE:
          return true;
        case RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE:
          req->sendReply(ResponsePayload{});
          return true;
        default:
          break;
      }
      break;
    case RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE:
      switch (req->rpcKind()) {
        case RpcKind::SINGLE_REQUEST_NO_RESPONSE:
        case RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE:
          return true;
        default:
          break;
      }
      break;
    default:
      if (kind == req->rpcKind()) {
        return true;
      }
  }
  if (req->rpcKind() != RpcKind::SINGLE_REQUEST_NO_RESPONSE) {
    req->sendErrorWrapped(
        folly::make_exception_wrapper<TApplicationException>(
            TApplicationException::TApplicationExceptionType::UNKNOWN_METHOD,
            "Function kind mismatch"),
        kRequestTypeDoesntMatchServiceFunctionType);
  }
  return false;
}

bool GeneratedAsyncProcessorBase::validateRpcKind(const ServerRequest& req) {
  DCHECK(req.methodMetadata()->rpcKind);
  return validateRpcKind(req.request(), *req.methodMetadata()->rpcKind);
}

bool GeneratedAsyncProcessorBase::setUpRequestProcessing(ServerRequest& req) {
  if (!validateRpcKind(req)) {
    return false;
  }

  auto ctx = req.requestContext();
  auto eb = apache::thrift::detail::ServerRequestHelper::eventBase(req);
  auto isEbMethod =
      req.methodMetadata()->executorType == MethodMetadata::ExecutorType::EVB;
  auto& interactionName = req.methodMetadata()->interactionName;
  bool interactionMetadataValid;
  if (auto interactionId = ctx->getInteractionId()) {
    if (auto interactionCreate = ctx->getInteractionCreate()) {
      if (!interactionName ||
          *interactionCreate->interactionName_ref() !=
              interactionName.value()) {
        interactionMetadataValid = false;
      } else if (!createInteraction(req)) {
        // Duplicate id is a contract violation so close the connection.
        // Terminate this interaction first so queued requests can't use it
        // (which could result in UB).
        terminateInteraction(interactionId, *ctx->getConnectionContext(), *eb);
        req.request()->sendErrorWrapped(
            TApplicationException(
                "Attempting to create interaction with duplicate id. Failing all requests in that interaction."),
            kConnectionClosingErrorCode);
        return false;
      } else {
        interactionMetadataValid = true;
      }
    } else {
      interactionMetadataValid = interactionName.has_value();
    }

    if (interactionMetadataValid && isEbMethod) {
      try {
        // This is otherwise done while constructing InteractionEventTask.
        auto& tile = ctx->getConnectionContext()->getTile(interactionId);
        ctx->setTile({&tile, eb});
      } catch (const std::out_of_range&) {
        req.request()->sendErrorWrapped(
            TApplicationException(
                "Invalid interaction id " + std::to_string(interactionId)),
            kInteractionIdUnknownErrorCode);
        return false;
      }
    }
  } else {
    interactionMetadataValid = !interactionName.has_value();
  }
  if (!interactionMetadataValid) {
    req.request()->sendErrorWrapped(
        folly::make_exception_wrapper<TApplicationException>(
            TApplicationException::TApplicationExceptionType::UNKNOWN_METHOD,
            "Interaction and method do not match"),
        kMethodUnknownErrorCode);
    return false;
  }

  return true;
}

bool GeneratedAsyncProcessorBase::setUpRequestProcessing(
    const ResponseChannelRequest::UniquePtr& req,
    Cpp2RequestContext* ctx,
    folly::EventBase* eb,
    concurrency::ThreadManager* tm,
    RpcKind kind,
    ServerInterface* si,
    folly::StringPiece interaction,
    bool isInteractionFactoryFunction) {
  if (!validateRpcKind(req, kind)) {
    return false;
  }

  bool interactionMetadataValid;
  if (auto interactionId = ctx->getInteractionId()) {
    if (auto interactionCreate = ctx->getInteractionCreate()) {
      if (*interactionCreate->interactionName_ref() != interaction) {
        interactionMetadataValid = false;
      } else if (!createInteraction(
                     req,
                     interactionId,
                     std::move(*interactionCreate->interactionName_ref()).str(),
                     *ctx,
                     tm,
                     *eb,
                     si,
                     isInteractionFactoryFunction)) {
        // Duplicate id is a contract violation so close the connection.
        // Terminate this interaction first so queued requests can't use it
        // (which could result in UB).
        terminateInteraction(interactionId, *ctx->getConnectionContext(), *eb);
        req->sendErrorWrapped(
            TApplicationException(
                "Attempting to create interaction with duplicate id. Failing all requests in that interaction."),
            kConnectionClosingErrorCode);
        return false;
      } else {
        interactionMetadataValid = true;
      }
    } else {
      interactionMetadataValid = !interaction.empty();
    }

    if (interactionMetadataValid && !tm) {
      try {
        // This is otherwise done while constructing InteractionEventTask.
        auto& tile = ctx->getConnectionContext()->getTile(interactionId);
        ctx->setTile({&tile, eb});
      } catch (const std::out_of_range&) {
        req->sendErrorWrapped(
            TApplicationException(
                "Invalid interaction id " + std::to_string(interactionId)),
            kInteractionIdUnknownErrorCode);
        return false;
      }
    }
  } else {
    interactionMetadataValid = interaction.empty();
  }
  if (!interactionMetadataValid) {
    req->sendErrorWrapped(
        folly::make_exception_wrapper<TApplicationException>(
            TApplicationException::TApplicationExceptionType::UNKNOWN_METHOD,
            "Interaction and method do not match"),
        kMethodUnknownErrorCode);
    return false;
  }

  return true;
}

concurrency::PRIORITY ServerInterface::getRequestPriority(
    Cpp2RequestContext* ctx, concurrency::PRIORITY prio) {
  concurrency::PRIORITY callPriority = ctx->getCallPriority();
  return callPriority == concurrency::N_PRIORITIES ? prio : callPriority;
}

void ServerInterface::setEventBase(folly::EventBase* eb) {
  folly::RequestEventBase::set(eb);
  requestParams_.eventBase_ = eb;
}

void ServerInterface::BlockingThreadManager::add(folly::Func f) {
  try {
    if (threadManagerKa_) {
      std::shared_ptr<concurrency::Runnable> task =
          concurrency::FunctionRunner::create(std::move(f));
      threadManagerKa_->add(
          std::move(task),
          std::chrono::milliseconds(kTimeout).count() /* deprecated */,
          0,
          false);
    } else {
      executorKa_->add(std::move(f));
    }
    return;
  } catch (...) {
    LOG(FATAL) << "Failed to schedule a task within timeout: "
               << folly::exceptionStr(std::current_exception());
  }
}

bool ServerInterface::BlockingThreadManager::keepAliveAcquire() noexcept {
  auto keepAliveCount = keepAliveCount_.fetch_add(1, std::memory_order_relaxed);
  // We should never increment from 0
  DCHECK(keepAliveCount > 0);
  return true;
}

void ServerInterface::BlockingThreadManager::keepAliveRelease() noexcept {
  auto keepAliveCount = keepAliveCount_.fetch_sub(1, std::memory_order_acq_rel);
  DCHECK(keepAliveCount >= 1);
  if (keepAliveCount == 1) {
    delete this;
  }
}

folly::Executor::KeepAlive<> ServerInterface::getInternalKeepAlive() {
  if (getThreadManager_deprecated()) {
    return getThreadManager_deprecated()->getKeepAlive(
        getRequestContext()->getRequestExecutionScope(),
        apache::thrift::concurrency::ThreadManager::Source::INTERNAL);
  } else {
    return folly::Executor::getKeepAliveToken(getHandlerExecutor());
  }
}

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
      exception(TApplicationException(
          TApplicationException::INTERNAL_ERROR,
          "apache::thrift::HandlerCallback not completed"));
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
        apache::thrift::detail::SinkConsumerImpl>&& responseAndSinkConsumer) {
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
  auto& sinkConsumer = responseAndSinkConsumer.second;
  sinkConsumer.interaction = std::move(interaction_);

  if (getEventBase()->isInEventBaseThread()) {
    SinkConsumerReplyInfo(
        std::move(req_), std::move(sinkConsumer), std::move(payload), crc32c)();
  } else {
    putMessageInReplyQueue(
        std::in_place_type_t<SinkConsumerReplyInfo>(),
        std::move(req_),
        std::move(sinkConsumer),
        std::move(payload),
        crc32c);
  }
#else
  std::terminate();
#endif
}

bool HandlerCallbackBase::fulfillTilePromise(std::unique_ptr<Tile> ptr) {
  if (!ptr) {
    DLOG(FATAL) << "Nullptr interaction yielded from handler";
    exception(TApplicationException(
        TApplicationException::MISSING_RESULT,
        "Nullptr interaction yielded from handler"));
    return false;
  }

  auto fn = [connCtx = reqCtx_->getConnectionContext(),
             interactionId = reqCtx_->getInteractionId(),
             interaction = std::move(interaction_),
             ptr = std::move(ptr),
             tm = getThreadManager_deprecated(),
             eb = eb_,
             executor = executor_,
             isRPEnabled = isResourcePoolEnabled()]() mutable {
    TilePtr tile{ptr.release(), eb};
    DCHECK(dynamic_cast<TilePromise*>(interaction.get()));
    if (isRPEnabled) {
      static_cast<TilePromise&>(*interaction).fulfill(*tile, executor, *eb);
    } else {
      static_cast<TilePromise&>(*interaction).fulfill(*tile, tm, *eb);
    }
    connCtx->tryReplaceTile(interactionId, std::move(tile));
  };

  eb_->runImmediatelyOrRunInEventBaseThread(std::move(fn));
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

HandlerCallback<void>::HandlerCallback(
    ResponseChannelRequest::UniquePtr req,
    ContextStack::UniquePtr ctx,
    cob_ptr cp,
    exnw_ptr ewp,
    int32_t protoSeqId,
    folly::EventBase* eb,
    concurrency::ThreadManager* tm,
    Cpp2RequestContext* reqCtx,
    TilePtr&& interaction)
    : HandlerCallbackBase(
          std::move(req),
          std::move(ctx),
          ewp,
          eb,
          tm,
          reqCtx,
          std::move(interaction)),
      cp_(cp) {
  this->protoSeqId_ = protoSeqId;
}

HandlerCallback<void>::HandlerCallback(
    ResponseChannelRequest::UniquePtr req,
    ContextStack::UniquePtr ctx,
    cob_ptr cp,
    exnw_ptr ewp,
    int32_t protoSeqId,
    folly::EventBase* eb,
    folly::Executor::KeepAlive<> executor,
    Cpp2RequestContext* reqCtx,
    RequestCompletionCallback* notifyRequestPile,
    RequestCompletionCallback* notifyConcurrencyController,
    ServerRequestData requestData,
    TilePtr&& interaction)
    : HandlerCallbackBase(
          std::move(req),
          std::move(ctx),
          ewp,
          eb,
          std::move(executor),
          reqCtx,
          notifyRequestPile,
          notifyConcurrencyController,
          std::move(requestData),
          std::move(interaction)),
      cp_(cp) {
  this->protoSeqId_ = protoSeqId;
}

void HandlerCallback<void>::complete(folly::Try<folly::Unit>&& r) {
  maybeNotifyComplete();
  if (r.hasException()) {
    exception(std::move(r.exception()));
  } else {
    done();
  }
}

void HandlerCallback<void>::doDone() {
  assert(cp_ != nullptr);
  auto queue = cp_(this->ctx_.get());
  this->ctx_.reset();
  sendReply(std::move(queue));
}

} // namespace thrift
} // namespace apache
