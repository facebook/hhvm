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

#include <fmt/core.h>
#include <thrift/compiler/codemod/codemod.h>
#include <thrift/compiler/compiler.h>

namespace apache::thrift::compiler {

int run_codemod(
    int argc,
    char** argv,
    std::function<void(source_manager&, t_program&)> codemod) {
  if (argc <= 1) {
    fmt::print(stderr, "Usage: {} <thrift-file>\n", argv[0]);
    return 1;
  }

  auto source_mgr = source_manager();
  auto program_bundle = parse_and_get_program(
      source_mgr, std::vector<std::string>(argv, argv + argc));
  if (!program_bundle) {
    return 1;
  }
  codemod(source_mgr, *program_bundle->root_program());
  return 0;
}

} // namespace apache::thrift::compiler
