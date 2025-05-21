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

  /*
   * This determines if the given structured type (struct, union or exception)
   * can be ordered.
   *
   * If the type is using any annotation for cpp2.type or cpp2.template
   * its not considered orderable, and we don't need to generate operator<
   * methods
   */
  static bool is_orderable(
      const t_structured& type, bool enableCustomTypeOrderingIfStructureHasUri);

  static bool is_orderable(
      std::unordered_map<const t_type*, bool>& memo,
      const t_structured& type,
      bool enableCustomTypeOrderingIfStructureHasUri);
};

} // namespace apache::thrift::compiler::cpp2
