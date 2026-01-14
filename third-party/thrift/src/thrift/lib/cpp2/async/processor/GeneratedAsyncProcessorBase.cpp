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

#include <thrift/lib/cpp2/async/processor/GeneratedAsyncProcessorBase.h>
#include <thrift/lib/cpp2/async/processor/ServerRequestHelper.h>
#include <thrift/lib/cpp2/async/processor/ServerRequestTask.h>

namespace apache::thrift {

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
      FOLLY_PUSH_WARNING
      FOLLY_CLANG_DISABLE_WARNING("-Wunreachable-code")
      throw std::runtime_error("Nullptr returned from interaction constructor");
      FOLLY_POP_WARNING
    }
    return tile;
  };
  auto* reqCtx = req.requestContext();
  auto& conn = *reqCtx->getConnectionContext();
  bool isFactoryFunction = req.methodMetadata()->createsInteraction;
  auto interactionCreate = reqCtx->getInteractionCreate();
  auto isEbMethod =
      req.methodMetadata()->executorType == MethodMetadata::ExecutorType::EVB;
  auto id = reqCtx->getInteractionId();

  // In the eb model with old-style constructor we create the interaction
  // inline.
  if (isEbMethod && !isFactoryFunction) {
    auto tile = folly::makeTryWith([&] {
      return nullthrows(createInteractionImpl(
          std::move(*interactionCreate->interactionName()).str(),
          reqCtx->getHeader()->getProtocolId()));
    });
    if (tile.hasException()) {
      req.request()->sendErrorWrapped(
          folly::make_exception_wrapper<TApplicationException>(
              "Interaction constructor failed with " +
              tile.exception().what().toStdString()),
          kInteractionConstructorErrorErrorCode);
      return true; // Not a duplicate; caller will see missing tile.
    }
    tile.value()->setOverloadPolicy(
        InteractionOverloadPolicy::createFromThriftFlag());
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
        [this,
         &eb,
         &conn,
         nullthrows,
         interactionCreate,
         promisePtr,
         executor,
         id,
         reqCtx] {
          std::exception_ptr ex;
          try {
            auto tilePtr = nullthrows(createInteractionImpl(
                std::move(*interactionCreate->interactionName()).str(),
                reqCtx->getHeader()->getProtocolId()));
            eb.add([=, &conn, &eb, t = std::move(tilePtr)]() mutable {
              TilePtr tile{t.release(), &eb};
              promisePtr->fulfill(*tile, executor, eb);
              conn.tryReplaceTile(id, std::move(tile));
            });
            return;
          } catch (...) {
            ex = folly::current_exception();
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
      FOLLY_PUSH_WARNING
      FOLLY_CLANG_DISABLE_WARNING("-Wunreachable-code")
      throw std::runtime_error("Nullptr returned from interaction constructor");
      FOLLY_POP_WARNING
    }
    return tile;
  };
  auto& conn = *ctx.getConnectionContext();

  // In the eb model with old-style constructor we create the interaction
  // inline.
  if (!tm && !isFactoryFunction) {
    si->setEventBase(&eb);
    si->setRequestContext(&ctx);
    auto tile = folly::makeTryWith([&] {
      return nullthrows(
          createInteractionImpl(name, ctx.getHeader()->getProtocolId()));
    });
    if (tile.hasException()) {
      req->sendErrorWrapped(
          folly::make_exception_wrapper<TApplicationException>(
              "Interaction constructor failed with " +
              tile.exception().what().toStdString()),
          kInteractionConstructorErrorErrorCode);
      return true; // Not a duplicate; caller will see missing tile.
    }
    tile.value()->setOverloadPolicy(
        InteractionOverloadPolicy::createFromThriftFlag());
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
    tm->add([this,
             &eb,
             &ctx,
             name = std::move(name),
             &conn,
             si,
             tm,
             nullthrows,
             promisePtr,
             id] {
      si->setEventBase(&eb);
      si->setThreadManager(tm);
      si->setRequestContext(&ctx);

      std::exception_ptr ex;
      try {
        auto tilePtr = nullthrows(
            createInteractionImpl(name, ctx.getHeader()->getProtocolId()));
        eb.add([=, &conn, &eb, t = std::move(tilePtr)]() mutable {
          TilePtr tile{t.release(), &eb};
          promisePtr->fulfill(*tile, tm, eb);
          conn.tryReplaceTile(id, std::move(tile));
        });
        return;
      } catch (...) {
        ex = folly::current_exception();
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
    const std::string&, int16_t) {
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
    std::string_view serviceName,
    std::string_view method,
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
    auto ctxStack = getContextStackForNonPerRequestCallbacks(
        handlers_, getServiceName(), "#terminateInteraction", &conn);
    if (ctxStack) {
      tile->onDestroy([id, ctxStack = std::move(ctxStack)] {
        ctxStack->onInteractionTerminate(id);
      });
    }
    Tile::onTermination(std::move(tile), eb);
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
  bool isFactoryFunction = req.methodMetadata()->createsInteraction;
  bool interactionMetadataValid;
  if (auto interactionId = ctx->getInteractionId()) {
    if (auto interactionCreate = ctx->getInteractionCreate()) {
      if (!interactionName ||
          *interactionCreate->interactionName() !=
              std::string_view(interactionName.value())) {
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
      interactionMetadataValid =
          interactionName.has_value() && !isFactoryFunction;
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
      if (*interactionCreate->interactionName() != interaction) {
        interactionMetadataValid = false;
      } else if (!createInteraction(
                     req,
                     interactionId,
                     std::move(*interactionCreate->interactionName()).str(),
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
      interactionMetadataValid =
          !interaction.empty() && !isInteractionFactoryFunction;
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

} // namespace apache::thrift
