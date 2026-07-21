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

#include <thrift/lib/cpp2/server/ThriftCatalogServerInterface.h>

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <folly/container/F14Map.h>
#include <folly/synchronization/DelayedInit.h>
#include <thrift/lib/cpp2/FieldRef.h>
#include <thrift/lib/cpp2/async/MultiplexAsyncProcessor.h>
#include <thrift/lib/cpp2/dynamic/ServiceCatalog.h>
#include <thrift/lib/cpp2/dynamic/ServiceCatalogDigest.h>
#include <thrift/lib/cpp2/dynamic/ServiceDescriptorSerialization.h>
#include <thrift/lib/cpp2/dynamic/TypeSystemDigest.h>
#include <thrift/lib/cpp2/runtime/SchemaRegistry.h>
#include <thrift/lib/cpp2/schema/SyntaxGraph.h>
#include <thrift/lib/thrift/gen-cpp2/ThriftCatalogService.h>

namespace apache::thrift::detail {
namespace {

template <typename Digest>
std::string toDigestBytes(const Digest& digest) {
  return std::string(
      reinterpret_cast<const char*>(digest.data()), digest.size());
}

void mergeCatalog(
    type_system::SerializableServiceCatalog& dst,
    const type_system::SerializableServiceCatalog& src) {
  if (src.types().has_value()) {
    if (!dst.types().has_value()) {
      dst.types() = type_system::SerializableTypeSystem{};
    }
    for (const auto& [uri, type] : *src.types()->types()) {
      dst.types()->types()[uri] = type;
    }
  }
  for (const auto& [uri, interfaceDef] : *src.interfaces()) {
    dst.interfaces()[uri] = interfaceDef;
  }
}

void updateTypeDigest(type_system::SerializableServiceCatalog& catalog) {
  if (catalog.types_ref().has_value()) {
    catalog.typesDigest() = toDigestBytes(
        type_system::TypeSystemHasher{}(can_throw(*catalog.types())));
  }
}

catalog::ThriftCatalogItem makeCatalogItem(
    type_system::SerializableServiceCatalog catalog) {
  updateTypeDigest(catalog);
  catalog::ThriftCatalogItem result;
  result.digest() = toDigestBytes(dynamic::ServiceCatalogHasher{}(catalog));
  result.catalog() = std::move(catalog);
  return result;
}

[[noreturn]] void throwUnavailable() {
  catalog::CatalogUnavailable unavailable;
  unavailable.reason() = "Thrift service catalog is not available";
  throw unavailable;
}

[[noreturn]] void throwNotFound(std::string_view uri) {
  catalog::NotFound notFound;
  notFound.uri() = std::string(uri);
  throw notFound;
}

struct CatalogItems {
  folly::F14FastMap<std::string, catalog::ThriftCatalogItem> descriptors;
  std::optional<catalog::ThriftCatalogItem> catalog;
};

CatalogItems buildCatalogItems(
    const std::vector<std::shared_ptr<AsyncProcessorFactory>>&
        processorFactories) {
  CatalogItems items{};
  MultiplexAsyncProcessorFactory services(processorFactories);
  const auto serviceNodes = services.getServiceSchemaNodes();
  if (serviceNodes.empty()) {
    return items;
  }

  type_system::SerializableServiceCatalog mergedCatalog;
  const dynamic::ServiceCatalog& registryCatalog =
      SchemaRegistry::get().asServiceCatalog();
  for (const auto& serviceNode : serviceNodes) {
    const auto uri = serviceNode->uri();
    if (uri.empty() || items.descriptors.contains(uri)) {
      continue;
    }

    const dynamic::ServiceDescriptor* descriptor =
        registryCatalog.getService(uri);
    if (descriptor == nullptr) {
      continue;
    }

    auto serialized = dynamic::toSerializable(*descriptor, uri);
    mergeCatalog(mergedCatalog, serialized);
    items.descriptors[std::string(uri)] =
        makeCatalogItem(std::move(serialized));
  }

  if (!items.descriptors.empty()) {
    items.catalog = makeCatalogItem(std::move(mergedCatalog));
  }
  return items;
}

class ThriftCatalogServerInterface final
    : public ServiceHandler<catalog::ThriftCatalogService> {
 public:
  explicit ThriftCatalogServerInterface(
      std::vector<std::shared_ptr<AsyncProcessorFactory>> processorFactories)
      : processorFactories_(std::move(processorFactories)) {}

  void sync_getThriftServiceCatalog(
      catalog::ThriftCatalogItem& _return) override {
    const auto& catalogItems = getCatalogItems();
    if (!catalogItems.catalog.has_value()) {
      throwUnavailable();
    }
    _return = *catalogItems.catalog;
  }

  void sync_getThriftServiceCatalogDigest(std::string& _return) override {
    const auto& catalogItems = getCatalogItems();
    if (!catalogItems.catalog.has_value()) {
      throwUnavailable();
    }
    _return = *catalogItems.catalog->digest();
  }

  void sync_getThriftServiceDescriptor(
      catalog::ThriftCatalogItem& _return,
      std::unique_ptr<type_system::Uri> serviceUri) override {
    _return = getDescriptor(serviceUri.get());
  }

  void sync_getThriftServiceDescriptorDigest(
      std::string& _return,
      std::unique_ptr<type_system::Uri> serviceUri) override {
    _return = *getDescriptor(serviceUri.get()).digest();
  }

 private:
  const CatalogItems& getCatalogItems() const {
    return catalogItems_.try_emplace_with([this] {
      auto catalogItems = buildCatalogItems(processorFactories_);
      processorFactories_.clear();
      return catalogItems;
    });
  }

  const catalog::ThriftCatalogItem& getDescriptor(
      const type_system::Uri* serviceUri) const {
    const auto& catalogItems = getCatalogItems();
    if (serviceUri == nullptr) {
      throwNotFound("");
    }
    auto it = catalogItems.descriptors.find(*serviceUri);
    if (it == catalogItems.descriptors.end()) {
      throwNotFound(*serviceUri);
    }
    return it->second;
  }

  mutable std::vector<std::shared_ptr<AsyncProcessorFactory>>
      processorFactories_;
  mutable folly::DelayedInit<CatalogItems> catalogItems_;
};

} // namespace

std::shared_ptr<AsyncProcessorFactory> createThriftCatalogServerInterface(
    const std::vector<std::shared_ptr<AsyncProcessorFactory>>&
        processorFactories) {
  return std::make_shared<ThriftCatalogServerInterface>(processorFactories);
}

} // namespace apache::thrift::detail
