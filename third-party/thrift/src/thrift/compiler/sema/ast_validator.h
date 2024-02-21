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

#include <thrift/compiler/ast/ast_visitor.h>
#include <thrift/compiler/sema/diagnostic_context.h>

namespace apache {
namespace thrift {
namespace compiler {

// An AST validator is a const ast_visitor that collects diagnostics.
using ast_validator = basic_ast_visitor<true, diagnostic_context&>;

} // namespace compiler
} // namespace thrift
} // namespace apache
