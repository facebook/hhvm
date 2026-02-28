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

#include <unordered_map>
#include <thrift/compiler/ast/t_structured.h>
#include <thrift/compiler/ast/t_type.h>

namespace apache::thrift::compiler::cpp2 {

class OrderableTypeUtils final {
 public:
  OrderableTypeUtils() = delete;

  /**
   * Determines if the given structured type (struct, union or exception) can be
   * ordered.
   *
   * If `true`, the C++ generated Thrift code for the given type will define an
   * implementation of `operator<` (note that a *declaration* of `operator<`
   * may be generated even if this function is `false`, to support user-defined
   * operators).
   *
   * In general, C++ generated Thrift structured types can be ordered, however
   * there can be issues if the structure includes fields whose types do not
   * have a natural order (i.e., maps, sets and typedefs that resolve to a map
   * or set).
   *
   * Thrift always supports ordering for the [default target
   * types](https://github.com/facebook/fbthrift/blob/main/thrift/doc/glossary/kinds-of-types.md#thrift-target-types)
   * of such fields (i.e., `std::map` and `std::set`, respectively), however it
   * may be unable to do so if a custom type is specified (via `@cpp.Type`,
   * `cpp2.type`, `cpp2.template`, etc.).
   *
   * If `structured_type` does not have any field whose type resolves to a map
   * or set with custom type, this function always returns `true`.
   *
   * Otherwise, this function returns `true` if and only if custom types are
   * explicitly considered orderable, through (at least) one of the following
   * mechanisms:
   * 1. The given type (or its enclosing package) is annotated with
   *    `@cpp.EnableCustomTypeOrdering`, or
   *
   * It follows that this function returns `false` iff `structured_type` has
   * unordered container field(s) with custom types AND custom type ordering is
   * disabled.
   */
  static bool is_orderable(const t_structured& structured_type);

  /**
   * Same as `is_orderable()` above, but with an explicitly provided memoization
   * cache.
   */
  static bool is_orderable(
      std::unordered_map<const t_type*, bool>& memo,
      const t_structured& structured_type);

  enum class StructuredOrderableCondition {
    /**
     * The generated structured type is always orderable (i.e., it does not
     * contain any unordered container field with custom type).
     */
    Always,

    /**
     * The generated structured type cannot be ordered as such, because it
     * contains unordered container field(s) with custom types, and custom type
     * ordering is not enabled. Enabling custom type ordering (via explicit
     * annotation) would make it orderable.
     */
    NotOrderable,

    /**
     * The generated structured type has unordered container field(s) with
     * custom types, but is explicitly annotated with
     * `@cpp.EnableCustomTypeOrdering`, thus making it orderable.
     */
    OrderableByExplicitAnnotation,
  };

  /**
   * Returns the conditions under which the C++ generated class for the given
   * `structured_type` can be made orderable.
   *
   * The `is_orderable()` methods above are semantically equivalent to
   * ```
   * get_orderable_condition(...) !=
   * StructuredOrderableCondition::NotOrderable
   * ```
   * but may be faster.
   */
  static StructuredOrderableCondition get_orderable_condition(
      const t_structured& structured_type);
};

} // namespace apache::thrift::compiler::cpp2
