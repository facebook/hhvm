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

#include <thrift/compiler/whisker/ast.h>

#include <iosfwd>

namespace whisker {

/**
 * Prints the AST to the given output stream in human-readable form.
 * The format of the output is roughly based on clang's `-ast-dump`:
 *   https://clang.llvm.org/docs/IntroductionToTheClangAST.html#examining-the-ast
 */
template <class Node>
void print_ast(const Node&, const source_manager&, std::ostream&);

} // namespace whisker
