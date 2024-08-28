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

#include <thrift/compiler/diagnostic.h>
#include <thrift/compiler/whisker/ast.h>
#include <thrift/compiler/whisker/diagnostic.h>
#include <thrift/compiler/whisker/source_location.h>

#include <optional>

namespace whisker {

/**
 * Parses a Whisker source file into an AST. If parsing succeeds, then an
 * ast::root object is returned. Otherwise, an empty optional is returned and
 * the provided diagnostics_engine is passed the relevant errors that caused
 * parsing to fail.
 */
std::optional<ast::root> parse(
    source, const source_manager&, diagnostics_engine&);

} // namespace whisker
