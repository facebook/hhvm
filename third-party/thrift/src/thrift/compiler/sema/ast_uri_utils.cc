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

#include <thrift/compiler/sema/ast_uri_utils.h>

#include <thrift/compiler/ast/t_enum.h>
#include <thrift/compiler/ast/t_exception.h>
#include <thrift/compiler/ast/t_named.h>
#include <thrift/compiler/ast/t_struct.h>
#include <thrift/compiler/ast/t_type.h>
#include <thrift/compiler/ast/t_union.h>

namespace apache::thrift::compiler {

bool AstUriUtils::shouldHaveUri(const t_named& node) {
  const auto* nodeAsType = dynamic_cast<const t_type*>(&node);
  if (nodeAsType == nullptr) {
    return false;
  }

  return (
      nodeAsType->is<t_enum>() || nodeAsType->is<t_exception>() ||
      nodeAsType->is<t_struct>() || nodeAsType->is<t_union>());
}

} // namespace apache::thrift::compiler
