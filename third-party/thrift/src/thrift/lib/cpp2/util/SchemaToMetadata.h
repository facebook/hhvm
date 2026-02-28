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
#include <vector>

#include <folly/portability/GFlags.h>
#include <thrift/lib/cpp2/runtime/SchemaRegistry.h>
#include <thrift/lib/thrift/gen-cpp2/metadata_types.h>

FOLLY_GFLAGS_DECLARE_bool(thrift_enable_schema_to_metadata_conversion);

namespace apache::thrift::detail::md {

template <class T>
struct GenMetadataResult {
  const bool preExists;
  T& metadata;
};

std::mutex& schemaRegistryMutex();

template <class T>
const auto& getNodeWithLock() {
  std::lock_guard lock(schemaRegistryMutex());
  return SchemaRegistry::get().getNode<T>();
}

template <class T>
const auto& getDefinitionNodeWithLock() {
  std::lock_guard lock(schemaRegistryMutex());
  return SchemaRegistry::get().getDefinitionNode<T>();
}

// Generate metadata of `node` inside `md`, return the generated metadata.
GenMetadataResult<metadata::ThriftEnum> genEnumMetadata(
    metadata::ThriftMetadata& md, const syntax_graph::EnumNode& node);

template <class E>
auto genEnumMetadata(metadata::ThriftMetadata& md) {
  return genEnumMetadata(md, getNodeWithLock<E>());
}

GenMetadataResult<metadata::ThriftStruct> genStructMetadata(
    metadata::ThriftMetadata& md, const syntax_graph::StructuredNode& node);

template <class T>
auto genStructMetadata(metadata::ThriftMetadata& md) {
  return genStructMetadata(md, getNodeWithLock<T>());
}

GenMetadataResult<metadata::ThriftException> genExceptionMetadata(
    metadata::ThriftMetadata& md, const syntax_graph::ExceptionNode& node);

template <class T>
auto genExceptionMetadata(metadata::ThriftMetadata& md) {
  return genExceptionMetadata(md, getNodeWithLock<T>());
}

metadata::ThriftService genServiceMetadata(
    const syntax_graph::ServiceNode& node, metadata::ThriftMetadata& md);

template <class Tag>
metadata::ThriftService genServiceMetadata(metadata::ThriftMetadata& md) {
  return genServiceMetadata(getDefinitionNodeWithLock<Tag>().asService(), md);
}

const metadata::ThriftServiceContextRef* genServiceMetadataRecurse(
    const syntax_graph::ServiceNode& node,
    metadata::ThriftMetadata& metadata,
    std::vector<metadata::ThriftServiceContextRef>& services);

// An implementation of metadata that should have the same observable behavior
// as ServiceMetadata::genRecurse(...)
template <class Service>
const metadata::ThriftServiceContextRef* genServiceMetadataRecurse(
    metadata::ThriftMetadata& metadata,
    std::vector<metadata::ThriftServiceContextRef>& services) {
  return genServiceMetadataRecurse(
      getDefinitionNodeWithLock<Service>().asService(), metadata, services);
}

// An implementation of metadata that should have the same observable behavior
// as ServiceMetadata::gen(...)
void genServiceMetadataResponse(
    const syntax_graph::ServiceNode& node,
    metadata::ThriftServiceMetadataResponse& response);

template <class Service>
void genServiceMetadataResponse(
    metadata::ThriftServiceMetadataResponse& response) {
  return genServiceMetadataResponse(
      getDefinitionNodeWithLock<Service>().asService(), response);
}

} // namespace apache::thrift::detail::md
