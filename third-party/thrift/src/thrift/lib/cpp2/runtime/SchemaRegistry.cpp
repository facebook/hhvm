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

namespace apache::thrift {

void SchemaRegistry::registerSchema(
    std::string_view name, std::string_view data, std::string_view path) {
  if (accessed()) {
    throw std::runtime_error("Schemas accessed before registration complete.");
  }
  if (auto it = getRawSchemas().find(name); it != getRawSchemas().end()) {
    if (it->second.path != path) { // Needed to support dynamic linking
      throw std::runtime_error(fmt::format(
          "Checksum collision between {} and {}. Make any change to either file's content (e.g.. whitespace/comment) to fix it.",
          it->second.path,
          path));
    }
    return;
  }
  getRawSchemas()[name] = {data, path};
}

#ifdef FBTHRIFT_HAS_SCHEMA
const type::Schema& SchemaRegistry::getMergedSchema() {
  static const folly::Indestructible<type::Schema> merged = [&] {
    accessed() = true;
    type::Schema mergedSchema;
    std::unordered_set<type::ProgramId> includedPrograms;

    for (auto& [name, data] : getRawSchemas()) {
      auto schema = CompactSerializer::deserialize<type::Schema>(data.data);

      for (auto& program : *schema.programs()) {
        auto id = *program.id();
        if (!includedPrograms.insert(id).second) {
          // We checked during registration that the program ids are unique,
          // so this file was already included by another program bundle.
          continue;
        }
        mergedSchema.programs()->push_back(std::move(program));
      }

      mergedSchema.valuesMap()->insert(
          std::make_move_iterator(schema.valuesMap()->begin()),
          std::make_move_iterator(schema.valuesMap()->end()));
      // This deduplicates common values.

      auto ndefs = mergedSchema.definitionsMap()->size();
      mergedSchema.definitionsMap()->insert(
          std::make_move_iterator(schema.definitionsMap()->begin()),
          std::make_move_iterator(schema.definitionsMap()->end()));
      if (mergedSchema.definitionsMap()->size() !=
          ndefs + schema.definitionsMap()->size()) {
        throw std::runtime_error("DefinitionKey collision");
      }
    }

    return mergedSchema;
  }();

  return *merged;
}
#endif

folly::F14FastMap<std::string_view, SchemaRegistry::RawSchema>&
SchemaRegistry::getRawSchemas() {
  static folly::Indestructible<
      folly::F14FastMap<std::string_view, SchemaRegistry::RawSchema>>
      schemas;
  return *schemas;
}

bool& SchemaRegistry::accessed() {
  static folly::Indestructible<bool> accessed = false;
  return *accessed;
}

} // namespace apache::thrift
