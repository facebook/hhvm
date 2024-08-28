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
#include <thrift/compiler/whisker/source_location.h>

#include <cassert>
#include <cstdio>
#include <optional>
#include <string_view>

#include <fmt/core.h>

using namespace std::literals;

namespace {
[[nodiscard]] int usage() {
  constexpr std::string_view kUsageMessage = R"(Usage: ast-check [FILE]...)
Checks (pass or fail) that provided FILEs are syntactically valid Whisker templates.

At least one FILE must be specified.

Examples:
  ast-check template-1.mustache template-2.mustache)";
  fmt::print(stderr, "{}\n", kUsageMessage);
  return 1;
}
} // namespace

int main(int argc, char** argv) {
  assert(argc >= 1);
  if (argc == 1) {
    return usage();
  }

  whisker::source_manager src_manager;
  whisker::diagnostics_engine diagnostics_engine(
      src_manager,
      [](const whisker::diagnostic& d) { fmt::print(stderr, "{}\n", d); },
      whisker::diagnostic_params::keep_all());

  for (int i = 1; i < argc; ++i) {
    const char* filepath = argv[i];
    std::optional<whisker::source> src = src_manager.get_file(filepath);
    if (!src.has_value()) {
      fmt::print(stderr, "Failed to read file '{}'.\n", filepath);
      continue;
    }
    std::optional<whisker::ast::root> ast =
        whisker::parse(*src, src_manager, diagnostics_engine);
    if (ast.has_value()) {
      fmt::print(stdout, "SUCCESS: '{}'\n", filepath);
    } else {
      fmt::print(stdout, "FAIL: '{}'\n", filepath);
    }
  }

  return 0;
}
