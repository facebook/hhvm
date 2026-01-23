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

#include <cstdint>
#include <memory>
#include <optional>
#include <variant>
#include <vector>

#include <folly/Function.h>

#include <thrift/lib/cpp2/async/AsyncProcessor.h>

namespace apache::thrift {

/**
 * AsyncProcessorFactory implementation that composes multiple services. The
 * composition is not commutative.
 *
 * The magic is achieved using information about the service exposed by
 * AsyncProcessorFactory::createMethodMetadata().
 *
 * Given that each service can return one of (MethodMetadataMap,
 * WildcardMethodMetadataMap), we define the composition of services (A, B) by
 * the following matrix:
 *
 * (MethodMetadataMap, MethodMetadataMap)
 *    A's methods take precedence. Their knownMethods are merged.
 *
 * (MethodMetadataMap, WildcardMethodMetadataMap)
 *    A's methods take precedence. All other methods are routed to B.
 *
 * (WildcardMethodMetadataMap, <any>)
 *    All methods routed to A. B is unused.
 *
 * For a list of services, the semantics are the same as left-folding using
 * the logic above.
 *
 * Notice that these semantics imply that an AsyncProcessorFactory that returns
 * WildcardMethodMetadataMap MUST be the last item of the list - all services
 * after it cannot logically be reached. However, it is not an error to do so.
 *
 * If createMethodMetadata() is unimplemented, then it is treated very similarly
 * to WildcardMethodMetadataMap except that processSerializedCompressedRequest
 * is called of processSerializedCompressedRequestWithMetadata.
 */
class MultiplexAsyncProcessorFactory final : public AsyncProcessorFactory {
 public:
  explicit MultiplexAsyncProcessorFactory(
      std::vector<std::shared_ptr<AsyncProcessorFactory>> processorFactories);
  std::optional<schema::DefinitionsSchema> getServiceSchema() override;
  /**
   * Returns descriptors of the services that this AsyncProcessorFactory serves.
   * Nodes are in the same order as the underlying AsyncProcessorFactories.
   */
  std::vector<folly::not_null<const syntax_graph::ServiceNode*>>
  getServiceSchemaNodes() override;
  std::unique_ptr<AsyncProcessor> getProcessor() override;
  CreateMethodMetadataResult createMethodMetadata() override;

  /**
   * Applies the provided modifier func to each underlying AsyncProcessor before
   * multiplexing them.
   */
  std::unique_ptr<AsyncProcessor> getProcessorWithUnderlyingModifications(
      folly::FunctionRef<void(AsyncProcessor&)> modifier);

  std::shared_ptr<folly::RequestContext> getBaseContextForRequest(
      const MethodMetadata&) override;

  SelectPoolResult selectResourcePool(
      const ServerRequest& request) const override;

  std::vector<ServiceHandlerBase*> getServiceHandlers() override;

  /**
   * Metadata about the chain of AsyncProcessorFactory's - computed once.
   */
  struct CompositionMetadata {
    // createMethodMetadata() returned WildcardMethodMetadata
    struct Wildcard {
      std::size_t index;
      std::shared_ptr<const WildcardMethodMetadata> metadata;
    };

    // The first occurrence of a wildcard-like AsyncProcessorFactory swallows up
    // all requests left unhandled by all previous factories.
    std::variant<std::monostate, Wildcard> firstWildcardLike;

    // The return value of createMethodMetadata()
    CreateMethodMetadataResult cachedMethodMetadataResult;

    std::optional<std::size_t> wildcardIndex() const;
  };

  bool isThriftGenerated() const final;

 private:
  const std::vector<std::shared_ptr<AsyncProcessorFactory>> processorFactories_;
  const CompositionMetadata compositionMetadata_;

  static std::vector<std::shared_ptr<AsyncProcessorFactory>>
      flattenProcessorFactories(
          std::vector<std::shared_ptr<AsyncProcessorFactory>>);
};

} // namespace apache::thrift
