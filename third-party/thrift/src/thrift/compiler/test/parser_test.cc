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

#include <thrift/compiler/parse/parse_ast.h>

#include <optional>

#include <folly/portability/GTest.h>

using namespace apache::thrift::compiler;

TEST(ParserTest, type_resolution) {
  auto source_mgr = source_manager();
  source_mgr.add_virtual_file("test.thrift", R"(
    struct S {
      1: MissingType field;
    }
  )");
  auto diag = std::optional<diagnostic>();
  auto diags =
      diagnostics_engine(source_mgr, [&diag](diagnostic d) { diag = d; });

  // Types must be resolved in parse_ast.
  auto programs = parse_ast(source_mgr, diags, "test.thrift", {});
  EXPECT_TRUE(diags.has_errors());
  EXPECT_EQ(diag->message(), "Type `test.MissingType` not defined.");

  // Programs must be non-null even in case of an error.
  EXPECT_TRUE(programs != nullptr);
}
