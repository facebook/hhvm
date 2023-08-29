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

#include <mutex>
#include <string>
#include <string_view>
#include <variant>

#include <folly/ExceptionWrapper.h>
#include <folly/Portability.h>
#include <folly/String.h>
#include <folly/Synchronized.h>
#include <folly/Unit.h>
#include <folly/container/F14Map.h>
#include <folly/experimental/PrimaryPtr.h>
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
#include <thrift/lib/cpp2/async/Interaction.h>
#include <thrift/lib/cpp2/async/ReplyInfo.h>
#include <thrift/lib/cpp2/async/ResponseChannel.h>
#include <thrift/lib/cpp2/async/RpcTypes.h>
#include <thrift/lib/cpp2/async/SchemaV1.h>
#include <thrift/lib/cpp2/async/ServerRequestData.h>
#include <thrift/lib/cpp2/async/ServerStream.h>
#include <thrift/lib/cpp2/async/Sink.h>
#include <thrift/lib/cpp2/protocol/Protocol.h>
#include <thrift/lib/cpp2/server/ConcurrencyControllerInterface.h>
#include <thrift/lib/cpp2/server/Cpp2ConnContext.h>
#include <thrift/lib/cpp2/server/IOWorkerContext.h>
#include <thrift/lib/cpp2/server/RequestPileInterface.h>
#include <thrift/lib/cpp2/server/ResourcePoolHandle.h>
#include <thrift/lib/cpp2/util/Checksum.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>
#include <thrift/lib/thrift/gen-cpp2/metadata_types.h>

namespace folly {
namespace coro {
class CancellableAsyncScope;
}
} // namespace folly

namespace apache {
namespace thrift {

class ThriftServer;
class ThriftServerStopController;

namespace detail {
template <typename T>
struct HandlerCallbackHelper;
}

class AsyncProcessor;
class ServiceHandlerBase;
class ServerRequest;
class IResourcePoolAcceptor;

// This contains information about a request that is required in the thrift
// server prior to the AsyncProcessor::executeRequest interface.
struct ServiceRequestInfo {
  bool isSync; // True if this has thread='eb'
  RpcKind rpcKind; // Type of this request
  // The qualified function name is currently an input to TProcessorEventHandler
  // callbacks. We will refactor TProcessorEventHandler to remove the
  // requirement to pass this as a single string. T112104402
  const char* functionName_deprecated; // Qualified function name (includes
                                       // service name)
  std::optional<std::string_view>
      interactionName; // Interaction name if part of an interaction
  concurrency::PRIORITY priority; // Method priority set in the IDL
  std::optional<std::string_view>
      createdInteraction; // The name of the interaction created by the RPC
};

using ServiceRequestInfoMap =
    folly::F14ValueMap<std::string, ServiceRequestInfo>;

// The base class for generated code that contains information about a service.
// Each service generates a subclass of this.
class ServiceInfoHolder {
 public:
  virtual ~ServiceInfoHolder() = default;

  // This function is generated from the thrift IDL.
  virtual const ServiceRequestInfoMap& requestInfoMap() const = 0;
};

/**
 * Descriptor of a Thrift service - its methods and how they should be handled.
 */
class AsyncProcessorFactory {
 public:
#if defined(THRIFT_SCHEMA_AVAILABLE)
  /**
   * Reflects on the current service's methods, associated structs etc. at
   * runtime. This is useful to, for example, a tool that can send requests to a
   * service without knowing its schemata ahead of time.
   *
   * This is analogous to GraphQL introspection
   * (https://graphql.org/learn/introspection/) and can be used to build a tool
   * like GraphiQL.
   */
  virtual std::optional<std::vector<schema::SchemaV1>> getServiceMetadataV1() {
    return {};
  }
#endif
  /**
   * Creates a per-connection processor that will handle requests for this
   * service. The returned AsyncProcessor has an implicit contract with the
   * result of createMethodMetadata() - they are tightly coupled. Typically,
   * both will need to be overridden.
   */
  virtual std::unique_ptr<AsyncProcessor> getProcessor() = 0;
  /**
   * Returns the known list of user-implemented service handlers. For generated
   * services, the AsyncProcessorFactory also serves as a ServiceHandlerBase.
   * However, custom implementations may wrap one or more other
   * AsyncProcessorFactory's, in which case, they MUST return their combined
   * result.
   */
  virtual std::vector<ServiceHandlerBase*> getServiceHandlers() = 0;

  struct WildcardMethodMetadata;

  struct MethodMetadata {
    enum class ExecutorType { UNKNOWN, EVB, ANY };

    enum class InteractionType { UNKNOWN, NONE, INTERACTION_V1 };

    MethodMetadata() = default;

   private:
    explicit MethodMetadata(ExecutorType executor) : executorType(executor) {}

    friend struct WildcardMethodMetadata;

   public:
    MethodMetadata(
        ExecutorType executor,
        InteractionType interaction,
        RpcKind kind,
        concurrency::PRIORITY prio,
        const std::optional<std::string_view> interactName,
        bool createsInteract)
        : executorType(executor),
          interactionType(interaction),
          rpcKind(kind),
          priority(prio),
          interactionName(interactName),
          createsInteraction(createsInteract) {}

   protected:
    MethodMetadata(const MethodMetadata& other)
        : executorType(other.executorType),
          interactionType(other.interactionType),
          rpcKind(other.rpcKind),
          priority(other.priority),
          interactionName(other.interactionName),
          createsInteraction(other.createsInteraction) {}

   public:
    virtual ~MethodMetadata() = default;

    bool isWildcard() const {
      if (auto status = isWildcard_.load(); status != WildcardStatus::UNKNOWN) {
        return status == WildcardStatus::YES;
      }
      auto status = dynamic_cast<const WildcardMethodMetadata*>(this)
          ? WildcardStatus::YES
          : WildcardStatus::NO;
      auto expected = WildcardStatus::UNKNOWN;
      isWildcard_.compare_exchange_strong(expected, status);
      return status == WildcardStatus::YES;
    }

    const ExecutorType executorType{ExecutorType::UNKNOWN};
    const InteractionType interactionType{InteractionType::UNKNOWN};
    const std::optional<RpcKind> rpcKind{};
    const std::optional<concurrency::PRIORITY> priority{};
    const std::optional<std::string_view> interactionName;
    const bool createsInteraction{false};

   private:
    enum class WildcardStatus : std::uint8_t { UNKNOWN, NO, YES };
    mutable folly::relaxed_atomic<WildcardStatus> isWildcard_{
        WildcardStatus::UNKNOWN};
  };

  /**
   * The concrete metadata type that will be passed if createMethodMetadata
   * returns WildcardMethodMetadataMap and the current method is not in its
   * knownMethods. This will carry all the information shared by wildcard
   * methods.
   */
  struct WildcardMethodMetadata final : public MethodMetadata {
    explicit WildcardMethodMetadata(ExecutorType executorType)
        : MethodMetadata(executorType) {}
    WildcardMethodMetadata() : WildcardMethodMetadata(ExecutorType::UNKNOWN) {}
    WildcardMethodMetadata(const WildcardMethodMetadata&) = delete;
    WildcardMethodMetadata& operator=(const WildcardMethodMetadata&) = delete;
  };

  /**
   * A map of method names to some loosely typed metadata that will be
   * passed to AsyncProcessor::processSerializedRequest. The concrete type of
   * the entries in the map is a contract between the AsyncProcessorFactory and
   * the AsyncProcessor returned by getProcessor.
   */
  using MethodMetadataMap =
      folly::F14FastMap<std::string, std::shared_ptr<const MethodMetadata>>;
  /**
   * A marker struct indicating that the AsyncProcessor supports any method, or
   * a list of methods that is not enumerable. This applies to AsyncProcessor
   * implementations such as proxies to external services.
   * The implementation may optionally enumerate a subset of known methods.
   */
  struct WildcardMethodMetadataMap {
    /**
     * Shared metadata that will be used for all methods
     * not present in knownMethods.
     */
    std::shared_ptr<const WildcardMethodMetadata> wildcardMetadata{};
    MethodMetadataMap knownMethods;
  };

  using CreateMethodMetadataResult =
      std::variant<MethodMetadataMap, WildcardMethodMetadataMap>;
  /**
   * This function enumerates the list of methods supported by the
   * AsyncProcessor returned by getProcessor(), if possible. The return value
   * represents one of the following states:
   *   1. This API is supported and there is a static list of known methods.
   *      This applies to all generated AsyncProcessors.
   *   2. This API is supported but the complete set of methods is not known or
   *      is not enumerable (e.g. all method names supported). This applies, for
   *      example, to AsyncProcessors that proxy to external services.
   *
   * If returning (1), Thrift server will lookup the method metadata in the map.
   * If the method name is not found, a not-found error will be sent and
   * getProcessor will not be called. Any metadata passed to the processor will
   * always be a reference from the map.
   *
   * If returning (2), Thrift server will lookup the method metadata in the map.
   * If the method name is not found, Thrift will pass a WildcardMethodMetadata
   * object instead (kWildcardMethodMetadata). Any metadata passed to the
   * processor will always be a reference from the map (or be
   * kWildcardMethodMetadata).
   */
  virtual CreateMethodMetadataResult createMethodMetadata() {
    // For generated APFs, this will lead to fallback to old logic
    // For custom APFs, we shouldn't use this default but owner shoul
    // override this function to provide dedicated executorType of their
    // service
    WildcardMethodMetadataMap wildcardMap;
    wildcardMap.wildcardMetadata = std::make_shared<WildcardMethodMetadata>();
    wildcardMap.knownMethods = {};

    return wildcardMap;
  }

  /**
   * Override to return a pre-initialized RequestContext.
   * Its content will be copied in the RequestContext initialized at
   * the beginning of each thrift request processing.
   *
   * The method metadata (per createMethodMetadata's contract) is also passed
   * in. If createMethodMetadata is unimplemented or the method is not found,
   * then this function will not be called.
   */
  virtual std::shared_ptr<folly::RequestContext> getBaseContextForRequest(
      const MethodMetadata&) {
    return nullptr;
  }

  virtual ~AsyncProcessorFactory() = default;
};

// Returned by resource pool components when a request is rejected.
class ServerRequestRejection {
 public:
  ServerRequestRejection(TApplicationException&& exception)
      : impl_(std::move(exception)) {}

  const TApplicationException& applicationException() const& { return impl_; }

  TApplicationException applicationException() && { return std::move(impl_); }

 private:
  TApplicationException impl_;
};

class AsyncProcessor;

// Returned to choose a resource pool
using SelectPoolResult = std::variant<
    std::monostate, // No opinion (use a reasonable default)
    std::reference_wrapper<const ResourcePoolHandle>, // Use this ResourcePool
    ServerRequestRejection>; // Reject the request with this reason

/**
 * A class that is created once per-connection and handles incoming requests.
 * This is the hand-off point from Thrift's IO threads to user code - the
 * functions here are called on the IO thread.
 *
 * While this is a customization point, its API is not stable. Most services use
 * GeneratedAsyncProcessorBase, which handles scheduling of methods on to the
 * ThreadManager (tm) or executing inline (eb).
 */
class AsyncProcessor : public TProcessorBase {
 public:
  ~AsyncProcessor() override = default;

  // Return the name of the service provided by this AsyncProcessor
  virtual const char* getServiceName();

  using MethodMetadata = AsyncProcessorFactory::MethodMetadata;
  using WildcardMethodMetadata = AsyncProcessorFactory::WildcardMethodMetadata;

  /**
   * Processes one incoming request / method.
   *
   * @param methodMetadata See AsyncProcessorFactory::createMethodMetadata()
   */
  virtual void processSerializedCompressedRequestWithMetadata(
      ResponseChannelRequest::UniquePtr req,
      SerializedCompressedRequest&& serializedRequest,
      const MethodMetadata& methodMetadata,
      protocol::PROTOCOL_TYPES prot_type,
      Cpp2RequestContext* context,
      folly::EventBase* eb,
      concurrency::ThreadManager* tm) = 0;

  /**
   * Reflects on the current service's methods, associated structs etc. at
   * runtime. This is useful to, for example, a tool that can send requests to a
   * service without knowing its schemata ahead of time.
   *
   * This is analogous to GraphQL introspection
   * (https://graphql.org/learn/introspection/) and can be used to build a tool
   * like GraphiQL.
   */
  virtual void getServiceMetadata(metadata::ThriftServiceMetadataResponse&) {}

  virtual void terminateInteraction(
      int64_t id, Cpp2ConnContext& conn, folly::EventBase&) noexcept;
  virtual void destroyAllInteractions(
      Cpp2ConnContext& conn, folly::EventBase&) noexcept;

  virtual void processInteraction(ServerRequest&&) {}

  // This is the main interface we are migrating to. Eventually it should
  // replace all the processSerialized... methods.
  //
  // An AsyncProcessor implementation should call the handler for this request
  // on the thread it this method is called on. It does not need to call any
  // hooks for modules before executing the request.
  virtual void executeRequest(
      ServerRequest&& request,
      const AsyncProcessorFactory::MethodMetadata& methodMetadata);
};

namespace detail {
class ServerRequestHelper;
}

// The ServerRequest is used to hold all the information about a request that we
// need to save when queueing it in order to execute the request later.
//
// In the thrift server this is constructed on the stack on the IO thread and
// only moved into allocated storage if the resource pool that will execute it
// has a queue (request pile associated with it).
//
// We provide various methods to accommodate read only access as well as moving
// out portions of the data.
//
// We keep the accessible interface of a ServerRequest and a const ServerRequest
// as narrow as possible as this type is used in several customization points.
class ServerRequest {
 public:
  // Eventually we won't need a default ctor once there is no path that doesn't
  // use the ServerRequest and resource pools.
  ServerRequest() : serializedRequest_(std::unique_ptr<folly::IOBuf>{}) {}

  ServerRequest(
      ResponseChannelRequest::UniquePtr&& request,
      SerializedCompressedRequest&& serializedRequest,
      Cpp2RequestContext* ctx,
      protocol::PROTOCOL_TYPES protocol,
      std::shared_ptr<folly::RequestContext> follyRequestContext,
      AsyncProcessor* asyncProcessor,
      const AsyncProcessor::MethodMetadata* methodMetadata)
      : request_(std::move(request)),
        serializedRequest_(std::move(serializedRequest)),
        ctx_(ctx),
        protocol_(protocol),
        follyRequestContext_(follyRequestContext),
        asyncProcessor_(asyncProcessor),
        methodMetadata_(methodMetadata) {}

  ServerRequest(const ServerRequest&) = delete;

  ServerRequest& operator=(const ServerRequest&) = delete;

  ServerRequest(ServerRequest&&) = default;

  ServerRequest& operator=(ServerRequest&&) = default;

  // in most cases, the completion callback should be done
  // on the thread where HandlerCallback destructor is run
  // e.g. on CPU thread.
  // This short-cut could make the callback run on different threads
  // e.g. on IO thread pool, which is ok.
  ~ServerRequest() {
    if (notifyRequestPile_) {
      notifyRequestPile_->onRequestFinished(requestData_);
    }

    if (notifyConcurrencyController_) {
      notifyConcurrencyController_->onRequestFinished(requestData_);
    }
  }

  // The public accessors are available to user code that receives the
  // ServerRequest through various customization points.

  const AsyncProcessor::MethodMetadata* methodMetadata() const {
    return methodMetadata_;
  }

  const Cpp2RequestContext* requestContext() const { return ctx_; }

  Cpp2RequestContext* requestContext() { return ctx_; }

  // TODO: T108089128 We should change this to return a ResponseChannelRequest
  // once we change downstream code to accept that instead of the
  // ResponseChannelRequest::UniquePtr&.
  const ResponseChannelRequest::UniquePtr& request() const { return request_; }

  ResponseChannelRequest::UniquePtr& request() { return request_; }

  ServerRequestData& requestData() { return requestData_; }

  const std::shared_ptr<folly::RequestContext>& follyRequestContext() const {
    return follyRequestContext_;
  }

  // Set this if the request pile should be notified (via
  // RequestPileInterfaceo::onRequestFinished) when the request is completed.
  void setRequestPileNotification(RequestCompletionCallback* requestPile) {
    DCHECK(notifyRequestPile_ == nullptr);
    notifyRequestPile_ = requestPile;
  }

  // Set this if the concurrency controller should be notified (via
  // ConcurrencyControllerInterface::onRequestFinished) when the request is
  // completed.
  void setConcurrencyControllerNotification(
      RequestCompletionCallback* concurrencyController) {
    DCHECK(notifyConcurrencyController_ == nullptr);
    notifyConcurrencyController_ = concurrencyController;
  }

 protected:
  using InternalPriority = int8_t;

  // The protected accessors are for use only by the thrift server
  // implementation itself. They are accessed using
  // detail::ServerRequestHelper.

  friend class detail::ServerRequestHelper;

  static AsyncProcessor* asyncProcessor(const ServerRequest& sr) {
    return sr.asyncProcessor_;
  }

  static SerializedCompressedRequest& compressedRequest(ServerRequest& sr) {
    return sr.serializedRequest_;
  }

  static SerializedCompressedRequest compressedRequest(ServerRequest&& sr) {
    return std::move(sr.serializedRequest_);
  }

  static ResponseChannelRequest::UniquePtr request(ServerRequest&& sr) {
    return std::move(sr.request_);
  }

  static folly::EventBase* eventBase(ServerRequest& sr) {
    return sr.ctx_->getConnectionContext()
        ->getWorkerContext()
        ->getWorkerEventBase();
  }

  // The executor is only available once the request has been assigned to
  // a resource pool.
  static folly::Executor::KeepAlive<> executor(ServerRequest& sr) {
    return sr.executor_ ? sr.executor_ : eventBase(sr);
  }

  // Only available once the request has been assigned to
  // a resource pool.
  static IResourcePoolAcceptor* resourcePool(ServerRequest& sr) {
    return sr.resourcePool_ ? sr.resourcePool_ : nullptr;
  }

  static InternalPriority internalPriority(ServerRequest& sr) {
    return sr.priority_;
  }

  static void setExecutor(
      ServerRequest& sr, folly::Executor::KeepAlive<> executor) {
    sr.executor_ = std::move(executor);
  }

  static void setResourcePool(ServerRequest& sr, IResourcePoolAcceptor* rp) {
    sr.resourcePool_ = rp;
  }

  static void setInternalPriority(
      ServerRequest& sr, InternalPriority priority) {
    sr.priority_ = priority;
  }

  static protocol::PROTOCOL_TYPES protocol(ServerRequest& sr) {
    return sr.protocol_;
  }

  static RequestCompletionCallback* moveRequestPileNotification(
      ServerRequest& sr) {
    return std::exchange(sr.notifyRequestPile_, nullptr);
  }

  static RequestCompletionCallback* moveConcurrencyControllerNotification(
      ServerRequest& sr) {
    return std::exchange(sr.notifyConcurrencyController_, nullptr);
  }

  static intptr_t& queueObserverPayload(ServerRequest& sr) {
    return sr.queueObserverPayload_;
  }

 private:
  ResponseChannelRequest::UniquePtr request_;
  SerializedCompressedRequest serializedRequest_;
  folly::Executor::KeepAlive<> executor_{};
  Cpp2RequestContext* ctx_;
  protocol::PROTOCOL_TYPES protocol_;
  std::shared_ptr<folly::RequestContext> follyRequestContext_;
  AsyncProcessor* asyncProcessor_;
  const AsyncProcessor::MethodMetadata* methodMetadata_;
  RequestCompletionCallback* notifyRequestPile_{nullptr};
  RequestCompletionCallback* notifyConcurrencyController_{nullptr};
  ServerRequestData requestData_;
  intptr_t queueObserverPayload_;
  IResourcePoolAcceptor* resourcePool_{nullptr};
  InternalPriority priority_{folly::Executor::LO_PRI};
};

namespace detail {

class ServerRequestHelper : public ServerRequest {
 public:
  using ServerRequest::asyncProcessor;
  using ServerRequest::compressedRequest;
  using ServerRequest::eventBase;
  using ServerRequest::executor;
  using ServerRequest::internalPriority;
  using ServerRequest::moveConcurrencyControllerNotification;
  using ServerRequest::moveRequestPileNotification;
  using ServerRequest::protocol;
  using ServerRequest::queueObserverPayload;
  using ServerRequest::request;
  using ServerRequest::requestContext;
  using ServerRequest::resourcePool;
  using ServerRequest::setExecutor;
  using ServerRequest::setInternalPriority;
  using ServerRequest::setResourcePool;
};

} // namespace detail

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
      const char* method,
      ProtocolOut* prot,
      int32_t protoSeqId,
      ContextStack* ctx,
      const Result& result);

  template <typename ProtocolOut, typename Result>
  static LegacySerializedResponse serializeLegacyResponse(
      const char* method,
      ProtocolOut* prot,
      int32_t protoSeqId,
      ContextStack* ctx,
      const Result& result);

  template <typename ProtocolOut, typename Result>
  static SerializedResponse serializeResponse(
      const char* method,
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
  virtual std::unique_ptr<Tile> createInteractionImpl(const std::string& name);

 public:
  void terminateInteraction(
      int64_t id, Cpp2ConnContext& conn, folly::EventBase&) noexcept final;
  void destroyAllInteractions(
      Cpp2ConnContext& conn, folly::EventBase&) noexcept final;
};

class EventTask : public concurrency::Runnable, public InteractionTask {
 public:
  EventTask(
      ResponseChannelRequest::UniquePtr req,
      SerializedCompressedRequest&& serializedRequest,
      folly::Executor::KeepAlive<> executor,
      Cpp2RequestContext* ctx,
      bool oneway)
      : req_(std::move(req), std::move(serializedRequest), ctx, {}, {}, {}, {}),
        oneway_(oneway) {
    detail::ServerRequestHelper::setExecutor(req_, std::move(executor));
  }

  ~EventTask() override;

  void expired();
  void failWith(folly::exception_wrapper ex, std::string exCode) override;

  void setTile(TilePtr&& tile) override;

  friend class TilePromise;

 protected:
  ServerRequest req_;
  bool oneway_;
};

class ServerRequestTask : public concurrency::Runnable, public InteractionTask {
 public:
  explicit ServerRequestTask(ServerRequest&& req) : req_(std::move(req)) {}
  ~ServerRequestTask() override;

  void failWith(folly::exception_wrapper ex, std::string exCode) override;

  void setTile(TilePtr&& tile) override;

  void run() override {
    // This override exists because these tasks are stored as Runnable in the
    // interaction queues.
    LOG(FATAL) << "Should never be called";
  }

  void acceptIntoResourcePool(int8_t priority) override;

  friend class Tile;
  friend class TilePromise;
  friend class GeneratedAsyncProcessorBase;

 private:
  ServerRequest req_;
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
 * This struct encapsulates the various thrift control information of interest
 * to request handlers; the executor on which we expect them to execute, the
 * Cpp2RequestContext of the incoming request struct, etc.
 */
class RequestParams {
 public:
  RequestParams(
      Cpp2RequestContext* requestContext,
      concurrency::ThreadManager* threadManager,
      folly::EventBase* eventBase,
      folly::Executor* handlerExecutor = nullptr)
      : requestContext_(requestContext),
        threadManager_(threadManager),
        handlerExecutor_(handlerExecutor),
        eventBase_(eventBase) {}
  RequestParams() = default;

  Cpp2RequestContext* getRequestContext() const { return requestContext_; }

  // For cases where caller only needs the folly::Executor* interface.
  // These calls can be replaced with getHandlerExecutor.
  [[deprecated("Use getHandlerExecutor()")]] folly::Executor* getThreadManager()
      const {
    return getHandlerExecutor();
  }

  // For cases where the caller needs the ThreadManager interface. Caller
  // needs to be refactored to replace these calls with getHandlerExecutor.
  [[deprecated("Use getHandlerExecutor()")]] concurrency::ThreadManager*
  getThreadManager_deprecated() const {
    return threadManager_;
  }

  folly::EventBase* getEventBase() const { return eventBase_; }
  folly::Executor* getHandlerExecutor() const {
    if (threadManager_) {
      return threadManager_;
    } else {
      return handlerExecutor_;
    }
  }

 private:
  friend class ServerInterface;

  Cpp2RequestContext* requestContext_{nullptr};
  concurrency::ThreadManager* threadManager_{nullptr};
  folly::Executor* handlerExecutor_{nullptr};
  folly::EventBase* eventBase_{nullptr};
};

/**
 * Base-class for user-implemented service handlers. This serves as a channel
 * user code to be notified by ThriftServer and respond to events (via
 * callbacks).
 */
class ServiceHandlerBase {
 private:
#if FOLLY_HAS_COROUTINES
  class MethodNotImplemented : public std::logic_error {
   public:
    MethodNotImplemented() : std::logic_error("Method not implemented") {}
  };
#endif

 public:
#if FOLLY_HAS_COROUTINES
  virtual folly::coro::Task<void> co_onStartServing() { co_return; }
  virtual folly::coro::Task<void> co_onStopRequested() {
    throw MethodNotImplemented();
  }
#endif

  virtual folly::SemiFuture<folly::Unit> semifuture_onStartServing() {
#if FOLLY_HAS_COROUTINES
    if constexpr (folly::kIsLinux) {
      return co_onStartServing().semi();
    }
#endif
    return folly::makeSemiFuture();
  }

  virtual folly::SemiFuture<folly::Unit> semifuture_onStopRequested() {
#if FOLLY_HAS_COROUTINES
    if constexpr (folly::kIsLinux) {
      // TODO(srir): onStopRequested should be implemented similar to
      // onStartServing
      try {
        return co_onStopRequested().semi();
      } catch (MethodNotImplemented&) {
        // If co_onStopRequested() is not implemented we just return
      }
    }
#endif
    return folly::makeSemiFuture();
  }

  ThriftServer* getServer() { return server_; }
  const ThriftServer* getServer() const { return server_; }
  void attachServer(ThriftServer& server);
  void detachServer();

  /**
   * Asynchronously begins shutting down the Thrift server this handler is
   * attached to.
   *
   * This function is idempotent for the duration of a server lifecycle -- so
   * it's safe to call multiple times (e.g. from folly::AsyncSignalHandler).
   */
  void shutdownServer();

  virtual ~ServiceHandlerBase() = default;

 protected:
#if FOLLY_HAS_COROUTINES
  folly::coro::CancellableAsyncScope* getAsyncScope();
#endif

 private:
  ThriftServer* server_{nullptr};
  folly::Synchronized<
      std::optional<folly::PrimaryPtrRef<ThriftServerStopController>>,
      std::mutex>
      serverStopController_;
};

/**
 * Base-class for generated service handlers. While AsyncProcessorFactory and
 * ServiceHandlerBase are separate layers of abstraction, generated code reuse
 * the same object for both.
 */
class ServerInterface : public virtual AsyncProcessorFactory,
                        public ServiceHandlerBase {
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
        const std::optional<std::string_view> interactionName,
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

 public:
  HandlerCallbackBase() : eb_(nullptr), reqCtx_(nullptr), protoSeqId_(0) {}

  HandlerCallbackBase(
      ResponseChannelRequest::UniquePtr req,
      ContextStack::UniquePtr ctx,
      exnw_ptr ewp,
      folly::EventBase* eb,
      concurrency::ThreadManager* tm,
      Cpp2RequestContext* reqCtx,
      TilePtr&& interaction = {})
      : req_(std::move(req)),
        ctx_(std::move(ctx)),
        interaction_(std::move(interaction)),
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

  void exception(std::exception_ptr ex) { doException(ex); }

  void exception(folly::exception_wrapper ew) { doExceptionWrapped(ew); }

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

#if !FOLLY_HAS_COROUTINES
  [[noreturn]]
#endif
  void
  sendReply(
      FOLLY_MAYBE_UNUSED std::pair<
          apache::thrift::SerializedResponse,
          apache::thrift::detail::SinkConsumerImpl>&& responseAndSinkConsumer);

  // Required for this call
  ResponseChannelRequest::UniquePtr req_;
  ContextStack::UniquePtr ctx_;
  TilePtr interaction_;

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

template <typename T>
class HandlerCallback : public HandlerCallbackBase {
  using Helper = apache::thrift::detail::HandlerCallbackHelper<T>;
  using InputType = typename Helper::InputType;
  using cob_ptr = typename Helper::CobPtr;

 public:
  using ResultType = std::decay_t<typename Helper::InputType>;

 public:
  HandlerCallback() : cp_(nullptr) {}

  HandlerCallback(
      ResponseChannelRequest::UniquePtr req,
      ContextStack::UniquePtr ctx,
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

  void result(InputType r) { doResult(std::forward<InputType>(r)); }
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
  using ResultType = void;

  HandlerCallback() : cp_(nullptr) {}

  HandlerCallback(
      ResponseChannelRequest::UniquePtr req,
      ContextStack::UniquePtr ctx,
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

  void done() { doDone(); }

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
        folly::exceptionStr(std::current_exception()).c_str());
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
        folly::exceptionStr(std::current_exception()).c_str());
  }
}

template <typename Response, typename ProtocolOut, typename Result>
Response GeneratedAsyncProcessorBase::serializeResponseImpl(
    const char* method,
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
    const char* method,
    ProtocolOut* prot,
    int32_t protoSeqId,
    ContextStack* ctx,
    const Result& result) {
  return serializeResponseImpl<LegacySerializedResponse>(
      method, prot, protoSeqId, ctx, result);
}

template <typename ProtocolOut, typename Result>
SerializedResponse GeneratedAsyncProcessorBase::serializeResponse(
    const char* method,
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

template <typename T>
HandlerCallback<T>::HandlerCallback(
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

template <typename T>
void HandlerCallback<T>::result(std::unique_ptr<ResultType> r) {
  r ? doResult(std::move(*r))
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
  this->ctx_.reset();
  sendReply(std::move(reply));
}

namespace detail {

// template that typedefs type to its argument, unless the argument is a
// unique_ptr<S>, in which case it typedefs type to S.
template <class S>
struct inner_type {
  typedef S type;
};
template <class S>
struct inner_type<std::unique_ptr<S>> {
  typedef S type;
};

template <typename T>
struct HandlerCallbackHelper {
  using InputType = const typename apache::thrift::detail::inner_type<T>::type&;
  using CobPtr =
      apache::thrift::SerializedResponse (*)(ContextStack*, InputType);
  static apache::thrift::SerializedResponse call(
      CobPtr cob, ContextStack* ctx, folly::Executor*, InputType input) {
    return cob(ctx, input);
  }
};

template <typename StreamInputType>
struct HandlerCallbackHelperServerStream {
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
  if (req_.requestContext()->getTimestamps().getSamplingStatus().isEnabled()) {
    // Since this request was queued, reset the processBegin
    // time to the actual start time, and not the queue time.
    req_.requestContext()->getTimestamps().processBegin =
        std::chrono::steady_clock::now();
  }
  if (!oneway_ && !req_.request()->getShouldStartProcessing()) {
    apache::thrift::HandlerCallbackBase::releaseRequest(
        apache::thrift::detail::ServerRequestHelper::request(std::move(req_)),
        apache::thrift::detail::ServerRequestHelper::eventBase(req_));
    return;
  }
  (childClass_->*executeFunc_)(std::move(req_));
}

} // namespace thrift
} // namespace apache
