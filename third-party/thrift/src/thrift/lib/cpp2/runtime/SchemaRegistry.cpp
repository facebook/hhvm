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
#include <folly/compression/Compression.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>

namespace apache::thrift {

SchemaRegistry& SchemaRegistry::get() {
  static folly::Indestructible<SchemaRegistry> self(BaseSchemaRegistry::get());
  return *self;
}

namespace {
void mergeInto(
    type::Schema& dst,
    type::Schema&& src,
    std::unordered_set<type::ProgramId>& includedPrograms,
    bool allowDuplicateDefinitionKeys) {
  for (auto& program : *src.programs()) {
    auto id = *program.id();
    if (!includedPrograms.insert(id).second) {
      // We checked during registration that the program ids are unique,
      // so this file was already included by another program bundle.
      continue;
    }
    dst.programs()->push_back(std::move(program));
  }

  // This deduplicates common values.
  dst.valuesMap()->insert(
      std::make_move_iterator(src.valuesMap()->begin()),
      std::make_move_iterator(src.valuesMap()->end()));

  auto ndefs = dst.definitionsMap()->size();
  dst.definitionsMap()->insert(
      std::make_move_iterator(src.definitionsMap()->begin()),
      std::make_move_iterator(src.definitionsMap()->end()));
  if (!allowDuplicateDefinitionKeys &&
      dst.definitionsMap()->size() != ndefs + src.definitionsMap()->size()) {
    throw std::runtime_error("DefinitionKey collision");
  }
}

std::optional<type::Schema> readSchema(std::string_view data) {
  if (data.empty()) {
    // This program's schema wasn't found under the expected path, e.g. due to
    // use of relative includes.
    return std::nullopt;
  }
  auto decompressed = folly::io::getCodec(folly::compression::CodecType::ZSTD)
                          ->uncompress(data);
  return CompactSerializer::deserialize<type::Schema>(decompressed);
}
} // namespace

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
    if (auto schema = readSchema(data.data)) {
      mergeInto(
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

    if (auto schema = readSchema(data)) {
      mergeInto(
          *mergedSchema_,
          std::move(*schema),
          includedPrograms_,
          /*allowDuplicateDefinitionKeys*/ false);
    }
  };

  mergedSchemaAccessed_ = true;
  return mergedSchema_;
}

type::Schema SchemaRegistry::mergeSchemas(
    folly::Range<const std::string_view*> schemas) {
  type::Schema mergedSchema;
  std::unordered_set<type::ProgramId> includedPrograms;

  for (const auto& data : schemas) {
    if (auto schema = readSchema(data)) {
      mergeInto(
          mergedSchema,
          std::move(*schema),
          includedPrograms,
          /*allowDuplicateDefinitionKeys*/ false);
    }
  }

  return mergedSchema;
}

type::Schema SchemaRegistry::mergeSchemas(std::vector<type::Schema>&& schemas) {
  type::Schema mergedSchema;
  std::unordered_set<type::ProgramId> includedPrograms;

  for (auto& schema : schemas) {
    /*
     * allowDuplicateDefinitionKeys is true here because this is called by
     * MultiplexAsyncProcessor, which may hold services with a shared base
     * service.
     * Additionally, since this function accepts deserialized schemas
     * those schemas were probably already checked for duplicates earlier.
     */
    mergeInto(
        mergedSchema,
        std::move(schema),
        includedPrograms,
        /*allowDuplicateDefinitionKeys*/ true);
  }

  return mergedSchema;
}

} // namespace apache::thrift
