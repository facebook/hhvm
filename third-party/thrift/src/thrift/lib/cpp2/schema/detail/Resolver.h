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

#include <vector>

#include <folly/memory/not_null.h>
#include <thrift/lib/cpp2/schema/SchemaV1.h>
#include <thrift/lib/thrift/gen-cpp2/type_id_types.h>

namespace apache::thrift::syntax_graph {
class ProgramNode;
class DefinitionNode;

namespace detail {
namespace type = apache::thrift::type;
namespace protocol = apache::thrift::protocol;

class Resolver {
 public:
  virtual ~Resolver() = default;

  /**
   * Programs are identified by a separate namespace of IDs than definitions.
   */
  virtual const ProgramNode& programOf(const type::ProgramId&) const = 0;

  /**
   * schema.thrift de-duplicates Thrift values in the IDL via interning. This
   * function performs a lookup to access an interned value by its ID.
   */
  virtual const protocol::Value& valueOf(const type::ValueId&) const = 0;

  /**
   * Returns the graph node representing the definition with the given key, or
   * nullptr if it doesn't exist.
   */
  virtual const DefinitionNode* definitionOf(
      const type::DefinitionKey&) const = 0;

  /**
   * List of all unique .thrift files that are accessible in the schema.
   */
  virtual std::vector<folly::not_null<const ProgramNode*>> programs() const = 0;

  /**
   * Generates a set of all user-defined type URIs currently known to the
   * fully resolved resolver. For non-fully resolved resolver, it returns an
   * empty optional since the set of URIs is not enumerable,
   */
  virtual std::optional<folly::F14FastSet<type_system::Uri>> getKnownUris()
      const = 0;
};

} // namespace detail
} // namespace apache::thrift::syntax_graph
