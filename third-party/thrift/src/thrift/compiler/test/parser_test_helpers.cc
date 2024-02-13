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

#include "thrift/compiler/test/parser_test_helpers.h"

#include <fstream>
#include <glog/logging.h>
#include <folly/Memory.h>
#include <folly/String.h>
#include <folly/experimental/TestUtil.h>
#include <thrift/compiler/ast/t_program_bundle.h>
#include <thrift/compiler/compiler.h>

std::shared_ptr<t_program> dedent_and_parse_to_program(
    source_manager& sm, std::string source) {
  auto temp_file = std::make_shared<const folly::test::TemporaryFile>();
  const auto path = temp_file->path().string();
  std::ofstream(path) << folly::stripLeftMargin(source);
  auto ctx = diagnostics_engine::ignore_all(sm);
  auto bundle = folly::to_shared_ptr(
      apache::thrift::compiler::parse_and_mutate_program(sm, ctx, path, {}));
  return {bundle->root_program(), [bundle, temp_file](auto) {}};
}
