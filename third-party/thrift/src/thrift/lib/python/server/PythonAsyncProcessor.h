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

#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <Python.h>
#include <glog/logging.h>
#include <folly/CppAttributes.h>
#include <folly/ExceptionWrapper.h>
#include <folly/Try.h>
#include <thrift/lib/cpp/protocol/TProtocolTypes.h>
#include <thrift/lib/cpp2/async/AsyncProcessor.h>
#include <thrift/lib/cpp2/async/Interaction.h>
#include <thrift/lib/cpp2/gen/service_tcc.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/lib/python/server/response_helpers.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>
#include <thrift/lib/thrift/gen-cpp2/metadata_types.h>

namespace apache::thrift::python {

enum class LifecycleFunc;

struct HandlerFunc {
  apache::thrift::RpcKind kind;
  PyObject* funcObject;
  std::string fullName; // <serviceName>.<functionName>
  // Interaction routing. For non-interaction methods these stay empty / NONE /
  // false / null and behavior is unchanged.
  //
  // For an inside-interaction method (`interactionType == INTERACTION_V1`),
  // `funcObject` is the *unbound* dispatch function (e.g.
  // `CounterInterface._fbthrift__handler_add`); the Cython dispatch layer binds
  // it to the per-session handler instance.
  //
  // `interactionName` is a view into the Python `bytes` held by the function
  // table (same lifetime as the funcMap keys); used to resolve the factory for
  // a created interaction. Set on factory methods (used) and inside methods.
  std::string_view interactionName;
  apache::thrift::AsyncProcessorFactory::MethodMetadata::InteractionType
      interactionType{apache::thrift::AsyncProcessorFactory::MethodMetadata::
                          InteractionType::NONE};
  bool createsInteraction{false};
  // Zero-arg Python callable that constructs the interaction's per-session
  // handler instance (i.e. `self.create<InteractionName>`). Borrowed reference,
  // owned by the Python function table that outlives this processor.
  PyObject* factoryObject{nullptr};
};

// Ordinary (non-interaction) service method.
HandlerFunc makeHandlerFunc(
    apache::thrift::RpcKind kind,
    PyObject* funcObject,
    const std::string& serviceName,
    std::string_view functionName);

// Interaction method: either the factory that creates `interactionName`
// (`createsInteraction == true`) or a method dispatched inside it
// (`createsInteraction == false`). `factoryObject` is the per-session handler
// constructor. `interactionType` is derived from `createsInteraction`.
HandlerFunc makeInteractionHandlerFunc(
    apache::thrift::RpcKind kind,
    PyObject* funcObject,
    const std::string& serviceName,
    std::string_view functionName,
    std::string_view interactionName,
    bool createsInteraction,
    PyObject* factoryObject);

// Per-interaction state on the C++ side. Holds a strong Python reference to the
// user's interaction handler instance so subsequent inbound interaction method
// calls can be routed into Python. Mirrors the C++ `apache::thrift::Tile` model
// that keys per-interaction state by interaction id in the connection context.
class PythonTile : public apache::thrift::Tile {
 public:
  PythonTile(PyObject* handler, folly::Executor::KeepAlive<> executor);
  ~PythonTile() override;

  PythonTile(const PythonTile&) = delete;
  PythonTile& operator=(const PythonTile&) = delete;
  PythonTile(PythonTile&&) = delete;
  PythonTile& operator=(PythonTile&&) = delete;

  PyObject* handler() const { return handler_; }

 private:
  PyObject* handler_; // owned reference (Py_INCREF in ctor / DECREF in dtor)
  // Python (asyncio-loop) executor used to drop `handler_` on the loop thread
  // at destruction; the KeepAlive guarantees it outlives that scheduled decref.
  folly::Executor::KeepAlive<> executor_;
};

using FunctionMapType = std::map<std::string_view, HandlerFunc>;

class PythonAsyncProcessor : public apache::thrift::GeneratedAsyncProcessorBase,
                             public apache::thrift::ServerInterface {
 public:
  PythonAsyncProcessor(
      PyObject* python_server,
      const FunctionMapType& functions,
      folly::Executor::KeepAlive<> executor,
      std::string serviceName)
      : python_server_(python_server),
        functions_(functions),
        executor_(std::move(executor)),
        serviceName_(std::move(serviceName)) {
    // Precompute interaction name -> per-session factory so interaction
    // dispatch doesn't linearly scan `functions_` on every request. All entries
    // for a given interaction carry the same factory callable, so first-wins
    // via `emplace` is fine. Keys are `string_view`s into the Python bytes held
    // by the function table, which outlives this processor (same as
    // functions_).
    for (const auto& [methodName, fn] : functions_) {
      if (fn.factoryObject != nullptr && !fn.interactionName.empty()) {
        interactionFactories_.emplace(fn.interactionName, fn.factoryObject);
      }
    }
  }

  PythonAsyncProcessor(PythonAsyncProcessor&&) = delete;
  PythonAsyncProcessor(const PythonAsyncProcessor&) = delete;
  PythonAsyncProcessor& operator=(PythonAsyncProcessor&&) = delete;
  PythonAsyncProcessor& operator=(const PythonAsyncProcessor&) = delete;

  struct PythonMetadata final
      : public apache::thrift::AsyncProcessorFactory::MethodMetadata {
    PythonMetadata(
        ExecutorType executor,
        InteractionType interaction,
        apache::thrift::RpcKind rpcKind,
        const std::optional<std::string>& interactionName = std::nullopt,
        bool createsInteraction = false)
        : MethodMetadata(
              executor,
              interaction,
              rpcKind,
              apache::thrift::concurrency::NORMAL,
              interactionName,
              createsInteraction) {}
  };

  std::unique_ptr<folly::IOBuf> getPythonMetadata();

  void getServiceMetadata(
      apache::thrift::metadata::ThriftServiceMetadataResponse& response)
      override {
    std::unique_ptr<folly::IOBuf> buf = folly::via(this->executor_, [this] {
                                          return getPythonMetadata();
                                        }).get();
    apache::thrift::BinarySerializer::deserialize<
        apache::thrift::metadata::ThriftServiceMetadataResponse>(
        buf.get(), response);
  }

  void processSerializedCompressedRequestWithMetadata(
      apache::thrift::ResponseChannelRequest::UniquePtr,
      apache::thrift::SerializedCompressedRequest&&,
      const apache::thrift::AsyncProcessorFactory::MethodMetadata&,
      apache::thrift::protocol::PROTOCOL_TYPES,
      apache::thrift::Cpp2RequestContext*,
      folly::EventBase*,
      apache::thrift::concurrency::ThreadManager*) override {
    LOG(FATAL)
        << "processSerializedCompressedRequestWithMetadata support has been "
        << "removed from PythonAsyncProcessor";
  }

  void executeRequest(
      apache::thrift::ServerRequest&& request,
      const AsyncProcessorFactory::MethodMetadata& untypedMethodMetadata)
      override;

  // Dud method for GeneratedAsyncProcessor
  std::string_view getServiceName() override { return serviceName_; }

  // Dud method for ServerInterface
  std::string_view getGeneratedName() const override {
    LOG(WARNING)
        << "PythonAsyncProcessor::getGeneratedName called unexpectedly";
    return "PythonService";
  }

  // Dud method for ServerInterface
  std::unique_ptr<apache::thrift::AsyncProcessor> getProcessor() override {
    LOG(WARNING) << "PythonAsyncProcessor::getProcessor called unexpectedly";
    return nullptr;
  }

 protected:
  // Framework override: construct the per-session Tile for a newly created
  // interaction named `name` by invoking its registered Python factory.
  std::unique_ptr<apache::thrift::Tile> createInteractionImpl(
      const std::string& name, int16_t protocol) override;

 private:
  // Look up the Python factory callable (`self.create<InteractionName>`) for
  // the interaction `name` from the function table. Returns a borrowed
  // reference, or nullptr if none is registered.
  PyObject* FOLLY_NULLABLE findInteractionFactory(std::string_view name);

  // For an explicit factory method (createsInteraction): if the framework has
  // parked a TilePromise for this request's interaction id, build the real
  // PythonTile via the registered factory and fulfill the promise on the
  // connection's EventBase thread. No-op if there is no parked promise.
  //
  // Returns an empty exception_wrapper on success; if the Python factory
  // raised, returns that error so the caller can fail the factory request with
  // the real message (rather than throwing here and stranding the request's
  // callback).
  folly::exception_wrapper maybeFulfillTilePromise(
      const HandlerFunc& function,
      apache::thrift::Cpp2RequestContext* context,
      folly::EventBase* eb);

  // For an inside-interaction method, find the PythonTile bound to this
  // request's interaction id and return a borrowed reference to its Python
  // handler instance (the per-session state). Returns nullptr if `function` is
  // not an inside-interaction method or no PythonTile is bound. The Cython
  // dispatch layer binds the unbound `funcObject` to this instance.
  PyObject* FOLLY_NULLABLE getInteractionHandler(
      const HandlerFunc& function, apache::thrift::Cpp2RequestContext* context);

  // Shared interaction step for a dispatch path: for a factory method, fulfill
  // any parked TilePromise; for an inside-interaction method, return the
  // per-session handler instance to bind the dispatch wrapper to. Returns
  // `Py_None` (borrowed) for ordinary methods / when no tile is bound. If a
  // factory method's Python factory raised, the returned Try holds that error
  // so the caller can fail the request with the real message.
  folly::Try<PyObject*> prepareInteractionDispatch(
      const HandlerFunc& function,
      apache::thrift::Cpp2RequestContext* context,
      folly::EventBase* eb);

  PyObject* python_server_;
  const FunctionMapType& functions_;
  // Interaction name -> per-session handler factory (`self.create<Name>`),
  // derived from `functions_` at construction. Borrowed references owned by the
  // Python function table that outlives this processor.
  std::unordered_map<std::string_view, PyObject*> interactionFactories_;
  folly::Executor::KeepAlive<> executor_;
  std::string serviceName_;

  folly::SemiFuture<folly::Unit> handlePythonServerCallback(
      apache::thrift::ProtocolType protocol,
      apache::thrift::Cpp2RequestContext* context,
      apache::thrift::SerializedRequest serializedRequest,
      apache::thrift::RpcKind kind,
      apache::thrift::HandlerCallback<std::unique_ptr<::folly::IOBuf>>::Ptr
          callback);

  folly::SemiFuture<folly::Unit> handlePythonServerCallbackStreaming(
      apache::thrift::ProtocolType protocol,
      apache::thrift::Cpp2RequestContext* context,
      apache::thrift::SerializedRequest serializedRequest,
      apache::thrift::RpcKind kind,
      ::apache::thrift::HandlerCallback<
          ::apache::thrift::ResponseAndServerStream<
              std::unique_ptr<::folly::IOBuf>,
              std::unique_ptr<::folly::IOBuf>>>::Ptr callback);

  folly::SemiFuture<folly::Unit> handlePythonServerCallbackSink(
      apache::thrift::ProtocolType protocol,
      apache::thrift::Cpp2RequestContext* context,
      apache::thrift::SerializedRequest serializedRequest,
      apache::thrift::RpcKind kind,
      ::apache::thrift::HandlerCallback<
          ::apache::thrift::ResponseAndSinkConsumer<
              std::unique_ptr<::folly::IOBuf>,
              std::unique_ptr<::folly::IOBuf>,
              std::unique_ptr<::folly::IOBuf>>>::Ptr callback);

  folly::SemiFuture<folly::Unit> handlePythonServerCallbackBidi(
      apache::thrift::ProtocolType protocol,
      apache::thrift::Cpp2RequestContext* context,
      apache::thrift::SerializedRequest serializedRequest,
      apache::thrift::RpcKind kind,
      ::apache::thrift::HandlerCallback<
          ::apache::thrift::ResponseAndStreamTransformation<
              std::unique_ptr<::folly::IOBuf>,
              std::unique_ptr<::folly::IOBuf>,
              std::unique_ptr<::folly::IOBuf>>>::Ptr callback);

  folly::SemiFuture<folly::Unit> handlePythonServerCallbackOneway(
      apache::thrift::ProtocolType protocol,
      apache::thrift::Cpp2RequestContext* context,
      apache::thrift::SerializedRequest serializedRequest,
      apache::thrift::RpcKind kind,
      apache::thrift::HandlerCallbackBase::Ptr callback);

  folly::SemiFuture<folly::Unit> dispatchRequest(
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
      apache::thrift::RpcKind kind);

  folly::SemiFuture<folly::Unit> dispatchRequestOneway(
      apache::thrift::protocol::PROTOCOL_TYPES protocol,
      apache::thrift::Cpp2RequestContext* ctx,
      apache::thrift::SerializedRequest serializedRequest,
      apache::thrift::RpcKind kind,
      apache::thrift::HandlerCallbackBase::Ptr callback);

  folly::SemiFuture<folly::Unit> dispatchRequestStreaming(
      apache::thrift::protocol::PROTOCOL_TYPES protocol,
      apache::thrift::Cpp2RequestContext* ctx,
      apache::thrift::SerializedRequest serializedRequest,
      apache::thrift::RpcKind kind,
      ::apache::thrift::HandlerCallback<
          ::apache::thrift::ResponseAndServerStream<
              std::unique_ptr<::folly::IOBuf>,
              std::unique_ptr<::folly::IOBuf>>>::Ptr callback);

  folly::SemiFuture<folly::Unit> dispatchRequestSink(
      apache::thrift::protocol::PROTOCOL_TYPES protocol,
      apache::thrift::Cpp2RequestContext* ctx,
      apache::thrift::SerializedRequest serializedRequest,
      apache::thrift::RpcKind kind,
      ::apache::thrift::HandlerCallback<
          ::apache::thrift::ResponseAndSinkConsumer<
              std::unique_ptr<::folly::IOBuf>,
              std::unique_ptr<::folly::IOBuf>,
              std::unique_ptr<::folly::IOBuf>>>::Ptr callback);

  folly::SemiFuture<folly::Unit> dispatchRequestBidi(
      apache::thrift::protocol::PROTOCOL_TYPES protocol,
      apache::thrift::Cpp2RequestContext* ctx,
      apache::thrift::SerializedRequest serializedRequest,
      apache::thrift::RpcKind kind,
      ::apache::thrift::HandlerCallback<
          ::apache::thrift::ResponseAndStreamTransformation<
              std::unique_ptr<::folly::IOBuf>,
              std::unique_ptr<::folly::IOBuf>,
              std::unique_ptr<::folly::IOBuf>>>::Ptr callback);

  folly::SemiFuture<folly::Unit> dispatchRequestResponse(
      apache::thrift::protocol::PROTOCOL_TYPES protocol,
      apache::thrift::Cpp2RequestContext* ctx,
      apache::thrift::SerializedRequest serializedRequest,
      apache::thrift::RpcKind kind,
      apache::thrift::HandlerCallback<std::unique_ptr<::folly::IOBuf>>::Ptr
          callback);

  void executeReadEventCallbacks(
      apache::thrift::Cpp2RequestContext* ctx,
      apache::thrift::ContextStack* ctxStack,
      apache::thrift::SerializedRequest& serializedRequest,
      apache::thrift::protocol::PROTOCOL_TYPES protocol);
};

} // namespace apache::thrift::python
