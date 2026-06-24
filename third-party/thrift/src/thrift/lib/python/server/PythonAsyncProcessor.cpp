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

#include <memory>

#include <folly/ScopeGuard.h>
#include <folly/python/error.h>
#include <folly/python/import.h>
#include <thrift/lib/cpp/TApplicationException.h>
#include <thrift/lib/cpp2/server/Cpp2ConnContext.h>
#include <thrift/lib/python/server/PythonAsyncProcessor.h>
#include <thrift/lib/python/server/python_async_processor_api.h> // @manual

namespace apache::thrift::python {

using apache::thrift::detail::processServiceInterceptorsOnRequest;
using apache::thrift::detail::ServiceInterceptorOnRequestArguments;
using apache::thrift::detail::shouldProcessServiceInterceptorsOnRequest;

using InteractionType =
    apache::thrift::AsyncProcessorFactory::MethodMetadata::InteractionType;

namespace {

void do_python_import() {
  static ::folly::python::import_cache_nocapture import(
      (::import_thrift__python__server_impl__python_async_processor));
  if (!import()) {
    // converts python error to thrown std::runtime_error
    ::folly::python::handlePythonError(
        "import thrift.python.server_impl.python_async_processor failed");
  }
}

auto get_deserialize_error_function(apache::thrift::ProtocolType protocol) {
  return protocol == apache::thrift::protocol::PROTOCOL_TYPES::T_BINARY_PROTOCOL
      ? apache::thrift::detail::ap::process_handle_exn_deserialization<
            apache::thrift::BinaryProtocolWriter>
      : apache::thrift::detail::ap::process_handle_exn_deserialization<
            apache::thrift::CompactProtocolWriter>;
}

// C++ ServiceInterceptors (i.e., those installed via cpp Service Framework)
// are invoked without serializing arguments.
ServiceInterceptorOnRequestArguments emptyInterceptorsArguments() {
  static std::tuple empty = std::make_tuple();
  return ServiceInterceptorOnRequestArguments(empty);
}

} // namespace

HandlerFunc makeHandlerFunc(
    apache::thrift::RpcKind kind,
    PyObject* funcObject,
    const std::string& serviceName,
    std::string_view functionName) {
  return HandlerFunc{
      kind,
      funcObject,
      fmt::format("{}.{}", serviceName, functionName),
      /*interactionName=*/{},
      InteractionType::NONE,
      /*createsInteraction=*/false,
      /*returnsInitialResponse=*/false,
      /*factoryObject=*/nullptr,
  };
}

HandlerFunc makeInteractionHandlerFunc(
    apache::thrift::RpcKind kind,
    PyObject* funcObject,
    const std::string& serviceName,
    std::string_view functionName,
    std::string_view interactionName,
    bool createsInteraction,
    PyObject* factoryObject,
    bool returnsInitialResponse) {
  return HandlerFunc{
      kind,
      funcObject,
      fmt::format("{}.{}", serviceName, functionName),
      interactionName,
      // The factory method creates the interaction; an inside-interaction
      // method (createsInteraction == false) routes via the Tile.
      createsInteraction ? InteractionType::NONE
                         : InteractionType::INTERACTION_V1,
      createsInteraction,
      returnsInitialResponse,
      factoryObject,
  };
}

PythonTile::PythonTile(PyObject* handler, folly::Executor::KeepAlive<> executor)
    : handler_(handler), executor_(std::move(executor)) {
  // Precondition: the caller holds the GIL. Every construction site does --
  // createInteractionImpl via PyGILState_Ensure(), the fulfill/install paths on
  // the Python executor (loop thread).
  Py_XINCREF(handler_);
}

PythonTile::~PythonTile() {
  if (handler_ == nullptr) {
    return;
  }
  // Runs on a C++ teardown thread that can't safely touch the GIL (the
  // interpreter may be finalizing), so hand the handler ref to the Python
  // executor (loop thread, GIL held; the KeepAlive keeps it alive for the
  // task).
  //
  // If teardown happened without an explicit client terminate (e.g. the
  // connection dropped first), `co_onTermination` never ran, so fire the
  // `onInteractionTermination` hook now, best-effort. Claim the hook here
  // (synchronously) so the single loop-thread task runs it plus the mandatory
  // decref. If the loop is already gone, that task and ref drop like a plain
  // decref.
  const bool fireHook = !hookFired_.exchange(true);
  executor_->add([handler = handler_, fireHook] {
    // loop thread, GIL held
    if (fireHook) {
      scheduleInteractionTermination(handler);
    }
    Py_DECREF(handler);
  });
}

#if FOLLY_HAS_COROUTINES
folly::coro::Task<void> PythonTile::co_onTermination() {
  // Explicit client terminate. Claim the hook so the destructor's
  // connection-close path won't run it again. The framework holds a TilePtr
  // ref across this coroutine, so `handler_` stays alive until we return.
  hookFired_.store(true);
  auto [promise, future] = folly::makePromiseContract<folly::Unit>();
  // Schedule the Python `onInteractionTermination` coro on the asyncio loop
  // (GIL held there) and resume once it completes, so the hook finishes before
  // teardown.
  executor_->add([handler = handler_, promise = std::move(promise)]() mutable {
    handleInteractionTermination(handler, std::move(promise));
  });
  // Await the Try and ignore it: if the fulfilling task is dropped (e.g. a hard
  // interpreter teardown racing this terminate) the future fails -- resume and
  // let teardown proceed rather than leak. The hook is best-effort once we are
  // racing shutdown.
  co_await folly::coro::co_awaitTry(std::move(future));
}
#endif

PyObject* FOLLY_NULLABLE
PythonAsyncProcessor::findInteractionFactory(std::string_view name) {
  auto it = interactionFactories_.find(name);
  return it != interactionFactories_.end() ? it->second : nullptr;
}

std::unique_ptr<apache::thrift::Tile>
PythonAsyncProcessor::createInteractionImpl(
    const std::string& name, int16_t /*protocol*/) {
  do_python_import();
  PyObject* factory = findInteractionFactory(name);
  if (factory == nullptr) {
    return nullptr;
  }
  // PyGILState_Ensure() is safe against finalization here: this override only
  // runs while a request is being processed, so Python `serve()` is still
  // parked on `ThriftServer::serve()` (interpreter up). `serve()` joins all
  // request threads before returning, so none can be here once Python can
  // finalize. (Contrast ~PythonTile, which can run during teardown and so must
  // not.)
  PyGILState_STATE g = PyGILState_Ensure();
  SCOPE_EXIT {
    PyGILState_Release(g);
  };
  PyObject* instance = callInteractionFactory(factory);
  // PythonTile takes its own ref; drop the +1 from callInteractionFactory even
  // if tile construction throws.
  SCOPE_EXIT {
    Py_XDECREF(instance);
  };
  if (instance == nullptr) {
    // The Python factory raised; it propagates per the C-API convention
    // (nullptr + error set). Surface it as a C++ exception -- the framework
    // reports it to the client as an interaction-constructor error.
    folly::python::handlePythonError("interaction factory failed: ");
  }
  std::unique_ptr<PythonTile> tile;
  if (instance != Py_None) {
    tile = std::make_unique<PythonTile>(instance, executor_);
  }
  return tile;
}

folly::exception_wrapper PythonAsyncProcessor::maybeFulfillTilePromise(
    const HandlerFunc& function,
    apache::thrift::Cpp2RequestContext* context,
    folly::EventBase* eb) {
  auto interactionId = context->getInteractionId();
  auto* connCtx = context->getConnectionContext();
  if (interactionId == 0 || connCtx == nullptr ||
      function.interactionName.empty() || eb == nullptr) {
    return {};
  }
  // Don't peek at the tile map here: `tiles_` is EventBase-thread-only, but
  // this runs on the Python executor thread -- a racing `findTile` is a
  // use-after-free against `createInteraction`'s rehash. The parked-promise
  // check happens inside the runInEventBaseThread lambda below instead.
  PyObject* factory = findInteractionFactory(function.interactionName);
  if (factory == nullptr) {
    return {};
  }
  // Runs on the Python executor (loop thread, GIL held): executeRequest hops
  // dispatch onto executor_ via `.via(executor_)` before this, so calling the
  // Python factory needs no manual GIL acquire or thread hop.
  std::unique_ptr<PythonTile> pythonTile;
  PyObject* instance = callInteractionFactory(factory);
  // Drop the +1 from callInteractionFactory even if construction below throws.
  SCOPE_EXIT {
    Py_XDECREF(instance);
  };
  if (instance == nullptr) {
    // The Python factory raised (C-API convention: nullptr + error set). Return
    // it to the caller so the factory request fails with the real message; we
    // can't throw here without stranding that request's HandlerCallback.
    try {
      folly::python::handlePythonError("interaction factory failed: ");
    } catch (...) {
      return folly::exception_wrapper(std::current_exception());
    }
  }
  if (instance != Py_None) {
    pythonTile = std::make_unique<PythonTile>(instance, executor_);
  }
  if (pythonTile == nullptr) {
    return {};
  }
  // Re-resolve the TilePromise inside the EventBase thread to avoid a
  // use-after-free if a concurrent caller already fulfilled and replaced the
  // promise. All tile-map mutations happen on the EventBase thread, so the
  // re-lookup + fulfill + replace is atomic with respect to other tile ops.
  auto executor = executor_;
  eb->runInEventBaseThread([eb,
                            executor,
                            tile = pythonTile.release(),
                            interactionId,
                            connCtx]() mutable {
    apache::thrift::TilePtr tilePtr{tile, eb};
    apache::thrift::detail::Cpp2ConnContextInternalAPI api(*connCtx);
    if (auto* tpromise = dynamic_cast<apache::thrift::TilePromise*>(
            api.findTile(interactionId))) {
      tpromise->fulfill(*tilePtr, executor, *eb);
      connCtx->tryReplaceTile(interactionId, std::move(tilePtr));
    }
    // else: someone already fulfilled; let our PythonTile drop on scope exit.
  });
  return {};
}

void installInteractionTileFromHandler(
    apache::thrift::Cpp2RequestContext* context,
    PyObject* instance,
    folly::Executor* executor) {
  // `instance` is borrowed (the generated handler wrapper still holds it); the
  // PythonTile below takes its own ref via Py_XINCREF.
  if (instance == nullptr || instance == Py_None || context == nullptr) {
    return;
  }
  auto interactionId = context->getInteractionId();
  auto* connCtx = context->getConnectionContext();
  if (interactionId == 0 || connCtx == nullptr) {
    return;
  }
  // The connection's EventBase owns the tile map; derive it from the transport.
  const auto* transport = connCtx->getTransport();
  folly::EventBase* eb = transport ? transport->getEventBase() : nullptr;
  if (eb == nullptr) {
    return;
  }
  // Runs on the Python executor (loop thread, GIL held), so constructing the
  // PythonTile is safe without a manual GIL acquire -- as in
  // maybeFulfillTilePromise.
  auto keepAlive = folly::getKeepAliveToken(executor);
  auto pythonTile = std::make_unique<PythonTile>(instance, keepAlive);
  // Re-resolve + fulfill on the EventBase thread for atomicity, as in
  // maybeFulfillTilePromise. Raw `connCtx` capture is safe: the outstanding
  // factory request keeps the Cpp2Connection (owner of the Cpp2ConnContext)
  // alive, and connection teardown is FIFO-serialized on this same EventBase,
  // so this task runs first.
  eb->runInEventBaseThread([eb,
                            keepAlive = std::move(keepAlive),
                            tile = pythonTile.release(),
                            interactionId,
                            connCtx]() mutable {
    apache::thrift::TilePtr tilePtr{tile, eb};
    apache::thrift::detail::Cpp2ConnContextInternalAPI api(*connCtx);
    if (auto* tpromise = dynamic_cast<apache::thrift::TilePromise*>(
            api.findTile(interactionId))) {
      tpromise->fulfill(*tilePtr, keepAlive, *eb);
      connCtx->tryReplaceTile(interactionId, std::move(tilePtr));
    }
    // else: someone already fulfilled; let our PythonTile drop on scope exit.
  });
}

PyObject* FOLLY_NULLABLE PythonAsyncProcessor::getInteractionHandler(
    const HandlerFunc& function,
    apache::thrift::Cpp2RequestContext* context,
    folly::EventBase* eb) {
  if (function.interactionType != InteractionType::INTERACTION_V1) {
    return nullptr;
  }
  auto interactionId = context->getInteractionId();
  auto* connCtx = context->getConnectionContext();
  if (interactionId == 0 || connCtx == nullptr) {
    return nullptr;
  }
  // Fast path: the request's own tile is request-thread-safe and avoids the
  // shared map. It's also preferred for correctness -- `fulfill()` sets the
  // request tile before `tryReplaceTile` updates the conn map, so the conn map
  // can briefly still hold the TilePromise.
  if (auto reqTile = context->releaseTile()) {
    auto* tile = reqTile.get();
    context->setTile(std::move(reqTile));
    auto* pyTile = dynamic_cast<PythonTile*>(tile);
    return pyTile ? pyTile->handler() : nullptr;
  }
  // Fallback: the tile lives only in the shared map, which is EventBase-only --
  // reading it here (Python executor thread) would race `createInteraction`'s
  // rehash. Resolve on the EventBase thread; release the GIL across the wait so
  // EventBase work that needs it can't deadlock.
  if (eb == nullptr) {
    return nullptr;
  }
  PyObject* handler = nullptr;
  // PyEval_SaveThread is safe here: this runs on executor_ (GIL held) during
  // request dispatch, and an in-flight request keeps serve() parked so Python
  // can't be finalizing -- same invariant as createInteractionImpl above.
  PyThreadState* saved = PyEval_SaveThread();
  // RAII so the GIL is reacquired even if the wait throws.
  SCOPE_EXIT {
    PyEval_RestoreThread(saved);
  };
  eb->runImmediatelyOrRunInEventBaseThreadAndWait(
      [connCtx, interactionId, context, eb, &handler]() {
        apache::thrift::detail::Cpp2ConnContextInternalAPI api(*connCtx);
        auto* pyTile = dynamic_cast<PythonTile*>(api.findTile(interactionId));
        if (pyTile == nullptr) {
          return;
        }
        // Anchor the tile to this request (acquires a ref on the EventBase
        // thread, where tile refcounting is safe) so neither it nor its handler
        // PyObject can be destroyed while dispatch is still in flight on the
        // executor thread -- the request now owns the tile, matching the fast
        // path above.
        context->setTile(apache::thrift::TilePtr{pyTile, eb});
        handler = pyTile->handler();
      });
  return handler;
}

folly::Try<PyObject*> PythonAsyncProcessor::prepareInteractionDispatch(
    const HandlerFunc& function,
    apache::thrift::Cpp2RequestContext* context,
    folly::EventBase* eb) {
  // Codegen only sets `returnsInitialResponse` for req/resp factories;
  // stream/sink factories get their Tile from the zero-arg factory (see
  // `test_stream_interaction_factory`). Guard against a mismatch silently
  // installing the wrong Tile via the zero-arg path.
  DCHECK(
      !function.returnsInitialResponse ||
      function.kind == apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE)
      << "returnsInitialResponse set for non-req/resp factory: "
      << function.fullName;
  if (function.createsInteraction) {
    if (function.returnsInitialResponse) {
      // Factory-with-initial-response: the per-session Tile comes from the
      // handler's return value (installed by `install_interaction_tile` once it
      // completes), not the zero-arg factory. Leave the TilePromise parked;
      // dispatch against `Py_None` since the handler is already bound.
      return folly::Try<PyObject*>(Py_None);
    }
    // On factory failure, return the error so the caller can fail the request
    // with the real message (instead of stranding its callback).
    if (auto factoryError = maybeFulfillTilePromise(function, context, eb)) {
      return folly::Try<PyObject*>(std::move(factoryError));
    }
  } else if (function.interactionType == InteractionType::INTERACTION_V1) {
    if (PyObject* handler = getInteractionHandler(function, context, eb)) {
      return folly::Try<PyObject*>(handler);
    }
    // INTERACTION_V1 inside-method but no PythonTile is bound to this
    // interaction id (e.g. the interaction's factory failed, or it was already
    // terminated). `funcObject` here is the *unbound* dispatch wrapper, so we
    // must not fall through to the Py_None path -- the Cython layer treats
    // Py_None as "ordinary method, already bound" and would call the wrapper
    // with the request IOBuf as `self`. Fail the request with a clear error.
    return folly::Try<PyObject*>(folly::exception_wrapper(
        apache::thrift::TApplicationException(
            apache::thrift::TApplicationException::INTERNAL_ERROR,
            fmt::format(
                "No interaction handler bound for '{}'", function.fullName))));
  }
  return folly::Try<PyObject*>(Py_None);
}

std::unique_ptr<folly::IOBuf> PythonAsyncProcessor::getPythonMetadata() {
  do_python_import();
  return getSerializedPythonMetadata(python_server_);
}

folly::SemiFuture<folly::Unit> PythonAsyncProcessor::handlePythonServerCallback(
    apache::thrift::ProtocolType protocol,
    apache::thrift::Cpp2RequestContext* context,
    apache::thrift::SerializedRequest serializedRequest,
    apache::thrift::RpcKind kind,
    apache::thrift::HandlerCallback<std::unique_ptr<::folly::IOBuf>>::Ptr
        callback) {
  do_python_import();
  const auto& function = functions_.at(context->getMethodName());

  // Interaction routing (req/resp path). A factory method may need to fulfill
  // a parked TilePromise; an inside-interaction method dispatches against the
  // per-session handler instance bound to this interaction id.
  auto interactionSelf =
      prepareInteractionDispatch(function, context, callback->getEventBase());

  auto [promise, future] =
      folly::makePromiseContract<std::unique_ptr<folly::IOBuf>>();
  if (interactionSelf.hasException()) {
    // The interaction factory raised; fail this request with the real error so
    // the client sees the actual message (the implicit `performs` path reports
    // it the same way via the framework's constructor-error handling).
    promise.setException(std::move(interactionSelf).exception());
  } else {
    const int retcode = handleServerCallback(
        function.funcObject,
        *interactionSelf,
        function.fullName,
        context,
        std::move(promise),
        std::move(serializedRequest),
        protocol,
        kind);
    if (retcode != 0) {
      DCHECK(PyErr_Occurred());
      // converts python error to thrown std::runtime_error
      folly::python::handlePythonError(
          "PythonAsyncProcessor::handlePythonServerCallback: ");
    }
  }
  return std::move(future).defer([callback = std::move(callback)](auto&& t) {
    callback->complete(std::move(t));
  });
}

folly::SemiFuture<folly::Unit>
PythonAsyncProcessor::handlePythonServerCallbackStreaming(
    apache::thrift::ProtocolType protocol,
    apache::thrift::Cpp2RequestContext* context,
    apache::thrift::SerializedRequest serializedRequest,
    apache::thrift::RpcKind kind,
    ::apache::thrift::HandlerCallback<::apache::thrift::ResponseAndServerStream<
        std::unique_ptr<::folly::IOBuf>,
        std::unique_ptr<::folly::IOBuf>>>::Ptr callback) {
  do_python_import();
  const auto& function = functions_.at(context->getMethodName());
  auto [promise, future] =
      folly::makePromiseContract<::apache::thrift::ResponseAndServerStream<
          std::unique_ptr<::folly::IOBuf>,
          std::unique_ptr<::folly::IOBuf>>>();
  auto interactionSelf =
      prepareInteractionDispatch(function, context, callback->getEventBase());
  if (interactionSelf.hasException()) {
    // A stream-returning factory method whose factory raised; fail the request
    // with the real message rather than stranding the callback.
    promise.setException(std::move(interactionSelf).exception());
  } else {
    const int retcode = handleServerStreamCallback(
        function.funcObject,
        *interactionSelf,
        function.fullName,
        context,
        std::move(promise),
        std::move(serializedRequest),
        protocol,
        kind);
    if (retcode != 0) {
      DCHECK(PyErr_Occurred());
      // converts python error to thrown std::runtime_error
      folly::python::handlePythonError(
          "PythonAsyncProcessor::handlePythonServerCallbackStreaming: ");
    }
  }
  return std::move(future).defer([callback = std::move(callback)](auto&& t) {
    callback->complete(std::move(t));
  });
}

folly::SemiFuture<folly::Unit>
PythonAsyncProcessor::handlePythonServerCallbackSink(
    apache::thrift::ProtocolType protocol,
    apache::thrift::Cpp2RequestContext* context,
    apache::thrift::SerializedRequest serializedRequest,
    apache::thrift::RpcKind kind,
    ::apache::thrift::HandlerCallback<::apache::thrift::ResponseAndSinkConsumer<
        std::unique_ptr<::folly::IOBuf>,
        std::unique_ptr<::folly::IOBuf>,
        std::unique_ptr<::folly::IOBuf>>>::Ptr callback) {
  do_python_import();
  const auto& function = functions_.at(context->getMethodName());
  auto [promise, future] =
      folly::makePromiseContract<::apache::thrift::ResponseAndSinkConsumer<
          std::unique_ptr<::folly::IOBuf>,
          std::unique_ptr<::folly::IOBuf>,
          std::unique_ptr<::folly::IOBuf>>>();
  const int retcode = handleServerSinkCallback(
      function.funcObject,
      Py_None,
      function.fullName,
      context,
      std::move(promise),
      std::move(serializedRequest),
      protocol,
      kind);
  if (retcode != 0) {
    DCHECK(PyErr_Occurred());
    // converts python error to thrown std::runtime_error
    folly::python::handlePythonError(
        "PythonAsyncProcessor::handlePythonServerCallbackSink: ");
  }
  return std::move(future).defer([callback = std::move(callback)](auto&& t) {
    callback->complete(std::move(t));
  });
}

folly::SemiFuture<folly::Unit>
PythonAsyncProcessor::handlePythonServerCallbackBidi(
    apache::thrift::ProtocolType protocol,
    apache::thrift::Cpp2RequestContext* context,
    apache::thrift::SerializedRequest serializedRequest,
    apache::thrift::RpcKind kind,
    ::apache::thrift::HandlerCallback<
        ::apache::thrift::ResponseAndStreamTransformation<
            std::unique_ptr<::folly::IOBuf>,
            std::unique_ptr<::folly::IOBuf>,
            std::unique_ptr<::folly::IOBuf>>>::Ptr callback) {
  do_python_import();
  const auto& function = functions_.at(context->getMethodName());
  auto [promise, future] = folly::makePromiseContract<
      ::apache::thrift::ResponseAndStreamTransformation<
          std::unique_ptr<::folly::IOBuf>,
          std::unique_ptr<::folly::IOBuf>,
          std::unique_ptr<::folly::IOBuf>>>();
  const int retcode = handleServerBidiCallback(
      function.funcObject,
      Py_None,
      function.fullName,
      context,
      std::move(promise),
      std::move(serializedRequest),
      protocol,
      kind);
  if (retcode != 0) {
    DCHECK(PyErr_Occurred());
    // converts python error to thrown std::runtime_error
    folly::python::handlePythonError(
        "PythonAsyncProcessor::handlePythonServerCallbackBidi: ");
  }
  return std::move(future).defer([callback = std::move(callback)](auto&& t) {
    callback->complete(std::move(t));
  });
}

folly::SemiFuture<folly::Unit>
PythonAsyncProcessor::handlePythonServerCallbackOneway(
    apache::thrift::ProtocolType protocol,
    apache::thrift::Cpp2RequestContext* context,
    apache::thrift::SerializedRequest serializedRequest,
    apache::thrift::RpcKind kind,
    apache::thrift::HandlerCallbackBase::Ptr callback) {
  do_python_import();
  const auto& function = functions_.at(context->getMethodName());

  // Inside-interaction oneway methods dispatch against the per-session handler.
  // Oneway methods are never interaction factories, so this never carries a
  // factory error -- but it may carry a "no handler bound" error if the inside
  // method's interaction has no live tile. There is no response to fail, so we
  // just skip dispatch rather than calling the *unbound* wrapper with Py_None.
  auto interactionSelf =
      prepareInteractionDispatch(function, context, callback->getEventBase());

  auto [promise, future] = folly::makePromiseContract<folly::Unit>();
  if (interactionSelf.hasException()) {
    promise.setException(std::move(interactionSelf).exception());
  } else {
    const int retcode = handleServerCallbackOneway(
        function.funcObject,
        *interactionSelf,
        function.fullName,
        context,
        std::move(promise),
        std::move(serializedRequest),
        protocol,
        kind);
    if (retcode != 0) {
      DCHECK(PyErr_Occurred());
      // converts python error to thrown std::runtime_error
      folly::python::handlePythonError(
          "PythonAsyncProcessor::handlePythonServerCallbackOneway: ");
    }
  }
  return std::move(future).defer([callback = std::move(callback)](auto&&) {});
}

void PythonAsyncProcessor::executeReadEventCallbacks(
    apache::thrift::Cpp2RequestContext* ctx,
    apache::thrift::ContextStack* ctxStack,
    apache::thrift::SerializedRequest& serializedRequest,
    apache::thrift::protocol::PROTOCOL_TYPES protocol) {
  if (ctxStack) {
    ctxStack->preRead();

    apache::thrift::SerializedMessage smsg;
    smsg.protocolType = protocol;
    smsg.buffer = serializedRequest.buffer.get();
    smsg.methodName = ctx->getMethodName();
    ctxStack->onReadData(smsg);

    ctxStack->postRead(
        nullptr,
        serializedRequest.buffer
            ->computeChainDataLength()); // TODO move this call to inside
                                         // the python code
  }
}

void PythonAsyncProcessor::executeRequest(
    apache::thrift::ServerRequest&& request,
    const AsyncProcessorFactory::MethodMetadata& untypedMethodMetadata) {
  const auto& methodMetadata =
      apache::thrift::AsyncProcessorHelper::expectMetadataOfType<
          PythonMetadata>(untypedMethodMetadata);

  auto protocol =
      apache::thrift::detail::ServerRequestHelper::protocol(request);
  auto* ctx = request.requestContext();

  if (!(protocol ==
            apache::thrift::protocol::PROTOCOL_TYPES::T_BINARY_PROTOCOL ||
        protocol ==
            apache::thrift::protocol::PROTOCOL_TYPES::T_COMPACT_PROTOCOL)) {
    request.request()->sendErrorWrapped(
        apache::thrift::TApplicationException(
            "Thrift Python server only supports Binary and Compact Protocols."),
        kConnectionClosingErrorCode);
    return;
  }

  const char* serviceName = serviceName_.c_str();
  const auto& function = functions_.at(ctx->getMethodName());
  auto ctxStack = apache::thrift::ContextStack::create(
      this->getEventHandlersSharedPtr(),
      serviceName,
      function.fullName.c_str(),
      ctx);

  auto serializedRequest =
      std::move(
          apache::thrift::detail::ServerRequestHelper::compressedRequest(
              request))
          .uncompress();

  auto* eb = apache::thrift::detail::ServerRequestHelper::eventBase(request);
  auto executor =
      apache::thrift::detail::ServerRequestHelper::executor(request);
  auto requestData = request.requestData();
  auto req =
      apache::thrift::detail::ServerRequestHelper::request(std::move(request));
  auto kind = methodMetadata.rpcKind;

  try {
    executeReadEventCallbacks(ctx, ctxStack.get(), serializedRequest, protocol);
  } catch (...) {
    folly::exception_wrapper ew(std::current_exception());
    auto throw_func = get_deserialize_error_function(protocol);
    throw_func(ew, std::move(req), ctx, eb, function.fullName.c_str());
    return;
  }

  // This folly::makeSemiFuture().deferValue()
  // ensures that the dispatchRequest(),
  // which imports the cython module that must happen
  // on the python thread, runs in the python thread.
  folly::makeSemiFuture()
      .deferValue([this,
                   protocol,
                   ctx,
                   eb,
                   executor,
                   serviceName,
                   qualifiedMethodName = function.fullName.c_str(),
                   kind,
                   requestData = std::move(requestData),
                   req = std::move(req),
                   ctxStack = std::move(ctxStack),
                   serializedRequest = std::move(serializedRequest)](
                      auto&& /* unused */) mutable {
        return dispatchRequest(
            protocol,
            ctx,
            eb,
            executor,
            std::move(requestData),
            std::move(req),
            std::move(ctxStack),
            serviceName,
            qualifiedMethodName,
            std::move(serializedRequest),
            kind.value());
      })
      .via(executor_);
}

folly::SemiFuture<folly::Unit> PythonAsyncProcessor::dispatchRequestOneway(
    apache::thrift::protocol::PROTOCOL_TYPES protocol,
    apache::thrift::Cpp2RequestContext* ctx,
    apache::thrift::SerializedRequest serializedRequest,
    apache::thrift::RpcKind kind,
    apache::thrift::HandlerCallbackBase::Ptr callback) {
  return folly::coro::co_invoke(
             [this,
              protocol,
              ctx,
              serializedRequest = std::move(serializedRequest),
              kind,
              callback = std::move(
                  callback)]() mutable -> folly::coro::Task<folly::Unit> {
               if (shouldProcessServiceInterceptorsOnRequest(*callback)) {
                 // see discussion below about why we don't handle exception
                 // here.
                 co_await processServiceInterceptorsOnRequest(
                     *callback,
                     emptyInterceptorsArguments(),
                     serializedRequest);
               }
               co_return co_await handlePythonServerCallbackOneway(
                   protocol,
                   ctx,
                   std::move(serializedRequest),
                   kind,
                   std::move(callback));
             })
      .semi();
}

folly::SemiFuture<folly::Unit> PythonAsyncProcessor::dispatchRequestStreaming(
    apache::thrift::protocol::PROTOCOL_TYPES protocol,
    apache::thrift::Cpp2RequestContext* ctx,
    apache::thrift::SerializedRequest serializedRequest,
    apache::thrift::RpcKind kind,
    ::apache::thrift::HandlerCallback<::apache::thrift::ResponseAndServerStream<
        std::unique_ptr<::folly::IOBuf>,
        std::unique_ptr<::folly::IOBuf>>>::Ptr callback) {
  return folly::coro::co_invoke(
             [this,
              protocol,
              ctx,
              serializedRequest = std::move(serializedRequest),
              kind,
              callback = std::move(
                  callback)]() mutable -> folly::coro::Task<folly::Unit> {
               if (shouldProcessServiceInterceptorsOnRequest(*callback)) {
                 // see discussion below about why we don't handle exception
                 // here.
                 co_await processServiceInterceptorsOnRequest(
                     *callback,
                     emptyInterceptorsArguments(),
                     serializedRequest);
               }
               co_return co_await handlePythonServerCallbackStreaming(
                   protocol,
                   ctx,
                   std::move(serializedRequest),
                   kind,
                   std::move(callback));
             })
      .semi();
}

folly::SemiFuture<folly::Unit> PythonAsyncProcessor::dispatchRequestSink(
    apache::thrift::protocol::PROTOCOL_TYPES protocol,
    apache::thrift::Cpp2RequestContext* ctx,
    apache::thrift::SerializedRequest serializedRequest,
    apache::thrift::RpcKind kind,
    ::apache::thrift::HandlerCallback<::apache::thrift::ResponseAndSinkConsumer<
        std::unique_ptr<::folly::IOBuf>,
        std::unique_ptr<::folly::IOBuf>,
        std::unique_ptr<::folly::IOBuf>>>::Ptr callback) {
  return folly::coro::co_invoke(
             [this,
              protocol,
              ctx,
              serializedRequest = std::move(serializedRequest),
              kind,
              callback = std::move(
                  callback)]() mutable -> folly::coro::Task<folly::Unit> {
               if (shouldProcessServiceInterceptorsOnRequest(*callback)) {
                 // see discussion below about why we don't handle exception
                 // here.
                 co_await processServiceInterceptorsOnRequest(
                     *callback,
                     emptyInterceptorsArguments(),
                     serializedRequest);
               }
               co_return co_await handlePythonServerCallbackSink(
                   protocol,
                   ctx,
                   std::move(serializedRequest),
                   kind,
                   std::move(callback));
             })
      .semi();
}

folly::SemiFuture<folly::Unit> PythonAsyncProcessor::dispatchRequestBidi(
    apache::thrift::protocol::PROTOCOL_TYPES protocol,
    apache::thrift::Cpp2RequestContext* ctx,
    apache::thrift::SerializedRequest serializedRequest,
    apache::thrift::RpcKind kind,
    ::apache::thrift::HandlerCallback<
        ::apache::thrift::ResponseAndStreamTransformation<
            std::unique_ptr<::folly::IOBuf>,
            std::unique_ptr<::folly::IOBuf>,
            std::unique_ptr<::folly::IOBuf>>>::Ptr callback) {
  return folly::coro::co_invoke(
             [this,
              protocol,
              ctx,
              serializedRequest = std::move(serializedRequest),
              kind,
              callback = std::move(
                  callback)]() mutable -> folly::coro::Task<folly::Unit> {
               if (shouldProcessServiceInterceptorsOnRequest(*callback)) {
                 // see discussion below about why we don't handle exception
                 // here.
                 co_await processServiceInterceptorsOnRequest(
                     *callback,
                     emptyInterceptorsArguments(),
                     serializedRequest);
               }
               co_return co_await handlePythonServerCallbackBidi(
                   protocol,
                   ctx,
                   std::move(serializedRequest),
                   kind,
                   std::move(callback));
             })
      .semi();
}

folly::SemiFuture<folly::Unit> PythonAsyncProcessor::dispatchRequestResponse(
    apache::thrift::protocol::PROTOCOL_TYPES protocol,
    apache::thrift::Cpp2RequestContext* ctx,
    apache::thrift::SerializedRequest serializedRequest,
    apache::thrift::RpcKind kind,
    HandlerCallback<std::unique_ptr<folly::IOBuf>>::Ptr callback) {
  return folly::coro::co_invoke(
             [this,
              protocol,
              ctx,
              serializedRequest = std::move(serializedRequest),
              kind,
              callback = std::move(
                  callback)]() mutable -> folly::coro::Task<folly::Unit> {
               if (shouldProcessServiceInterceptorsOnRequest(*callback)) {
                 // It may appear that we're discarding exception result of
                 // onRequest interceptor, but it's actually caught via
                 // throw_wrapped, which invokes sendException to report the
                 // callback completed with exception, thereby invoking the
                 // onResponse interceptor. Explicitly handling it here results
                 // in double invocation.
                 co_await processServiceInterceptorsOnRequest(
                     *callback,
                     emptyInterceptorsArguments(),
                     serializedRequest);
               }
               co_return co_await handlePythonServerCallback(
                   protocol,
                   ctx,
                   std::move(serializedRequest),
                   kind,
                   std::move(callback));
             })
      .semi();
}

folly::SemiFuture<folly::Unit> PythonAsyncProcessor::dispatchRequest(
    apache::thrift::protocol::PROTOCOL_TYPES protocol,
    apache::thrift::Cpp2RequestContext* ctx,
    folly::EventBase* eb,
    folly::Executor::KeepAlive<> executor,
    apache::thrift::ServerRequestData requestData,
    apache::thrift::ResponseChannelRequest::UniquePtr req,
    apache::thrift::ContextStack::UniquePtr ctxStack,
    const char* serviceName,
    const char* qualifiedMethodName,
    apache::thrift::SerializedRequest serializedRequest,
    apache::thrift::RpcKind kind) {
  const char* methodName = ctx->getMethodName().c_str();
  auto get_throw_wrapped = [](protocol::PROTOCOL_TYPES protocol) {
    return protocol ==
            apache::thrift::protocol::PROTOCOL_TYPES::T_BINARY_PROTOCOL
        ? &detail::throw_wrapped<
              apache::thrift::BinaryProtocolReader,
              apache::thrift::BinaryProtocolWriter>
        : &detail::throw_wrapped<
              apache::thrift::CompactProtocolReader,
              apache::thrift::CompactProtocolWriter>;
  };

  switch (kind) {
    case apache::thrift::RpcKind::SINGLE_REQUEST_NO_RESPONSE: {
      auto callback = apache::thrift::HandlerCallbackBase::Ptr::make(
          std::move(req),
          std::move(ctxStack),
          apache::thrift::HandlerCallbackBase::MethodNameInfo{
              .serviceName = serviceName,
              .definingServiceName = serviceName,
              .methodName = methodName,
              .qualifiedMethodName = qualifiedMethodName},
          nullptr,
          eb,
          executor,
          ctx,
          nullptr,
          nullptr,
          requestData);
      return dispatchRequestOneway(
          protocol,
          ctx,
          std::move(serializedRequest),
          kind,
          std::move(callback));
    }
    case apache::thrift::RpcKind::SINGLE_REQUEST_STREAMING_RESPONSE: {
      auto return_streaming = protocol ==
              apache::thrift::protocol::PROTOCOL_TYPES::T_BINARY_PROTOCOL
          ? &detail::return_streaming<
                apache::thrift::BinaryProtocolReader,
                apache::thrift::BinaryProtocolWriter>
          : &detail::return_streaming<
                apache::thrift::CompactProtocolReader,
                apache::thrift::CompactProtocolWriter>;
      auto callback = apache::thrift::HandlerCallback<
          ::apache::thrift::ResponseAndServerStream<
              std::unique_ptr<::folly::IOBuf>,
              std::unique_ptr<::folly::IOBuf>>>::Ptr::
          make(
              std::move(req),
              std::move(ctxStack),
              apache::thrift::HandlerCallbackBase::MethodNameInfo{
                  .serviceName = serviceName,
                  .definingServiceName = serviceName,
                  .methodName = methodName,
                  .qualifiedMethodName = qualifiedMethodName},
              return_streaming,
              get_throw_wrapped(protocol),
              ctx->getProtoSeqId(),
              eb,
              executor,
              ctx,
              nullptr,
              nullptr,
              requestData);
      return dispatchRequestStreaming(
          protocol,
          ctx,
          std::move(serializedRequest),
          kind,
          std::move(callback));
    }
    case apache::thrift::RpcKind::SINK: {
      auto return_sink = protocol ==
              apache::thrift::protocol::PROTOCOL_TYPES::T_BINARY_PROTOCOL
          ? &detail::return_sink<
                apache::thrift::BinaryProtocolReader,
                apache::thrift::BinaryProtocolWriter>
          : &detail::return_sink<
                apache::thrift::CompactProtocolReader,
                apache::thrift::CompactProtocolWriter>;
      auto callback = apache::thrift::HandlerCallback<
          ::apache::thrift::ResponseAndSinkConsumer<
              std::unique_ptr<::folly::IOBuf>,
              std::unique_ptr<::folly::IOBuf>,
              std::unique_ptr<::folly::IOBuf>>>::Ptr::
          make(
              std::move(req),
              std::move(ctxStack),
              apache::thrift::HandlerCallbackBase::MethodNameInfo{
                  .serviceName = serviceName,
                  .definingServiceName = serviceName,
                  .methodName = methodName,
                  .qualifiedMethodName = qualifiedMethodName},
              return_sink,
              get_throw_wrapped(protocol),
              ctx->getProtoSeqId(),
              eb,
              executor,
              ctx,
              nullptr,
              nullptr,
              requestData);
      return dispatchRequestSink(
          protocol,
          ctx,
          std::move(serializedRequest),
          kind,
          std::move(callback));
    }
    case apache::thrift::RpcKind::BIDIRECTIONAL_STREAM: {
      auto return_bidistream = protocol ==
              apache::thrift::protocol::PROTOCOL_TYPES::T_BINARY_PROTOCOL
          ? &detail::return_bidistream<
                apache::thrift::BinaryProtocolReader,
                apache::thrift::BinaryProtocolWriter>
          : &detail::return_bidistream<
                apache::thrift::CompactProtocolReader,
                apache::thrift::CompactProtocolWriter>;
      auto callback = apache::thrift::HandlerCallback<
          ::apache::thrift::ResponseAndStreamTransformation<
              std::unique_ptr<::folly::IOBuf>,
              std::unique_ptr<::folly::IOBuf>,
              std::unique_ptr<::folly::IOBuf>>>::Ptr::
          make(
              std::move(req),
              std::move(ctxStack),
              apache::thrift::HandlerCallbackBase::MethodNameInfo{
                  .serviceName = serviceName,
                  .definingServiceName = serviceName,
                  .methodName = methodName,
                  .qualifiedMethodName = qualifiedMethodName},
              return_bidistream,
              get_throw_wrapped(protocol),
              ctx->getProtoSeqId(),
              eb,
              executor,
              ctx,
              nullptr,
              nullptr,
              requestData);
      return dispatchRequestBidi(
          protocol,
          ctx,
          std::move(serializedRequest),
          kind,
          std::move(callback));
    }
    default: {
      auto return_serialized = protocol ==
              apache::thrift::protocol::PROTOCOL_TYPES::T_BINARY_PROTOCOL
          ? &detail::return_serialized<
                apache::thrift::BinaryProtocolReader,
                apache::thrift::BinaryProtocolWriter>
          : &detail::return_serialized<
                apache::thrift::CompactProtocolReader,
                apache::thrift::CompactProtocolWriter>;
      auto callback = apache::thrift::
          HandlerCallback<std::unique_ptr<::folly::IOBuf>>::Ptr::make(
              std::move(req),
              std::move(ctxStack),
              apache::thrift::HandlerCallbackBase::MethodNameInfo{
                  .serviceName = serviceName,
                  .definingServiceName = serviceName,
                  .methodName = methodName,
                  .qualifiedMethodName = qualifiedMethodName},
              return_serialized,
              get_throw_wrapped(protocol),
              ctx->getProtoSeqId(),
              eb,
              executor,
              ctx,
              nullptr,
              nullptr,
              requestData);
      return dispatchRequestResponse(
          protocol,
          ctx,
          std::move(serializedRequest),
          kind,
          std::move(callback));
    }
  }
}

} // namespace apache::thrift::python
