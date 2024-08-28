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

#include <thrift/compiler/whisker/ast.h>
#include <thrift/compiler/whisker/diagnostic.h>
#include <thrift/compiler/whisker/parser.h>
#include <thrift/compiler/whisker/print_ast.h>
#include <thrift/compiler/whisker/source_location.h>

#include <cassert>
#include <cstdio>
#include <iostream>
#include <iterator>
#include <optional>
#include <string>
#include <string_view>

#include <fmt/core.h>

using namespace std::literals;

namespace {
[[nodiscard]] int usage() {
  constexpr std::string_view kUsageMessage = R"(Usage: ast-dump [FILE])
Dump a human-readable representation of the AST for a Whisker template.

With no FILE, of when FILE is -, read standard input.

Examples:
  ast-dump template.mustache
  ast-dump < template.mustache)";
  fmt::print(stderr, "{}\n", kUsageMessage);
  return 1;
}
} // namespace

int main(int argc, char** argv) {
  assert(argc >= 1);
  if (argc > 2) {
    return usage();
  }

  whisker::source_manager src_manager;
  std::optional<whisker::source> src;
  if (argc == 1 || argv[1] == "-"sv) {
    std::istreambuf_iterator<char> begin(std::cin);
    std::istreambuf_iterator<char> end;
    src = src_manager.add_virtual_file("<stdin>", std::string(begin, end));
  } else {
    const char* filepath = argv[1];
    src = src_manager.get_file(filepath);
    if (!src.has_value()) {
      fmt::print(stderr, "Failed to read file '{}'.\n", filepath);
      return 1;
    }
  }
  assert(src.has_value());

  whisker::diagnostics_engine diagnostics_engine(
      src_manager,
      [](const whisker::diagnostic& d) { fmt::print(stderr, "{}\n", d); },
      whisker::diagnostic_params::keep_all());

  std::optional<whisker::ast::root> ast =
      whisker::parse(*src, src_manager, diagnostics_engine);

  if (!ast.has_value()) {
    // diagnostics should already have been reported
    return 1;
  }
  whisker::print_ast(*ast, src_manager, std::cout);

  return 0;
}
