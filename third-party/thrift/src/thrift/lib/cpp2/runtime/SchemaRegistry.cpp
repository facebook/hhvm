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

#include <thrift/lib/cpp2/runtime/SchemaRegistry.h>

#include <folly/Indestructible.h>
#include <thrift/lib/cpp2/schema/SyntaxGraph.h>
#include <thrift/lib/cpp2/schema/detail/Merge.h>

#if __has_include(<thrift/facebook/schema/omnibus_schema_header.h>)
#include <thrift/facebook/schema/omnibus_schema_header.h>
#else
namespace {
constexpr std::string_view __fbthrift_omnibus_schema;
}
#endif

namespace apache::thrift {

SchemaRegistry::SchemaRegistry(BaseSchemaRegistry& base) : base_(base) {
  auto resolver = std::make_unique<syntax_graph::detail::IncrementalResolver>();
  resolver_ = resolver.get();
  syntaxGraph_ =
      std::make_unique<syntax_graph::SyntaxGraph>(std::move(resolver));
}
SchemaRegistry::~SchemaRegistry() = default;

SchemaRegistry& SchemaRegistry::get() {
  static folly::Indestructible<SchemaRegistry> self(BaseSchemaRegistry::get());
  static bool addOmnibusSchema = [&] {
    // This loads the omnibus schema bundle. The URI we pass in is only used to
    // check that some definition from the bundle loaded successfully via the
    // return value of this call.
    return self->resolver_->getDefinitionNodeByUri(
        "facebook.com/thrift/type/Any",
        type::ProgramId{},
        {{__fbthrift_omnibus_schema}});
  }();
  assert(addOmnibusSchema == !__fbthrift_omnibus_schema.empty());
  (void)addOmnibusSchema;
  return *self;
}

SchemaRegistry::Ptr SchemaRegistry::getMergedSchema() {
  std::shared_lock rlock(base_.mutex_);
  if (mergedSchema_) {
    mergedSchemaAccessed_ = true;
    return mergedSchema_;
  }
  rlock.unlock();

  std::unique_lock wlock(base_.mutex_);
  if (mergedSchema_) {
    mergedSchemaAccessed_ = true;
    return mergedSchema_;
  }

  mergedSchema_ = std::make_shared<type::Schema>();
  for (auto& [name, data] : base_.rawSchemas_) {
    if (data.data.empty()) {
      continue;
    }
    if (auto schema = schema::detail::readSchema(data.data[0])) {
      schema::detail::mergeInto(
          *mergedSchema_,
          std::move(*schema),
          includedPrograms_,
          /*allowDuplicateDefinitionKeys*/ false);
    }
  }

  base_.insertCallback_ = [this](std::string_view data) {
    // The caller is holding a write lock.

    // If no one else has a reference yet we can reuse the storage.
    if (mergedSchemaAccessed_.exchange(false)) {
      mergedSchema_ = std::make_shared<type::Schema>(*mergedSchema_);
    }

    if (auto schema = schema::detail::readSchema(data)) {
      schema::detail::mergeInto(
          *mergedSchema_,
          std::move(*schema),
          includedPrograms_,
          /*allowDuplicateDefinitionKeys*/ false);
    }
  };

  mergedSchemaAccessed_ = true;
  return mergedSchema_;
}

const syntax_graph::DefinitionNode*
SchemaRegistry::getSyntaxGraphDefinitionNodeByUri(
    const std::string_view uri) const {
  // Note: we check rawSchemasByUri_ first to ensure the definition is available
  // via dynamic bundling, even if it is already available because of an earlier
  // call to getNode using static bundling.
  // This prevents nondeterministic behavior where URIs are only sometimes found
  // depending on which other code happened to run earlier.
  auto* data = folly::get_default(base_.rawSchemasByUri_, uri);
  if (!data) {
    return nullptr;
  }
  return resolver_->getDefinitionNodeByUri(
      uri, type::ProgramId{data->programId}, data->data);
}

std::optional<type_system::DefinitionRef>
SchemaRegistry::getTypeSystemDefinitionRefByUri(
    const std::string_view uri) const {
  if (const auto* sgDef = getSyntaxGraphDefinitionNodeByUri(uri)) {
    return syntaxGraph_->asTypeSystemDefinitionRef(*sgDef);
  }
  return std::nullopt;
}

const folly::F14FastSet<type_system::Uri>& SchemaRegistry::getTypeSystemUris()
    const {
  return knownUris_.try_emplace_with([&] {
    folly::F14FastSet<type_system::Uri> ret;
    ret.reserve(base_.rawSchemasByUri_.size());
    for (const auto& [uri, _] : base_.rawSchemasByUri_) {
      ret.insert(type_system::Uri(uri));
    }
    return ret;
  });
}

} // namespace apache::thrift
