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

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>

#include <thrift/compiler/test/test_utils.h>

#include <thrift/compiler/sema/standard_validator.h>

namespace apache::thrift::compiler {

using ::testing::UnorderedElementsAre;

TEST(AstValidatorTest, Output) {
  ast_validator validator;
  validator.add_program_visitor(
      [](sema_context& ctx, const t_program& program) {
        ctx.report(program, diagnostic_level::info, "test");
      });

  t_program program("path/to/program.thrift");
  source_manager source_mgr;
  auto loc = source_mgr.add_virtual_file(program.path(), "").start;
  program.set_src_range({loc, loc});
  diagnostic_results results;
  sema_context ctx(source_mgr, results, diagnostic_params::keep_all());
  validator(ctx, program);
  EXPECT_THAT(
      results.diagnostics(),
      UnorderedElementsAre(
          diagnostic(diagnostic_level::info, "test", program.path(), 1)));
}

} // namespace apache::thrift::compiler
