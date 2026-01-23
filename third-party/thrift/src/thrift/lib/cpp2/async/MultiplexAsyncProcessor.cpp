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

#include <thrift/lib/cpp2/async/MultiplexAsyncProcessor.h>

#include <optional>
#include <string_view>

#include <folly/Overload.h>
#include <folly/container/F14Map.h>
#include <folly/container/F14Set.h>

#include <thrift/lib/cpp2/async/AsyncProcessorHelper.h>
#include <thrift/lib/cpp2/async/processor/ServerRequestHelper.h>
#include <thrift/lib/cpp2/schema/detail/Merge.h>
#include <thrift/lib/cpp2/server/ServerFlags.h>

namespace apache::thrift {

namespace {

std::optional<std::string_view> getInteractionNameFromMethodName(
    std::string_view methodName) {
  // Interaction methods are formatted like "{interaction_name}.{method_name}"
  auto separator = methodName.find('.');
  if (separator == std::string_view::npos) {
    return std::nullopt;
  }
  return methodName.substr(0, separator);
}

struct MetadataImpl final : public AsyncProcessorFactory::MethodMetadata {
  // Index of the AsyncProcessor to route this method to
  std::size_t sourceIndex;
  // The underlying metadata produced by the AsyncProcessorFactory at
  // sourceIndex
  std::shared_ptr<const AsyncProcessorFactory::MethodMetadata> inner;

  explicit MetadataImpl(
      std::size_t sourceIndex,
      std::shared_ptr<const AsyncProcessorFactory::MethodMetadata>&& inner)
      : MethodMetadata(*inner),
        sourceIndex(sourceIndex),
        inner(std::move(inner)) {
    CHECK(this->inner != nullptr);
  }
};

MultiplexAsyncProcessorFactory::CompositionMetadata computeCompositionMetadata(
    const std::vector<std::shared_ptr<AsyncProcessorFactory>>&
        processorFactories) {
  MultiplexAsyncProcessorFactory::CompositionMetadata compositionMetadata;

  AsyncProcessorFactory::MethodMetadataMap knownMethods;
  folly::F14FastSet<std::string> seenInteractions;
  auto addToKnownMethods =
      [&](AsyncProcessorFactory::MethodMetadataMap& metadataMap,
          std::size_t sourceIndex) {
        folly::F14FastSet<std::string_view> newlySeenInteractions;
        for (auto& [name, metadata] : metadataMap) {
          if (auto interactionName = getInteractionNameFromMethodName(name)) {
            if (seenInteractions.contains(*interactionName)) {
              // The interaction defined earlier gets precedence and no methods
              // should be added for it from a later AsyncProcessorFactory. This
              // prevents the latter interaction from being ever created.
              continue;
            }
            newlySeenInteractions.emplace(*interactionName);
          }
          // Note that emplace does NOT replace existing entries - methods
          // from earlier AsyncProcessorFactory's always take precedence.
          knownMethods.emplace(
              name,
              std::make_shared<MetadataImpl>(sourceIndex, std::move(metadata)));
        }
        auto oldSize = seenInteractions.size();
        (void)oldSize;
        seenInteractions.insert(
            newlySeenInteractions.begin(), newlySeenInteractions.end());
        // If we implemented it right, seenInteractions and
        // newlySeenInteractions should be distinct
        DCHECK(
            seenInteractions.size() == oldSize + newlySeenInteractions.size());
      };

  for (std::size_t sourceIndex = 0; sourceIndex < processorFactories.size();
       ++sourceIndex) {
    auto metadataResult =
        processorFactories[sourceIndex]->createMethodMetadata();
    auto* metadataMap =
        std::get_if<AsyncProcessorFactory::MethodMetadataMap>(&metadataResult);
    if (metadataMap == nullptr) {
      // Per our contract, the first wildcard-like processor will swallow up all
      // methods that are not explicitly named in knownMethods by a preceding
      // processor. So, as long as we keep track of the first such processor, we
      // can route all wildcard methods to it in constant time.
      if (auto* wildcardMetadataMap =
              std::get_if<AsyncProcessorFactory::WildcardMethodMetadataMap>(
                  &metadataResult)) {
        addToKnownMethods(wildcardMetadataMap->knownMethods, sourceIndex);
        compositionMetadata.firstWildcardLike =
            MultiplexAsyncProcessorFactory::CompositionMetadata::Wildcard{
                sourceIndex, wildcardMetadataMap->wildcardMetadata};
      }
      break;
    }
    addToKnownMethods(*metadataMap, sourceIndex);
  }

  compositionMetadata.cachedMethodMetadataResult =
      [&]() -> AsyncProcessorFactory::CreateMethodMetadataResult {
    if (compositionMetadata.wildcardIndex().has_value()) {
      if (auto wildcard = std::get_if<
              MultiplexAsyncProcessorFactory::CompositionMetadata::Wildcard>(
              &compositionMetadata.firstWildcardLike)) {
        return AsyncProcessorFactory::WildcardMethodMetadataMap{
            wildcard->metadata, std::move(knownMethods)};
      } else {
        return AsyncProcessorFactory::WildcardMethodMetadataMap{
            std::make_shared<
                const AsyncProcessorFactory::WildcardMethodMetadata>(),
            std::move(knownMethods)};
      }
    }
    return std::move(knownMethods);
  }();

  return compositionMetadata;
}

class MultiplexAsyncProcessor final : public AsyncProcessor {
 public:
  void processSerializedCompressedRequestWithMetadata(
      ResponseChannelRequest::UniquePtr req,
      SerializedCompressedRequest&& serializedRequest,
      const MethodMetadata& untypedMethodMetadata,
      protocol::PROTOCOL_TYPES protocolType,
      Cpp2RequestContext* context,
      folly::EventBase* eb,
      concurrency::ThreadManager* tm) override {
    if (!untypedMethodMetadata.isWildcard()) {
      const auto& methodMetadata =
          AsyncProcessorHelper::expectMetadataOfType<MetadataImpl>(
              untypedMethodMetadata);
      DCHECK(methodMetadata.sourceIndex < processors_.size());
      auto& processor = *processors_[methodMetadata.sourceIndex];
      maybeTrackInteraction(context, processor);
      processor.processSerializedCompressedRequestWithMetadata(
          std::move(req),
          std::move(serializedRequest),
          *methodMetadata.inner,
          protocolType,
          context,
          eb,
          tm);
      return;
    }

    const auto& wildcardMethodMetadata = untypedMethodMetadata;
    folly::variant_match(
        compositionMetadata_.firstWildcardLike,
        [](std::monostate) {
          LOG(FATAL)
              << "Received WildcardMethodMetadata but expected no WildcardMethodMetadataMap was composed";
        },
        [&](const MultiplexAsyncProcessorFactory::CompositionMetadata::Wildcard&
                wildcard) {
          DCHECK(wildcard.index < processors_.size());
          auto& processor = *processors_[wildcard.index];
          maybeTrackInteraction(context, processor);
          processor.processSerializedCompressedRequestWithMetadata(
              std::move(req),
              std::move(serializedRequest),
              wildcardMethodMetadata,
              protocolType,
              context,
              eb,
              tm);
        });
  }

  void processInteraction(ServerRequest&& req) override {
    auto context = req.requestContext();
    auto& untypedMethodMetadata = *req.methodMetadata();
    if (!untypedMethodMetadata.isWildcard()) {
      const auto& methodMetadata =
          AsyncProcessorHelper::expectMetadataOfType<MetadataImpl>(
              untypedMethodMetadata);
      DCHECK(methodMetadata.sourceIndex < processors_.size());
      auto& processor = *processors_[methodMetadata.sourceIndex];
      maybeTrackInteraction(context, processor);
      // Unwrap the metadata and update the processor pointer. When the
      // sub-processor queues the request to a resource pool
      // (GeneratedAsyncProcessorBase.cpp:67), the pool will later execute it
      // by calling AsyncProcessorHelper::executeRequest(), which retrieves the
      // processor from the ServerRequest (AsyncProcessorHelper.cpp:40) and
      // calls processor->executeRequest(request, metadata)
      // (AsyncProcessorHelper.cpp:45). We must ensure both the processor
      // pointer and metadata point to the sub-processor's unwrapped values.
      detail::ServerRequestHelper::setMethodMetadata(
          req, methodMetadata.inner.get());
      detail::ServerRequestHelper::setAsyncProcessor(req, &processor);
      processor.processInteraction(std::move(req));
      return;
    }

    folly::variant_match(
        compositionMetadata_.firstWildcardLike,
        [](std::monostate) {
          LOG(FATAL)
              << "Received WildcardMethodMetadata but expected no WildcardMethodMetadataMap was composed";
        },
        [&](const MultiplexAsyncProcessorFactory::CompositionMetadata::Wildcard&
                wildcard) {
          DCHECK(wildcard.index < processors_.size());
          auto& processor = *processors_[wildcard.index];
          maybeTrackInteraction(context, processor);
          processor.processInteraction(std::move(req));
        });
  }

  void getServiceMetadata(
      metadata::ThriftServiceMetadataResponse& actualResponse) override {
    auto copyMetadataInto =
        [](metadata::ThriftMetadata& dst,
           const metadata::ThriftServiceMetadataResponse& src) {
          // The multiplexed services makes reference to a composed service, so
          // we need to include it in the metadata.
          auto& metadata = *src.metadata();
          dst.enums()->insert(
              metadata.enums()->begin(), metadata.enums()->end());
          dst.structs()->insert(
              metadata.structs()->begin(), metadata.structs()->end());
          dst.exceptions()->insert(
              metadata.exceptions()->begin(), metadata.exceptions()->end());
          dst.services()->insert(
              metadata.services()->begin(), metadata.services()->end());
        };

    for (auto& processor : processors_) {
      metadata::ThriftServiceMetadataResponse response;
      processor->getServiceMetadata(response);
      // Copying gives precedence to earlier inserted entries - this matches
      // the semantics of the processor method delegation.
      copyMetadataInto(*actualResponse.metadata(), response);

      actualResponse.services()->insert(
          actualResponse.services()->end(),
          response.services()->begin(),
          response.services()->end());
    }

    // The underlying AsyncProcessor::getServiceMetadata may not be implemented
    // if using: a service compiled without metadata support; or a custom
    // AsyncProcessor implementation
    if (actualResponse.services()->empty()) {
      return;
    }

    // TODO(praihan): Remove context field from metadata response object
    // There is no "correct" way to set context for MultiplexAsyncProcessor.
    // Our best guess would be the first service, which is likely to be most
    // relevant to the user.
    const auto& defaultServiceContextRef = actualResponse.services()->front();
    actualResponse.context()->service_info() =
        actualResponse.metadata()->services()->at(
            *defaultServiceContextRef.service_name());
    actualResponse.context()->module() = *defaultServiceContextRef.module();
  }

  void terminateInteraction(
      int64_t id,
      Cpp2ConnContext& ctx,
      folly::EventBase& eb) noexcept override {
    AsyncProcessor* processor = nullptr;
    if (auto foundProcessor = folly::get_ptr(inflightInteractions_, id)) {
      processor = *foundProcessor;
    } else {
      // The first processor serves the niche of being the "default" interaction
      // creator. We don't put its IDs in the map.
      processor = defaultInteractionProcessor_;
    }
    if (processor != nullptr) {
      processor->terminateInteraction(id, ctx, eb);
    }
  }

  void destroyAllInteractions(
      Cpp2ConnContext& ctx, folly::EventBase& eb) noexcept override {
    inflightInteractions_.clear();
    for (auto& processor : processors_) {
      processor->destroyAllInteractions(ctx, eb);
    }
  }

  explicit MultiplexAsyncProcessor(
      std::vector<std::unique_ptr<AsyncProcessor>>&& processors,
      const MultiplexAsyncProcessorFactory::CompositionMetadata&
          compositionMetadata)
      : AsyncProcessor(IgnoreGlobalEventHandlers{}),
        processors_(std::move(processors)),
        compositionMetadata_(compositionMetadata) {
    DCHECK(!processors_.empty());
  }

  void executeRequest(
      ServerRequest&& request, const MethodMetadata& methodMetadata) override {
    auto [processor, metadata] = derefProcessor(methodMetadata);
    processor->executeRequest(std::move(request), *metadata);
    return;
  }

  void addEventHandler(
      const std::shared_ptr<TProcessorEventHandler>& eventHandler) override {
    for (auto& processor : processors_) {
      processor->addEventHandler(eventHandler);
    }
  }

 private:
  const std::vector<std::unique_ptr<AsyncProcessor>> processors_;
  const MultiplexAsyncProcessorFactory::CompositionMetadata&
      compositionMetadata_;
  folly::F14FastMap<std::int64_t, AsyncProcessor*> inflightInteractions_;
  // The "default" processor is expected to consume a disproportionately large
  // chunk of the traffic. To save memory, we avoid tracking interactions in the
  // map for the first processor that creates an interaction.
  AsyncProcessor* defaultInteractionProcessor_{nullptr};

  void maybeTrackInteraction(
      Cpp2RequestContext* context, AsyncProcessor& processor) {
    if (auto interactionCreate = context->getInteractionCreate();
        interactionCreate.has_value() &&
        *interactionCreate->interactionId() > 0) {
      if (&processor == defaultInteractionProcessor_) {
        return;
      }
      if (defaultInteractionProcessor_ == nullptr) {
        // The first to create an interaction gets the privilege of being the
        // "default"
        defaultInteractionProcessor_ = &processor;
      } else {
        // Note that when there is a duplicate interaction we do not replace
        // the existing entry. A connection should have unique interaction IDs
        // which means that the underlying processor should fail (and close
        // the connection) - it's safe to ignore the duplicated ID.
        inflightInteractions_.emplace(
            *interactionCreate->interactionId(), &processor);
      }
    }
  }

  std::pair<AsyncProcessor*, const AsyncProcessorFactory::MethodMetadata*>
  derefProcessor(const MethodMetadata& methodMetadata) const {
    if (methodMetadata.isWildcard()) {
      return std::make_pair(
          processors_[compositionMetadata_.wildcardIndex().value()].get(),
          &methodMetadata);
    }
    const auto& multiplexMethodMetadata =
        AsyncProcessorHelper::expectMetadataOfType<MetadataImpl>(
            methodMetadata);
    return std::make_pair(
        processors_[multiplexMethodMetadata.sourceIndex].get(),
        multiplexMethodMetadata.inner.get());
  }
};

} // namespace

MultiplexAsyncProcessorFactory::MultiplexAsyncProcessorFactory(
    std::vector<std::shared_ptr<AsyncProcessorFactory>> processorFactories)
    : processorFactories_(
          flattenProcessorFactories(std::move(processorFactories))),
      compositionMetadata_(computeCompositionMetadata(processorFactories_)) {
  CHECK(!processorFactories_.empty());
}

/* static */
std::vector<std::shared_ptr<AsyncProcessorFactory>>
MultiplexAsyncProcessorFactory::flattenProcessorFactories(
    std::vector<std::shared_ptr<AsyncProcessorFactory>> processorFactories) {
  std::vector<std::shared_ptr<AsyncProcessorFactory>> result;
  for (auto& factory : processorFactories) {
    auto* multiplexFactory =
        dynamic_cast<MultiplexAsyncProcessorFactory*>(factory.get());
    if (multiplexFactory == nullptr) {
      result.emplace_back(std::move(factory));
    } else {
      result.insert(
          result.end(),
          std::make_move_iterator(
              multiplexFactory->processorFactories_.begin()),
          std::make_move_iterator(multiplexFactory->processorFactories_.end()));
    }
  }
  return result;
}

std::optional<schema::DefinitionsSchema>
MultiplexAsyncProcessorFactory::getServiceSchema() {
  std::vector<type::Schema> allSchemas;
  std::set<type::DefinitionKey> allKeys;
  for (auto& processorFactory : processorFactories_) {
    auto schema = processorFactory->getServiceSchema();
    if (schema.has_value()) {
      allSchemas.insert(allSchemas.end(), std::move(schema->schema));
      allKeys.insert(
          std::make_move_iterator(schema->definitions.begin()),
          std::make_move_iterator(schema->definitions.end()));
    }
  }
  schema::DefinitionsSchema result;
  result.schema = schema::detail::mergeSchemas(std::move(allSchemas));
  result.definitions.insert(
      result.definitions.end(),
      std::make_move_iterator(allKeys.begin()),
      std::make_move_iterator(allKeys.end()));
  return result;
}

std::vector<folly::not_null<const syntax_graph::ServiceNode*>>
MultiplexAsyncProcessorFactory::getServiceSchemaNodes() {
  std::vector<folly::not_null<const syntax_graph::ServiceNode*>> result;
  result.reserve(processorFactories_.size());
  for (auto& processorFactory : processorFactories_) {
    auto nodes = processorFactory->getServiceSchemaNodes();
    result.insert(result.end(), nodes.begin(), nodes.end());
  }
  return result;
}

std::unique_ptr<AsyncProcessor> MultiplexAsyncProcessorFactory::getProcessor() {
  return getProcessorWithUnderlyingModifications({} /* modifier */);
}

std::unique_ptr<AsyncProcessor>
MultiplexAsyncProcessorFactory::getProcessorWithUnderlyingModifications(
    folly::FunctionRef<void(AsyncProcessor&)> modifier) {
  std::vector<std::unique_ptr<AsyncProcessor>> processors;
  processors.reserve(processorFactories_.size());
  for (auto& processorFactory : processorFactories_) {
    auto processor = processorFactory->getProcessor();
    if (modifier) {
      modifier(*processor);
    }
    processors.emplace_back(std::move(processor));
  }
  return std::make_unique<MultiplexAsyncProcessor>(
      std::move(processors), compositionMetadata_);
}

AsyncProcessorFactory::CreateMethodMetadataResult
MultiplexAsyncProcessorFactory::createMethodMetadata() {
  return compositionMetadata_.cachedMethodMetadataResult;
}

std::shared_ptr<folly::RequestContext>
MultiplexAsyncProcessorFactory::getBaseContextForRequest(
    const MethodMetadata& untypedMethodMetadata) {
  if (!untypedMethodMetadata.isWildcard()) {
    const auto& methodMetadata =
        AsyncProcessorHelper::expectMetadataOfType<MetadataImpl>(
            untypedMethodMetadata);
    return processorFactories_[methodMetadata.sourceIndex]
        ->getBaseContextForRequest(*methodMetadata.inner);
  }

  const auto& wildcardMethodMetadata = untypedMethodMetadata;
  auto wildcardIndex = compositionMetadata_.wildcardIndex();
  DCHECK(wildcardIndex.has_value())
      << "Received WildcardMethodMetadata but expected no WildcardMethodMetadataMap was composed";
  return processorFactories_[*wildcardIndex]->getBaseContextForRequest(
      wildcardMethodMetadata);
}

SelectPoolResult MultiplexAsyncProcessorFactory::selectResourcePool(
    const ServerRequest& request) const {
  const auto* methodMetadata = request.methodMetadata();
  if (!methodMetadata) {
    return std::monostate{};
  }

  std::size_t index;
  if (methodMetadata->isWildcard()) {
    auto wildcardIndex = compositionMetadata_.wildcardIndex();
    DCHECK(wildcardIndex.has_value())
        << "Received WildcardMethodMetadata but expected no WildcardMethodMetadataMap was composed";
    index = *wildcardIndex;
  } else {
    index = AsyncProcessorHelper::expectMetadataOfType<MetadataImpl>(
                *methodMetadata)
                .sourceIndex;
  }

  return processorFactories_[index]->selectResourcePool(request);
}

std::vector<ServiceHandlerBase*>
MultiplexAsyncProcessorFactory::getServiceHandlers() {
  std::vector<ServiceHandlerBase*> result;
  for (auto& processorFactory : processorFactories_) {
    auto newHandlers = processorFactory->getServiceHandlers();
    result.insert(result.end(), newHandlers.begin(), newHandlers.end());
  }
  return result;
}

std::optional<std::size_t>
MultiplexAsyncProcessorFactory::CompositionMetadata::wildcardIndex() const {
  using Result = std::optional<std::size_t>;
  return folly::variant_match(
      firstWildcardLike,
      [](std::monostate) -> Result { return std::nullopt; },
      [](auto&& wildcard) -> Result { return wildcard.index; });
}

bool MultiplexAsyncProcessorFactory::isThriftGenerated() const {
  return std::all_of(
      processorFactories_.begin(),
      processorFactories_.end(),
      [](const auto& fac) { return fac->isThriftGenerated(); });
}

} // namespace apache::thrift
