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

#include <variant>

#include <folly/io/async/Request.h>
#include <folly/memory/not_null.h>
#include <thrift/lib/cpp/TApplicationException.h>
#include <thrift/lib/cpp/concurrency/Thread.h>
#include <thrift/lib/cpp2/schema/SchemaV1.h>
#include <thrift/lib/cpp2/server/ResourcePoolHandle.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift {

class AsyncProcessor;
class ServiceHandlerBase;
class ServerRequest;
namespace syntax_graph {
class FunctionNode;
class ServiceNode;
} // namespace syntax_graph

// Returned by resource pool components when a request is rejected.
class ServerRequestRejection {
 public:
  explicit ServerRequestRejection(TApplicationException&& exception)
      : impl_(std::move(exception)) {}

  const TApplicationException& applicationException() const& { return impl_; }

  TApplicationException applicationException() && { return std::move(impl_); }

 private:
  TApplicationException impl_;
};

// Returned to choose a resource pool
using SelectPoolResult = std::variant<
    std::monostate, // No opinion (use a reasonable default)
    std::reference_wrapper<const ResourcePoolHandle>, // Use this ResourcePool
    ServerRequestRejection>; // Reject the request with this reason

/**
 * Descriptor of a Thrift service - its methods and how they should be handled.
 */
class AsyncProcessorFactory {
 public:
  /**
   * Reflects on the current service's methods, associated structs etc. at
   * runtime. This is useful to, for example, a tool that can send requests to a
   * service without knowing its schemata ahead of time.
   *
   * This is analogous to GraphQL introspection
   * (https://graphql.org/learn/introspection/) and can be used to build a tool
   * like GraphiQL.
   */
  virtual std::optional<schema::DefinitionsSchema> getServiceSchema() {
    return {};
  }

  /**
   * Returns descriptors of the services that this AsyncProcessorFactory serves.
   * Generated handler types will return a single-element vector.
   * Multiple elements are only expected from MultiplexAsyncProcessor.
   */
  virtual std::vector<folly::not_null<const syntax_graph::ServiceNode*>>
  getServiceSchemaNodes() {
    return {};
  }
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
        const std::optional<std::string>& interactName,
        bool createsInteract,
        const syntax_graph::FunctionNode* fnNode = nullptr)
        : executorType(executor),
          interactionType(interaction),
          rpcKind(kind),
          priority(prio),
          interactionName(interactName),
          createsInteraction(createsInteract),
          functionNode(fnNode) {}

   protected:
    MethodMetadata(const MethodMetadata& other)
        : executorType(other.executorType),
          interactionType(other.interactionType),
          rpcKind(other.rpcKind),
          priority(other.priority),
          interactionName(other.interactionName),
          createsInteraction(other.createsInteraction),
          functionNode(other.functionNode) {}

    std::string describeFields() const;

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

    virtual std::string describe() const;

    const ExecutorType executorType{ExecutorType::UNKNOWN};
    const InteractionType interactionType{InteractionType::UNKNOWN};
    const std::optional<RpcKind> rpcKind{};
    const std::optional<concurrency::PRIORITY> priority{};
    const std::optional<std::string> interactionName{};
    const bool createsInteraction{false};
    const syntax_graph::FunctionNode* functionNode{nullptr};

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

    std::string describe() const override;
  };

  /**
   * A map of method names to some loosely typed metadata that will be
   * passed to AsyncProcessor::processSerializedRequest. The concrete type of
   * the entries in the map is a contract between the AsyncProcessorFactory and
   * the AsyncProcessor returned by getProcessor.
   */
  using MethodMetadataMap =
      folly::F14FastMap<std::string, std::shared_ptr<const MethodMetadata>>;

  static std::string describe(const MethodMetadataMap&);

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

  static std::string describe(const WildcardMethodMetadataMap&);

  using CreateMethodMetadataResult =
      std::variant<MethodMetadataMap, WildcardMethodMetadataMap>;

  static std::string describe(const CreateMethodMetadataResult&);

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

  /**
   * Selects the ResourcePool to be used for requests handled by the processor
   * returned by getProcessor.
   *
   * The default implementation of this method has no effect (i.e., default
   * ResourcePool selection process will be used).
   *
   * NOTE: Users of this API must ensure that ResourcePool(s) corresponding to
   * the ResourcePoolHandle(s) returned by this method exist in the
   * ResourcePoolSet of the ThriftServer in which the AsyncProcessorFactory is
   * installed. Failure to do so will result in undefined behavior.
   */
  virtual SelectPoolResult selectResourcePool(const ServerRequest&) const {
    return std::monostate{};
  }

  virtual ~AsyncProcessorFactory() = default;

  virtual bool isThriftGenerated() const { return false; }
};

} // namespace apache::thrift
