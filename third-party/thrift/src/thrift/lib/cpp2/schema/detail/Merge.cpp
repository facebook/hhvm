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

#include <thrift/lib/cpp2/schema/detail/Merge.h>

#ifdef THRIFT_SCHEMA_AVAILABLE

#include <folly/compression/Compression.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>

#if __has_include(<thrift/facebook/schema/omnibus_schema_header.h>)
#include <thrift/facebook/schema/omnibus_schema_header.h>
#else
namespace {
constexpr std::string_view __fbthrift_omnibus_schema;
}
#endif

namespace apache::thrift::schema::detail {
std::optional<type::Schema> readSchema(std::string_view data) {
  if (data.empty()) {
    // This program's schema wasn't found under the expected path, e.g. due to
    // use of relative includes.
    return std::nullopt;
  }
  auto decompressed =
      folly::compression::getCodec(folly::compression::CodecType::ZSTD)
          ->uncompress(data);
  return CompactSerializer::deserialize<type::Schema>(decompressed);
}

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

type::Schema mergeSchemas(folly::Range<const std::string_view*> schemas) {
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

type::Schema mergeSchemas(std::vector<type::Schema>&& schemas) {
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

type::Schema loadBundledSchema(folly::Range<const std::string_view*> schemas) {
  type::Schema mergedSchema;
  std::unordered_set<type::ProgramId> includedPrograms;

  if (auto schema = readSchema(__fbthrift_omnibus_schema)) {
    mergeInto(
        mergedSchema,
        std::move(*schema),
        includedPrograms,
        /*allowDuplicateDefinitionKeys*/ false);
  }
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

} // namespace apache::thrift::schema::detail

#endif
