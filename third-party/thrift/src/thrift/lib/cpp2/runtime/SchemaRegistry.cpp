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
#include <thrift/lib/cpp2/runtime/BaseSchemaRegistry.h>

namespace apache::thrift {

const type::Schema& SchemaRegistry::getMergedSchema() {
  static const folly::Indestructible<type::Schema> merged = [&] {
    BaseSchemaRegistry::accessed() = true;
    std::vector<std::string_view> schemas;
    schemas.reserve(BaseSchemaRegistry::getRawSchemas().size());
    for (auto& [name, data] : BaseSchemaRegistry::getRawSchemas()) {
      schemas.push_back(data.data);
    }
    return mergeSchemas(folly::range(schemas));
  }();

  return *merged;
}

namespace {
void mergeInto(
    type::Schema& dst,
    type::Schema&& src,
    std::unordered_set<type::ProgramId> includedPrograms) {
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
  if (dst.definitionsMap()->size() != ndefs + src.definitionsMap()->size()) {
    throw std::runtime_error("DefinitionKey collision");
  }
}
} // namespace

type::Schema SchemaRegistry::mergeSchemas(
    folly::Range<const std::string_view*> schemas) {
  type::Schema mergedSchema;
  std::unordered_set<type::ProgramId> includedPrograms;

  for (const auto& data : schemas) {
    if (data.empty()) {
      // This program's schema wasn't found under the expected path, e.g. due to
      // use of relative includes.
      continue;
    }
    auto decompressed = folly::io::getCodec(folly::compression::CodecType::ZSTD)
                            ->uncompress(data);
    auto schema = CompactSerializer::deserialize<type::Schema>(decompressed);
    mergeInto(mergedSchema, std::move(schema), includedPrograms);
  }

  return mergedSchema;
}

type::Schema SchemaRegistry::mergeSchemas(std::vector<type::Schema>&& schemas) {
  type::Schema mergedSchema;
  std::unordered_set<type::ProgramId> includedPrograms;

  for (auto& schema : schemas) {
    mergeInto(mergedSchema, std::move(schema), includedPrograms);
  }

  return mergedSchema;
}

} // namespace apache::thrift
