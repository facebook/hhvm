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

#include <exception>
#include <mutex>
#include <string>
#include <string_view>
#include <variant>

#include <folly/ExceptionWrapper.h>
#include <folly/Portability.h>
#include <folly/String.h>
#include <folly/Synchronized.h>
#include <folly/Unit.h>
#include <folly/concurrency/memory/PrimaryPtr.h>
#include <folly/container/F14Map.h>
#include <folly/futures/Future.h>
#include <folly/io/async/EventBase.h>
#include <folly/lang/Badge.h>

#include <thrift/lib/cpp/TApplicationException.h>
#include <thrift/lib/cpp/TProcessor.h>
#include <thrift/lib/cpp/Thrift.h>
#include <thrift/lib/cpp/concurrency/Thread.h>
#include <thrift/lib/cpp/concurrency/ThreadManager.h>
#include <thrift/lib/cpp/protocol/TProtocolTypes.h>
#include <thrift/lib/cpp/transport/THeader.h>
#include <thrift/lib/cpp2/SerializationSwitch.h>
#include <thrift/lib/cpp2/Thrift.h>
#include <thrift/lib/cpp2/TrustedServerException.h>
#include <thrift/lib/cpp2/async/AsyncProcessorFactory.h>
#include <thrift/lib/cpp2/async/Interaction.h>
#include <thrift/lib/cpp2/async/ReplyInfo.h>
#include <thrift/lib/cpp2/async/ResponseChannel.h>
#include <thrift/lib/cpp2/async/RpcTypes.h>
#include <thrift/lib/cpp2/async/ServerRequestData.h>
#include <thrift/lib/cpp2/async/ServerStream.h>
#include <thrift/lib/cpp2/async/Sink.h>
#include <thrift/lib/cpp2/async/processor/AsyncProcessor.h>
#include <thrift/lib/cpp2/async/processor/EventTask.h>
#include <thrift/lib/cpp2/async/processor/RequestParams.h>
#include <thrift/lib/cpp2/async/processor/ServerRequest.h>
#include <thrift/lib/cpp2/async/processor/ServerRequestHelper.h>
#include <thrift/lib/cpp2/async/processor/ServerRequestTask.h>
#include <thrift/lib/cpp2/async/processor/ServiceHandlerBase.h>
#include <thrift/lib/cpp2/protocol/Protocol.h>
#include <thrift/lib/cpp2/schema/SchemaV1.h>
#include <thrift/lib/cpp2/server/ConcurrencyControllerInterface.h>
#include <thrift/lib/cpp2/server/Cpp2ConnContext.h>
#include <thrift/lib/cpp2/server/IOWorkerContext.h>
#include <thrift/lib/cpp2/server/RequestPileInterface.h>
#include <thrift/lib/cpp2/server/ResourcePoolHandle.h>
#include <thrift/lib/cpp2/util/Checksum.h>
#include <thrift/lib/cpp2/util/IntrusiveSharedPtr.h>
#include <thrift/lib/cpp2/util/TypeErasedValue.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>
#include <thrift/lib/thrift/gen-cpp2/metadata_types.h>

namespace apache::thrift {

namespace detail {
template <typename T>
struct HandlerCallbackHelper;
}

class ServerRequest;
class IResourcePoolAcceptor;
class ServerInterface;

class GeneratedAsyncProcessorBase : public AsyncProcessor {
 public:
  template <typename Derived>
  using ProcessFunc = void (Derived::*)(
      ResponseChannelRequest::UniquePtr,
      SerializedCompressedRequest&&,
      Cpp2RequestContext* context,
      folly::EventBase* eb,
      concurrency::ThreadManager* tm);

  template <typename Derived>
  using ExecuteFunc = void (Derived::*)(ServerRequest&& request);

  template <typename Derived>
  struct ProcessFuncs {
    ProcessFunc<Derived> compact;
    ProcessFunc<Derived> binary;
    ExecuteFunc<Derived> compactExecute;
    ExecuteFunc<Derived> binaryExecute;
  };

  template <typename ProcessFuncs>
  using ProcessMap = folly::F14ValueMap<std::string, ProcessFuncs>;

  template <typename Derived>
  using InteractionConstructor = std::unique_ptr<Tile> (Derived::*)();
  template <typename InteractionConstructor>
  using InteractionConstructorMap =
      folly::F14ValueMap<std::string, InteractionConstructor>;

  // Sends an error response if validation fails.
  static bool validateRpcKind(
      const ResponseChannelRequest::UniquePtr& req, RpcKind kind);
  static bool validateRpcKind(const ServerRequest& req);

 protected:
  template <typename ProtocolIn, typename Args>
  static void deserializeRequest(
      Args& args,
      folly::StringPiece methodName,
      const SerializedRequest& serializedRequest,
      ContextStack* c);

  template <typename ProtocolIn, typename Args>
  static void simpleDeserializeRequest(
      Args& args, const SerializedRequest& serializedRequest);

  template <typename Response, typename ProtocolOut, typename Result>
  static Response serializeResponseImpl(
      std::string_view method,
      ProtocolOut* prot,
      int32_t protoSeqId,
      ContextStack* ctx,
      const Result& result);

  template <typename ProtocolOut, typename Result>
  static LegacySerializedResponse serializeLegacyResponse(
      std::string_view method,
      ProtocolOut* prot,
      int32_t protoSeqId,
      ContextStack* ctx,
      const Result& result);

  template <typename ProtocolOut, typename Result>
  static SerializedResponse serializeResponse(
      std::string_view method,
      ProtocolOut* prot,
      ContextStack* ctx,
      const Result& result);

  // Returns true if setup succeeded and sends an error response otherwise.
  // Always runs in eb thread.
  // tm is null if the method is annotated with thread='eb'.
  bool setUpRequestProcessing(
      const ResponseChannelRequest::UniquePtr& req,
      Cpp2RequestContext* ctx,
      folly::EventBase* eb,
      concurrency::ThreadManager* tm,
      RpcKind kind,
      ServerInterface* si,
      folly::StringPiece interaction = "",
      bool isInteractionFactoryFunction = false);

  bool setUpRequestProcessing(ServerRequest& req);

  void processInteraction(ServerRequest&& req) override;

 public:
  template <typename ChildType>
  static void processInThread(
      ResponseChannelRequest::UniquePtr req,
      SerializedCompressedRequest&& serializedRequest,
      Cpp2RequestContext* ctx,
      folly::EventBase* eb,
      concurrency::ThreadManager* tm,
      RpcKind kind,
      ExecuteFunc<ChildType> executeFunc,
      ChildType* childClass);

 private:
  template <typename ChildType>
  static std::unique_ptr<concurrency::Runnable> makeEventTaskForRequest(
      ResponseChannelRequest::UniquePtr req,
      SerializedCompressedRequest&& serializedRequest,
      Cpp2RequestContext* ctx,
      folly::Executor::KeepAlive<> executor,
      RpcKind kind,
      ExecuteFunc<ChildType> executeFunc,
      ChildType* childClass,
      Tile* tile);

  // Returns false if interaction id is duplicated.
  bool createInteraction(
      const ResponseChannelRequest::UniquePtr& req,
      int64_t id,
      std::string&& name,
      Cpp2RequestContext& ctx,
      concurrency::ThreadManager* tm,
      folly::EventBase& eb,
      ServerInterface* si,
      bool isFactoryFunction);

  // Returns false if interaction id is duplicated.
  bool createInteraction(ServerRequest& req);

 protected:
  virtual std::unique_ptr<Tile> createInteractionImpl(
      const std::string& name,
      // This is only used by Rust, since Rust implementations of interaction is
      // not fully compatible with standard interaction contract.
      int16_t protocol);

 public:
  void terminateInteraction(
      int64_t id, Cpp2ConnContext& conn, folly::EventBase&) noexcept final;
  void destroyAllInteractions(
      Cpp2ConnContext& conn, folly::EventBase&) noexcept final;
};

template <typename ChildType>
class RequestTask final : public EventTask {
 public:
  RequestTask(
      ResponseChannelRequest::UniquePtr req,
      SerializedCompressedRequest&& serializedRequest,
      folly::Executor::KeepAlive<> executor,
      Cpp2RequestContext* ctx,
      bool oneway,
      ChildType* childClass,
      GeneratedAsyncProcessorBase::ExecuteFunc<ChildType> executeFunc)
      : EventTask(
            std::move(req),
            std::move(serializedRequest),
            std::move(executor),
            ctx,
            oneway),
        childClass_(childClass),
        executeFunc_(executeFunc) {}

  void run() override;

 private:
  ChildType* childClass_;
  GeneratedAsyncProcessorBase::ExecuteFunc<ChildType> executeFunc_;
};

/**
 * Base-class for generated service handlers. While AsyncProcessorFactory and
 * ServiceHandlerBase are separate layers of abstraction, generated code reuse
 * the same object for both.
 */
class ServerInterface : public virtual AsyncProcessorFactory,
                        public virtual ServiceHandlerBase {
 public:
  ServerInterface() = default;
  ServerInterface(const ServerInterface&) = delete;
  ServerInterface& operator=(const ServerInterface&) = delete;

  std::string_view getName() const {
    return nameOverride_ ? *nameOverride_ : getGeneratedName();
  }
  virtual std::string_view getGeneratedName() const = 0;

  [[deprecated("Replaced by getRequestContext")]] Cpp2RequestContext*
  getConnectionContext() const {
    return requestParams_.requestContext_;
  }

  Cpp2RequestContext* getRequestContext() const {
    return requestParams_.requestContext_;
  }

  [[deprecated("Replaced by setRequestContext")]] void setConnectionContext(
      Cpp2RequestContext* c) {
    requestParams_.requestContext_ = c;
  }

  void setRequestContext(Cpp2RequestContext* c) {
    requestParams_.requestContext_ = c;
  }

  void setThreadManager(concurrency::ThreadManager* tm) {
    requestParams_.threadManager_ = tm;
  }

  // For cases where caller only needs the folly::Executor* interface.
  // These calls can be replaced with getHandlerExecutor.
  [[deprecated("Use getHandlerExecutor")]] folly::Executor* getThreadManager() {
    return getHandlerExecutor();
  }

  // For cases where the caller needs the ThreadManager interface. Caller
  // needs to be refactored to replace these calls with getHandlerExecutor.
  [[deprecated("Use getHandlerExecutor")]] concurrency::ThreadManager*
  getThreadManager_deprecated() {
    return requestParams_.threadManager_;
  }

  void setHandlerExecutor(folly::Executor* executor) {
    requestParams_.handlerExecutor_ = executor;
  }

  folly::Executor* getHandlerExecutor() {
    return requestParams_.handlerExecutor_ ? requestParams_.handlerExecutor_
                                           : requestParams_.threadManager_;
  }

  folly::Executor::KeepAlive<> getBlockingThreadManager() {
    if (requestParams_.threadManager_) {
      return BlockingThreadManager::create(requestParams_.threadManager_);
    } else {
      return BlockingThreadManager::create(requestParams_.handlerExecutor_);
    }
  }

  static folly::Executor::KeepAlive<> getBlockingThreadManager(
      concurrency::ThreadManager* threadManager) {
    return BlockingThreadManager::create(threadManager);
  }

  static folly::Executor::KeepAlive<> getBlockingThreadManager(
      folly::Executor* executor) {
    return BlockingThreadManager::create(executor);
  }

  void setEventBase(folly::EventBase* eb);

  folly::EventBase* getEventBase() { return requestParams_.eventBase_; }

  void clearRequestParams() { requestParams_ = RequestParams(); }

  virtual concurrency::PRIORITY getRequestPriority(
      Cpp2RequestContext* ctx, concurrency::PRIORITY prio);
  // TODO: replace with getRequestExecutionScope.
  concurrency::PRIORITY getRequestPriority(Cpp2RequestContext* ctx) {
    return getRequestPriority(ctx, concurrency::NORMAL);
  }

  virtual concurrency::ThreadManager::ExecutionScope getRequestExecutionScope(
      Cpp2RequestContext* ctx, concurrency::PRIORITY defaultPriority) {
    concurrency::ThreadManager::ExecutionScope es(
        getRequestPriority(ctx, defaultPriority));
    return es;
  }
  concurrency::ThreadManager::ExecutionScope getRequestExecutionScope(
      Cpp2RequestContext* ctx) {
    return getRequestExecutionScope(ctx, concurrency::NORMAL);
  }

  std::vector<ServiceHandlerBase*> getServiceHandlers() override {
    return {this};
  }

  /**
   * The concrete instance of MethodMetadata that generated AsyncProcessors
   * expect will be passed to them. Therefore, generated service handlers will
   * also create instances of these for entries in
   * AsyncProcessorFactory::createMethodMetadata.
   */
  template <typename Processor>
  struct GeneratedMethodMetadata final
      : public AsyncProcessorFactory::MethodMetadata {
    GeneratedMethodMetadata(
        GeneratedAsyncProcessorBase::ProcessFuncs<Processor> funcs,
        ExecutorType executor,
        InteractionType interaction,
        RpcKind rpcKind,
        concurrency::PRIORITY priority,
        const std::optional<std::string>& interactionName,
        const bool createsInteraction)
        : MethodMetadata(
              executor,
              interaction,
              rpcKind,
              priority,
              interactionName,
              createsInteraction),
          processFuncs(funcs) {}

    GeneratedAsyncProcessorBase::ProcessFuncs<Processor> processFuncs;
  };

 protected:
  folly::Executor::KeepAlive<> getInternalKeepAlive();

 private:
  class BlockingThreadManager : public folly::Executor {
   public:
    static folly::Executor::KeepAlive<> create(
        concurrency::ThreadManager* executor) {
      return makeKeepAlive(new BlockingThreadManager(executor));
    }
    static folly::Executor::KeepAlive<> create(folly::Executor* executor) {
      return makeKeepAlive(new BlockingThreadManager(executor));
    }

    void add(folly::Func f) override;

   private:
    explicit BlockingThreadManager(concurrency::ThreadManager* threadManager)
        : threadManagerKa_(folly::getKeepAliveToken(threadManager)) {}
    explicit BlockingThreadManager(folly::Executor* executor)
        : executorKa_(folly::getKeepAliveToken(executor)) {}

    bool keepAliveAcquire() noexcept override;
    void keepAliveRelease() noexcept override;

    static constexpr std::chrono::seconds kTimeout{30};
    std::atomic<size_t> keepAliveCount_{1};
    folly::Executor::KeepAlive<concurrency::ThreadManager> threadManagerKa_;
    folly::Executor::KeepAlive<folly::Executor> executorKa_;
  };

  /**
   * This variable is only used for sync calls when in a threadpool it
   * is threadlocal, because the threadpool will probably be
   * processing multiple requests simultaneously, and we don't want to
   * mix up the connection contexts.
   *
   * This threadlocal trick doesn't work for async requests, because
   * multiple async calls can be running on the same thread.  Instead,
   * use the callback->getConnectionContext() method.  This reqCtx_
   * will be NULL for async calls.
   */
  static thread_local RequestParams requestParams_;

  std::optional<std::string> nameOverride_;

 protected:
  /**
   * If set, getName will return this name instead of getGeneratedName.
   *
   * NOTE: This method will be removed soon. Do not call it directly.
   */
  void setNameOverride(std::string name) { nameOverride_ = std::move(name); }
};

template <class T>
class HandlerCallback;

class HandlerCallbackBase;

namespace detail {
// These functions allow calling the function within generated code since
// the corresponding functions are protected in HandlerCallbackBase

#if FOLLY_HAS_COROUTINES
bool shouldProcessServiceInterceptorsOnRequest(HandlerCallbackBase&) noexcept;

folly::coro::Task<void> processServiceInterceptorsOnRequest(
    HandlerCallbackBase&,
    detail::ServiceInterceptorOnRequestArguments arguments);
#endif // FOLLY_HAS_COROUTINES
} // namespace detail

/**
 * HandlerCallback class for async callbacks.
 *
 * These are constructed by the generated code, and your handler calls
 * either result(value), done(), exception(ex), or appOverloadedException() to
 * finish the async call.  Only one of these must be called, otherwise your
 * client will likely get confused with multiple response messages.
 */
class HandlerCallbackBase {
 private:
  IOWorkerContext::ReplyQueue& getReplyQueue() {
    auto workerContext = reqCtx_->getConnectionContext()->getWorkerContext();
    DCHECK(workerContext != nullptr);
    return workerContext->getReplyQueue();
  }

 protected:
  using exnw_ptr = void (*)(
      ResponseChannelRequest::UniquePtr,
      int32_t protoSeqId,
      ContextStack*,
      folly::exception_wrapper,
      Cpp2RequestContext*);

  void maybeNotifyComplete() {
    // We do not expect this to be reentrant
    if (notifyRequestPile_) {
      notifyRequestPile_->onRequestFinished(requestData_);
      notifyRequestPile_ = nullptr;
    }
    if (notifyConcurrencyController_) {
      notifyConcurrencyController_->onRequestFinished(requestData_);
      notifyConcurrencyController_ = nullptr;
    }
  }

  util::BasicIntrusiveSharedPtrControlBlock intrusivePtrControlBlock_;

 public:
  struct IntrusiveSharedPtrAccess {
    static void acquireRef(HandlerCallbackBase& callback) noexcept {
      callback.intrusivePtrControlBlock_.acquireRef();
    }
    static util::BasicIntrusiveSharedPtrControlBlock::RefCount releaseRef(
        HandlerCallbackBase& callback) noexcept {
      return callback.intrusivePtrControlBlock_.releaseRef();
    }
  };

  using Ptr =
      util::IntrusiveSharedPtr<HandlerCallbackBase, IntrusiveSharedPtrAccess>;

 private:
  Ptr sharedFromThis() {
    // Constructing from raw pointer is safe in this case because
    // `this` is guaranteed to be alive while the current
    // function is executing.
    return Ptr(typename Ptr::UnsafelyFromRawPointer(), this);
  }

 public:
  struct MethodNameInfo {
    // The "leaf" or most derived service name
    std::string_view serviceName;
    // The service name where the incoming method name is defined (could be
    // different if inherited service)
    std::string_view definingServiceName;
    // Self explanatory
    std::string_view methodName;
    // This is {serviceName}.{methodName}
    std::string_view qualifiedMethodName;
  };

  HandlerCallbackBase() : eb_(nullptr), reqCtx_(nullptr), protoSeqId_(0) {}

  HandlerCallbackBase(
      ResponseChannelRequest::UniquePtr req,
      ContextStack::UniquePtr ctx,
      MethodNameInfo methodNameInfo,
      exnw_ptr ewp,
      folly::EventBase* eb,
      concurrency::ThreadManager* tm,
      Cpp2RequestContext* reqCtx,
      TilePtr&& interaction = {})
      : req_(std::move(req)),
        ctx_(std::move(ctx)),
        interaction_(std::move(interaction)),
        methodNameInfo_(std::move(methodNameInfo)),
        ewp_(ewp),
        eb_(eb),
        executor_(
            tm ? tm->getKeepAlive(
                     reqCtx->getRequestExecutionScope(),
                     apache::thrift::concurrency::ThreadManager::Source::
                         INTERNAL)
               : nullptr),
        reqCtx_(reqCtx),
        protoSeqId_(0) {}

  HandlerCallbackBase(
      ResponseChannelRequest::UniquePtr req,
      ContextStack::UniquePtr ctx,
      MethodNameInfo methodNameInfo,
      exnw_ptr ewp,
      folly::EventBase* eb,
      folly::Executor::KeepAlive<> executor,
      Cpp2RequestContext* reqCtx,
      RequestCompletionCallback* notifyRequestPile,
      RequestCompletionCallback* notifyConcurrencyController,
      ServerRequestData requestData,
      TilePtr&& interaction = {})
      : req_(std::move(req)),
        ctx_(std::move(ctx)),
        interaction_(std::move(interaction)),
        methodNameInfo_(std::move(methodNameInfo)),
        ewp_(ewp),
        eb_(eb),
        executor_(std::move(executor)),
        reqCtx_(reqCtx),
        protoSeqId_(0),
        notifyRequestPile_(notifyRequestPile),
        notifyConcurrencyController_(notifyConcurrencyController),
        requestData_(std::move(requestData)) {}

  virtual ~HandlerCallbackBase();

  static void releaseRequest(
      ResponseChannelRequest::UniquePtr request,
      folly::EventBase* eb,
      TilePtr interaction = {});

  void exception(std::exception_ptr ex) {
    class ExceptionHandler {
     public:
      explicit ExceptionHandler(std::exception_ptr&& ex) : ex_(std::move(ex)) {}

      folly::exception_wrapper exception() && {
        return folly::exception_wrapper(std::move(ex_));
      }

      static void handle(
          HandlerCallbackBase& callback, folly::exception_wrapper&& ew) {
        callback.doException(ew.to_exception_ptr());
      }

     private:
      std::exception_ptr ex_;
    };
    handleExceptionAndExecuteServiceInterceptors(
        ExceptionHandler(std::move(ex)));
  }

  void exception(folly::exception_wrapper ew) {
    class ExceptionHandler {
     public:
      explicit ExceptionHandler(folly::exception_wrapper&& ew)
          : ew_(std::move(ew)) {}

      folly::exception_wrapper exception() && { return std::move(ew_); }

      static void handle(
          HandlerCallbackBase& callback, folly::exception_wrapper&& ew) {
        callback.doExceptionWrapped(std::move(ew));
      }

     private:
      folly::exception_wrapper ew_;
    };
    handleExceptionAndExecuteServiceInterceptors(
        ExceptionHandler(std::move(ew)));
  }

  // Warning: just like "throw ex", this captures the STATIC type of ex, not
  // the dynamic type.  If you need the dynamic type, then either you should
  // be using exception_wrapper instead of a reference to a base exception
  // type, or your exception hierarchy should be equipped to throw
  // polymorphically, see //
  // http://www.parashift.com/c++-faq/throwing-polymorphically.html
  template <class Exception>
  void exception(const Exception& ex) {
    exception(folly::make_exception_wrapper<Exception>(ex));
  }

  void appOverloadedException(const std::string& message) {
    exception(TrustedServerException::appOverloadError(message));
  }

  folly::EventBase* getEventBase();

  [[deprecated("use getHandlerExecutor()")]] folly::Executor*
  getThreadManager();

  [[deprecated("use getHandlerExecutor()")]] concurrency::ThreadManager*
  getThreadManager_deprecated();

  bool isResourcePoolEnabled();

  folly::Executor* getHandlerExecutor();

  [[deprecated("Replaced by getRequestContext")]] Cpp2RequestContext*
  getConnectionContext() {
    return reqCtx_;
  }

  Cpp2RequestContext* getRequestContext() { return reqCtx_; }

  bool isRequestActive() {
    // If req_ is nullptr probably it is not managed by this HandlerCallback
    // object and we just return true. An example can be found in task 3106731
    return !req_ || req_->isActive();
  }

  ResponseChannelRequest* getRequest() { return req_.get(); }

  const folly::Executor::KeepAlive<>& getInternalKeepAlive();

 protected:
  folly::Optional<uint32_t> checksumIfNeeded(
      LegacySerializedResponse& response);

  folly::Optional<uint32_t> checksumIfNeeded(SerializedResponse& response);

  virtual ResponsePayload transform(ResponsePayload&& response);

#if FOLLY_HAS_COROUTINES
  template <class T>
  void startOnExecutor(folly::coro::Task<T>&& task) {
    folly::Executor::KeepAlive<> executor =
        executor_ ? executor_ : folly::getKeepAliveToken(eb_);
    if (executor.get() == eb_ && eb_->isInEventBaseThread()) {
      // Avoid rescheduling in the common case where result() is called inline
      // on the EB thread where request execution began
      std::move(task).scheduleOn(std::move(executor)).startInlineUnsafe();
    } else {
      std::move(task).scheduleOn(std::move(executor)).start();
    }
  }

  template <class ExceptionHandler>
  static folly::coro::Task<void>
  doInvokeServiceInterceptorsOnResponseWithException(
      Ptr callback, ExceptionHandler handler) {
    folly::exception_wrapper ew = std::move(handler).exception();
    folly::Try<void> onResponseResult = co_await folly::coro::co_awaitTry(
        callback->processServiceInterceptorsOnResponse(ew));
    // When both user code and ServiceInterceptor::onResponse have exceptions,
    // the ServiceInterceptor wins. This is because
    // ServiceInterceptor::onResponse has access to the user-thrown exception
    // and can choose to either swallow it or not.
    if (onResponseResult.hasException()) {
      ExceptionHandler::handle(
          *callback, std::move(onResponseResult).exception());
    } else {
      ExceptionHandler::handle(*callback, std::move(ew));
    }
  }
#endif // FOLLY_HAS_COROUTINES

  template <class ExceptionHandler>
  void handleExceptionAndExecuteServiceInterceptors(ExceptionHandler handler) {
#if FOLLY_HAS_COROUTINES
    if (!shouldProcessServiceInterceptorsOnResponse()) {
      ExceptionHandler::handle(*this, std::move(handler).exception());
      return;
    }
    startOnExecutor(doInvokeServiceInterceptorsOnResponseWithException(
        sharedFromThis(), std::move(handler)));
#else
    ExceptionHandler::handle(*this, std::move(handler).exception());
#endif // FOLLY_HAS_COROUTINES
  }

  // Can be called from IO or TM thread
  virtual void doException(std::exception_ptr ex) {
    doExceptionWrapped(folly::exception_wrapper(ex));
  }

  virtual void doExceptionWrapped(folly::exception_wrapper ew);

  template <typename F, typename T>
  void callExceptionInEventBaseThread(F&& f, T&& ex);

  template <typename Reply, typename... A>
  void putMessageInReplyQueue(std::in_place_type_t<Reply> tag, A&&... a);

  void sendReply(SerializedResponse response);
  void sendReply(ResponseAndServerStreamFactory&& responseAndStream);

  bool fulfillTilePromise(std::unique_ptr<Tile> ptr);
  void breakTilePromise();

#if FOLLY_HAS_COROUTINES
  bool shouldProcessServiceInterceptorsOnRequest() const noexcept;
  friend bool detail::shouldProcessServiceInterceptorsOnRequest(
      HandlerCallbackBase&) noexcept;

  bool shouldProcessServiceInterceptorsOnResponse() const noexcept;

  folly::coro::Task<void> processServiceInterceptorsOnRequest(
      detail::ServiceInterceptorOnRequestArguments arguments);
  folly::coro::Task<void> processServiceInterceptorsOnResponse(
      detail::ServiceInterceptorOnResponseResult resultOrActiveException);

  friend folly::coro::Task<void> detail::processServiceInterceptorsOnRequest(
      HandlerCallbackBase&,
      detail::ServiceInterceptorOnRequestArguments arguments);
#endif // FOLLY_HAS_COROUTINES

#if !FOLLY_HAS_COROUTINES
  [[noreturn]]
#endif
  void sendReply(
      [[maybe_unused]] std::pair<
          apache::thrift::SerializedResponse,
          apache::thrift::detail::SinkConsumerImpl>&& responseAndSinkConsumer);

  // Required for this call
  ResponseChannelRequest::UniquePtr req_;
  ContextStack::UniquePtr ctx_;
  TilePtr interaction_;

  MethodNameInfo methodNameInfo_;

  // May be null in a oneway call
  exnw_ptr ewp_;

  // Useful pointers, so handler doesn't need to have a pointer to the server
  folly::EventBase* eb_{nullptr};
  folly::Executor::KeepAlive<> executor_{};
  Cpp2RequestContext* reqCtx_{nullptr};

  int32_t protoSeqId_;

  RequestCompletionCallback* notifyRequestPile_{nullptr};
  RequestCompletionCallback* notifyConcurrencyController_{nullptr};
  ServerRequestData requestData_;
};

class HandlerCallbackOneWay : public HandlerCallbackBase {
 public:
  using Ptr =
      util::IntrusiveSharedPtr<HandlerCallbackOneWay, IntrusiveSharedPtrAccess>;
  using HandlerCallbackBase::HandlerCallbackBase;

 private:
#if FOLLY_HAS_COROUTINES
  static folly::coro::Task<void> doInvokeServiceInterceptorsOnResponse(
      Ptr callback);
#endif // FOLLY_HAS_COROUTINES

  Ptr sharedFromThis() noexcept {
    // Constructing from raw pointer is safe in this case because
    // `this` is guaranteed to be alive while the current
    // function is executing.
    return Ptr(typename Ptr::UnsafelyFromRawPointer(), this);
  }

 public:
  void done() noexcept;
  void complete(folly::Try<folly::Unit>&& r) noexcept;

  class CompletionGuard {
   public:
    explicit CompletionGuard(Ptr&& callback) noexcept
        : callback_(std::move(callback)) {}
    CompletionGuard(CompletionGuard&& other) noexcept
        : callback_(other.release()) {}
    CompletionGuard& operator=(CompletionGuard&& other) noexcept {
      callback_ = other.release();
      return *this;
    }

    ~CompletionGuard() noexcept {
      if (callback_ == nullptr) {
        return;
      }
      if (auto ex = folly::current_exception()) {
        callback_->exception(std::move(ex));
      } else {
        callback_->done();
      }
    }

    Ptr release() noexcept { return std::exchange(callback_, nullptr); }

   private:
    Ptr callback_;
  };
};

template <class T>
using HandlerCallbackPtr = util::IntrusiveSharedPtr<
    HandlerCallback<T>,
    HandlerCallbackBase::IntrusiveSharedPtrAccess>;

template <typename T>
class HandlerCallback : public HandlerCallbackBase {
  using Helper = apache::thrift::detail::HandlerCallbackHelper<T>;
  using InnerType = typename Helper::InnerType;
  using InputType = typename Helper::InputType;
  using cob_ptr = typename Helper::CobPtr;

 public:
  using Ptr = HandlerCallbackPtr<T>;
  using ResultType = std::decay_t<typename Helper::InputType>;

 private:
  Ptr sharedFromThis() {
    // Constructing from raw pointer is safe in this case because
    // `this` is guaranteed to be alive while the current
    // function is executing.
    return Ptr(typename Ptr::UnsafelyFromRawPointer(), this);
  }

 public:
  HandlerCallback() : cp_(nullptr) {}

  HandlerCallback(
      ResponseChannelRequest::UniquePtr req,
      ContextStack::UniquePtr ctx,
      MethodNameInfo methodNameInfo,
      cob_ptr cp,
      exnw_ptr ewp,
      int32_t protoSeqId,
      folly::EventBase* eb,
      concurrency::ThreadManager* tm,
      Cpp2RequestContext* reqCtx,
      TilePtr&& interaction = {});

  HandlerCallback(
      ResponseChannelRequest::UniquePtr req,
      ContextStack::UniquePtr ctx,
      MethodNameInfo methodNameInfo,
      cob_ptr cp,
      exnw_ptr ewp,
      int32_t protoSeqId,
      folly::EventBase* eb,
      folly::Executor::KeepAlive<> executor,
      Cpp2RequestContext* reqCtx,
      RequestCompletionCallback* notifyRequestPile,
      RequestCompletionCallback* notifyConcurrencyController,
      ServerRequestData requestData,
      TilePtr&& interaction = {});

#if FOLLY_HAS_COROUTINES
  static folly::coro::Task<void> doInvokeServiceInterceptorsOnResponse(
      Ptr callback, std::decay_t<InputType> result) {
    folly::Try<void> onResponseResult = co_await folly::coro::co_awaitTry(
        callback->processServiceInterceptorsOnResponse(
            apache::thrift::util::TypeErasedRef::of<InnerType>(result)));
    if (onResponseResult.hasException()) {
      callback->doException(onResponseResult.exception().to_exception_ptr());
    } else {
      callback->doResult(std::move(result));
    }
  }
#endif

  void result(InnerType r) {
#if FOLLY_HAS_COROUTINES
    if (!shouldProcessServiceInterceptorsOnResponse()) {
      // Some service code (especially unit tests) assume that doResult() is
      // called synchronously within a result() call. This check exists simply
      // for backwards compatibility with those services. As an added bonus, we
      // get to avoid allocating a coroutine frame + Future core in the case
      // where they will be unused.
      doResult(std::forward<InputType>(r));
    } else {
      startOnExecutor(doInvokeServiceInterceptorsOnResponse(
          sharedFromThis(), std::decay_t<InputType>(std::move(r))));
    }
#else
    doResult(std::forward<InputType>(r));
#endif // FOLLY_HAS_COROUTINES
  }
  [[deprecated("Pass the inner value directly to result()")]] void result(
      std::unique_ptr<ResultType> r);

  void complete(folly::Try<T>&& r);

 protected:
  virtual void doResult(InputType r);

  cob_ptr cp_;
};

template <>
class HandlerCallback<void> : public HandlerCallbackBase {
  using cob_ptr = SerializedResponse (*)(ContextStack*);

 public:
  using Ptr = HandlerCallbackPtr<void>;
  using ResultType = void;

 private:
  Ptr sharedFromThis() {
    // Constructing from raw pointer is safe in this case because
    // `this` is guaranteed to be alive while the current
    // function is executing.
    return Ptr(typename Ptr::UnsafelyFromRawPointer(), this);
  }

 public:
  HandlerCallback() : cp_(nullptr) {}

  HandlerCallback(
      ResponseChannelRequest::UniquePtr req,
      ContextStack::UniquePtr ctx,
      MethodNameInfo methodNameInfo,
      cob_ptr cp,
      exnw_ptr ewp,
      int32_t protoSeqId,
      folly::EventBase* eb,
      concurrency::ThreadManager* tm,
      Cpp2RequestContext* reqCtx,
      TilePtr&& interaction = {});

  HandlerCallback(
      ResponseChannelRequest::UniquePtr req,
      ContextStack::UniquePtr ctx,
      MethodNameInfo methodNameInfo,
      cob_ptr cp,
      exnw_ptr ewp,
      int32_t protoSeqId,
      folly::EventBase* eb,
      folly::Executor::KeepAlive<> executor,
      Cpp2RequestContext* reqCtx,
      RequestCompletionCallback* notifyRequestPile,
      RequestCompletionCallback* notifyConcurrencyController,
      ServerRequestData requestData,
      TilePtr&& interaction = {});

#if FOLLY_HAS_COROUTINES
  folly::coro::Task<void> doInvokeServiceInterceptorsOnResponse(Ptr callback) {
    folly::Try<void> onResponseResult = co_await folly::coro::co_awaitTry(
        callback->processServiceInterceptorsOnResponse(
            apache::thrift::util::TypeErasedRef::of<folly::Unit>(folly::unit)));
    if (onResponseResult.hasException()) {
      callback->doException(onResponseResult.exception().to_exception_ptr());
    } else {
      callback->doDone();
    }
  }
#endif // FOLLY_HAS_COROUTINES

  void done() {
#if FOLLY_HAS_COROUTINES
    if (!shouldProcessServiceInterceptorsOnResponse()) {
      // Some service code (especially unit tests) assume that doResult() is
      // called synchronously within a result() call. This check exists simply
      // for backwards compatibility with those services. As an added bonus, we
      // get to avoid allocating a coroutine frame + Future core in the case
      // where they will be unused.
      doDone();
    } else {
      startOnExecutor(doInvokeServiceInterceptorsOnResponse(sharedFromThis()));
    }
#else
    doDone();
#endif // FOLLY_HAS_COROUTINES
  }

  void complete(folly::Try<folly::Unit>&& r);

 protected:
  virtual void doDone();

  cob_ptr cp_;
};

template <typename InteractionIf, typename Response>
struct TileAndResponse {
  std::unique_ptr<InteractionIf> tile;
  Response response;
};
template <typename InteractionIf>
struct TileAndResponse<InteractionIf, void> {
  std::unique_ptr<InteractionIf> tile;
};

template <typename InteractionIf, typename Response>
class HandlerCallback<TileAndResponse<InteractionIf, Response>> final
    : public HandlerCallback<Response> {
 public:
  using Ptr = HandlerCallbackPtr<TileAndResponse<InteractionIf, Response>>;

  void result(TileAndResponse<InteractionIf, Response>&& r) {
    if (this->fulfillTilePromise(std::move(r.tile))) {
      if constexpr (!std::is_void_v<Response>) {
        HandlerCallback<Response>::result(std::move(r.response));
      } else {
        this->done();
      }
    }
  }
  void complete(folly::Try<TileAndResponse<InteractionIf, Response>>&& r) {
    if (r.hasException()) {
      this->exception(std::move(r.exception()));
    } else {
      this->result(std::move(r.value()));
    }
  }

  using HandlerCallback<Response>::HandlerCallback;

  ~HandlerCallback() override {
    if (this->interaction_) {
      this->breakTilePromise();
    }
  }
};

////
// Implementation details
////

template <typename ProtocolIn, typename Args>
void GeneratedAsyncProcessorBase::deserializeRequest(
    Args& args,
    folly::StringPiece methodName,
    const SerializedRequest& serializedRequest,
    ContextStack* c) {
  ProtocolIn iprot;
  iprot.setInput(serializedRequest.buffer.get());
  if (c) {
    c->preRead();
  }
  SerializedMessage smsg;
  smsg.protocolType = iprot.protocolType();
  smsg.buffer = serializedRequest.buffer.get();
  smsg.methodName = methodName;
  if (c) {
    c->onReadData(smsg);
  }
  uint32_t bytes = 0;
  try {
    bytes = apache::thrift::detail::deserializeRequestBody(&iprot, &args);
    iprot.readMessageEnd();
  } catch (const std::exception& ex) {
    throw TrustedServerException::requestParsingError(ex.what());
  } catch (...) {
    throw TrustedServerException::requestParsingError(
        folly::exceptionStr(folly::current_exception()).c_str());
  }
  if (c) {
    c->postRead(nullptr, bytes);
  }
}

template <typename ProtocolIn, typename Args>
void GeneratedAsyncProcessorBase::simpleDeserializeRequest(
    Args& args, const SerializedRequest& serializedRequest) {
  ProtocolIn iprot;
  iprot.setInput(serializedRequest.buffer.get());
  try {
    apache::thrift::detail::deserializeRequestBody(&iprot, &args);
    iprot.readMessageEnd();
  } catch (const std::exception& ex) {
    throw TrustedServerException::requestParsingError(ex.what());
  } catch (...) {
    throw TrustedServerException::requestParsingError(
        folly::exceptionStr(folly::current_exception()).c_str());
  }
}

template <typename Response, typename ProtocolOut, typename Result>
Response GeneratedAsyncProcessorBase::serializeResponseImpl(
    std::string_view method,
    ProtocolOut* prot,
    int32_t protoSeqId,
    ContextStack* ctx,
    const Result& result) {
  folly::IOBufQueue queue(folly::IOBufQueue::cacheChainLength());
  size_t bufSize =
      apache::thrift::detail::serializedResponseBodySizeZC(prot, &result);
  if constexpr (std::is_same_v<Response, LegacySerializedResponse>) {
    bufSize += prot->serializedMessageSize(method);
  }

  // Preallocate small buffer headroom for transports metadata & framing.
  constexpr size_t kHeadroomBytes = 128;
  auto buf = folly::IOBuf::create(kHeadroomBytes + bufSize);
  buf->advance(kHeadroomBytes);
  queue.append(std::move(buf));

  prot->setOutput(&queue, bufSize);
  if (ctx) {
    ctx->preWrite();
  }

  if constexpr (std::is_same_v<Response, LegacySerializedResponse>) {
    prot->writeMessageBegin(method, MessageType::T_REPLY, protoSeqId);
  }
  apache::thrift::detail::serializeResponseBody(prot, &result);
  if constexpr (std::is_same_v<Response, LegacySerializedResponse>) {
    prot->writeMessageEnd();
  }
  if (ctx) {
    SerializedMessage smsg;
    smsg.protocolType = prot->protocolType();
    smsg.methodName = method;
    if constexpr (std::is_same_v<Response, LegacySerializedResponse>) {
      apache::thrift::LegacySerializedResponse legacyResponse(
          queue.front()->clone());
      apache::thrift::SerializedResponse response(
          std::move(legacyResponse), prot->protocolType());

      smsg.buffer = response.buffer.get();
    } else {
      smsg.buffer = queue.front();
    }
    ctx->onWriteData(smsg);
  }
  DCHECK_LE(
      queue.chainLength(),
      static_cast<size_t>(std::numeric_limits<int>::max()));
  if (ctx) {
    ctx->postWrite(folly::to_narrow(queue.chainLength()));
  }
  return Response{queue.move()};
}

template <typename ProtocolOut, typename Result>
LegacySerializedResponse GeneratedAsyncProcessorBase::serializeLegacyResponse(
    std::string_view method,
    ProtocolOut* prot,
    int32_t protoSeqId,
    ContextStack* ctx,
    const Result& result) {
  return serializeResponseImpl<LegacySerializedResponse>(
      method, prot, protoSeqId, ctx, result);
}

template <typename ProtocolOut, typename Result>
SerializedResponse GeneratedAsyncProcessorBase::serializeResponse(
    std::string_view method,
    ProtocolOut* prot,
    ContextStack* ctx,
    const Result& result) {
  return serializeResponseImpl<SerializedResponse>(
      method, prot, 0, ctx, result);
}

template <typename ChildType>
std::unique_ptr<concurrency::Runnable>
GeneratedAsyncProcessorBase::makeEventTaskForRequest(
    ResponseChannelRequest::UniquePtr req,
    SerializedCompressedRequest&& serializedRequest,
    Cpp2RequestContext* ctx,
    folly::Executor::KeepAlive<> executor,
    RpcKind kind,
    ExecuteFunc<ChildType> executeFunc,
    ChildType* childClass,
    Tile* tile) {
  auto task = std::make_unique<RequestTask<ChildType>>(
      std::move(req),
      std::move(serializedRequest),
      std::move(executor),
      ctx,
      kind == RpcKind::SINGLE_REQUEST_NO_RESPONSE,
      childClass,
      executeFunc);
  if (tile) {
    task->setTile(
        {tile,
         ctx->getConnectionContext()
             ->getWorkerContext()
             ->getWorkerEventBase()});
  }
  return task;
}

template <typename ChildType>
void GeneratedAsyncProcessorBase::processInThread(
    ResponseChannelRequest::UniquePtr req,
    SerializedCompressedRequest&& serializedRequest,
    Cpp2RequestContext* ctx,
    folly::EventBase*,
    concurrency::ThreadManager* tm,
    RpcKind kind,
    ExecuteFunc<ChildType> executeFunc,
    ChildType* childClass) {
  Tile* tile = nullptr;
  if (auto interactionId = ctx->getInteractionId()) { // includes create
    try {
      tile = &ctx->getConnectionContext()->getTile(interactionId);
    } catch (const std::out_of_range&) {
      req->sendErrorWrapped(
          TApplicationException(
              "Invalid interaction id " + std::to_string(interactionId)),
          kInteractionIdUnknownErrorCode);
      return;
    }
  }

  auto scope = ctx->getRequestExecutionScope();
  auto task = makeEventTaskForRequest(
      std::move(req),
      std::move(serializedRequest),
      ctx,
      tm ? tm->getKeepAlive(scope, concurrency::ThreadManager::Source::INTERNAL)
         : folly::Executor::KeepAlive{},
      kind,
      executeFunc,
      childClass,
      tile);

  if (tile && tile->maybeEnqueue(std::move(task), scope)) {
    return;
  }

  if (tm) {
    using Source = concurrency::ThreadManager::Source;
    auto source = tile && !ctx->getInteractionCreate()
        ? Source::EXISTING_INTERACTION
        : Source::UPSTREAM;
    tm->getKeepAlive(std::move(scope), source)->add([task = std::move(task)] {
      task->run();
    });
  } else {
    task->run();
  }
}

template <typename F, typename T>
void HandlerCallbackBase::callExceptionInEventBaseThread(F&& f, T&& ex) {
  if (!f) {
    return;
  }
  if (!getEventBase() || getEventBase()->inRunningEventBaseThread()) {
    f(std::exchange(req_, {}), protoSeqId_, ctx_.get(), ex, reqCtx_);
    ctx_.reset();
  } else {
    getEventBase()->runInEventBaseThread([f = std::forward<F>(f),
                                          req = std::move(req_),
                                          protoSeqId = protoSeqId_,
                                          ctx = std::move(ctx_),
                                          ex = std::forward<T>(ex),
                                          reqCtx = reqCtx_,
                                          eb = getEventBase()]() mutable {
      f(std::move(req), protoSeqId, ctx.get(), ex, reqCtx);
    });
  }
}

template <typename Reply, typename... A>
void HandlerCallbackBase::putMessageInReplyQueue(
    std::in_place_type_t<Reply> tag, A&&... a) {
  auto eb = getEventBase();
  if (!eb) {
    Reply(std::forward<A>(a)...)();
    return;
  }
  if constexpr (folly::kIsWindows) {
    // TODO(T88449658): We are seeing performance regression on Windows if we
    // use the reply queue. The exact cause is under investigation. Before it is
    // fixed, we can use the default EventBase queue on Windows for now.
    eb->runInEventBaseThread(
        [reply = Reply(static_cast<A&&>(a)...)]() mutable { reply(); });
  } else {
    getReplyQueue().putMessage(tag, static_cast<A&&>(a)...);
  }
}

template <typename T>
HandlerCallback<T>::HandlerCallback(
    ResponseChannelRequest::UniquePtr req,
    ContextStack::UniquePtr ctx,
    MethodNameInfo methodNameInfo,
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
          std::move(methodNameInfo),
          ewp,
          eb,
          tm,
          reqCtx,
          std::move(interaction)),
      cp_(cp) {
  this->protoSeqId_ = protoSeqId;
}

template <typename T>
HandlerCallback<T>::HandlerCallback(
    ResponseChannelRequest::UniquePtr req,
    ContextStack::UniquePtr ctx,
    MethodNameInfo methodNameInfo,
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
          std::move(methodNameInfo),
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

template <typename T>
void HandlerCallback<T>::result(std::unique_ptr<ResultType> r) {
  r ? result(std::move(*r))
    : exception(TApplicationException(
          TApplicationException::MISSING_RESULT,
          "nullptr yielded from handler"));
}

template <typename T>
void HandlerCallback<T>::complete(folly::Try<T>&& r) {
  maybeNotifyComplete();
  if (r.hasException()) {
    exception(std::move(r.exception()));
  } else {
    result(std::move(r.value()));
  }
}

template <typename T>
void HandlerCallback<T>::doResult(InputType r) {
  assert(cp_ != nullptr);
  auto reply = Helper::call(
      cp_,
      this->ctx_.get(),
      executor_ ? executor_.get() : eb_,
      std::forward<InputType>(r));
  sendReply(std::move(reply));
}

namespace detail {

// template that typedefs type to its argument, unless the argument is a
// unique_ptr<S>, in which case it typedefs type to S.
template <class S>
struct inner_type {
  using type = S;
};
template <class S>
struct inner_type<std::unique_ptr<S>> {
  using type = S;
};

template <typename T>
struct HandlerCallbackHelper {
  using InnerType = typename apache::thrift::detail::inner_type<T>::type;
  using InputType = const InnerType&;
  using CobPtr =
      apache::thrift::SerializedResponse (*)(ContextStack*, InputType);
  static apache::thrift::SerializedResponse call(
      CobPtr cob, ContextStack* ctx, folly::Executor*, InputType input) {
    return cob(ctx, input);
  }
};

template <typename StreamInputType>
struct HandlerCallbackHelperServerStream {
  using InnerType = StreamInputType&&;
  using InputType = StreamInputType&&;
  using CobPtr = ResponseAndServerStreamFactory (*)(
      ContextStack*, folly::Executor::KeepAlive<>, InputType);
  static ResponseAndServerStreamFactory call(
      CobPtr cob, ContextStack* ctx, folly::Executor* ex, InputType input) {
    return cob(ctx, ex, std::move(input));
  }
};

template <typename Response, typename StreamItem>
struct HandlerCallbackHelper<ResponseAndServerStream<Response, StreamItem>>
    : public HandlerCallbackHelperServerStream<
          ResponseAndServerStream<Response, StreamItem>> {};

template <typename StreamItem>
struct HandlerCallbackHelper<ServerStream<StreamItem>>
    : public HandlerCallbackHelperServerStream<ServerStream<StreamItem>> {};

template <typename SinkInputType>
struct HandlerCallbackHelperSink {
  using InnerType = SinkInputType&&;
  using InputType = SinkInputType&&;
  using CobPtr =
      std::pair<apache::thrift::SerializedResponse, SinkConsumerImpl> (*)(
          ContextStack*, InputType, folly::Executor::KeepAlive<>);
  static std::pair<apache::thrift::SerializedResponse, SinkConsumerImpl> call(
      CobPtr cob, ContextStack* ctx, folly::Executor* ex, InputType input) {
    return cob(ctx, std::move(input), ex);
  }
};

template <typename Response, typename SinkElement, typename FinalResponse>
struct HandlerCallbackHelper<
    ResponseAndSinkConsumer<Response, SinkElement, FinalResponse>>
    : public HandlerCallbackHelperSink<
          ResponseAndSinkConsumer<Response, SinkElement, FinalResponse>> {};

template <typename SinkElement, typename FinalResponse>
struct HandlerCallbackHelper<SinkConsumer<SinkElement, FinalResponse>>
    : public HandlerCallbackHelperSink<
          SinkConsumer<SinkElement, FinalResponse>> {};

} // namespace detail

template <typename ChildType>
void RequestTask<ChildType>::run() {
  // Since this request was queued, reset the processBegin
  // time to the actual start time, and not the queue time.
  req_.requestContext()->getTimestamps().processBegin =
      std::chrono::steady_clock::now();
  if (!oneway_ && !req_.request()->getShouldStartProcessing()) {
    apache::thrift::HandlerCallbackBase::releaseRequest(
        apache::thrift::detail::ServerRequestHelper::request(std::move(req_)),
        apache::thrift::detail::ServerRequestHelper::eventBase(req_));
    return;
  }
  (childClass_->*executeFunc_)(std::move(req_));
}

} // namespace apache::thrift
