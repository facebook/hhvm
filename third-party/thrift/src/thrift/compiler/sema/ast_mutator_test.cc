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

#include <thrift/compiler/sema/ast_mutator.h>

#include <memory>

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>
#include <thrift/compiler/ast/t_program.h>
#include <thrift/compiler/diagnostic.h>
#include <thrift/compiler/sema/diagnostic_context.h>

namespace apache::thrift::compiler {
namespace {

using ::testing::UnorderedElementsAre;

class AstMutatorTest : public ::testing::Test {};

TEST_F(AstMutatorTest, Output) {
  ast_mutator mutator;
  mutator.add_program_visitor([](diagnostic_context& ctx,
                                 mutator_context& mctx,
                                 t_program& program) {
    ctx.report(program, diagnostic_level::info, "test");
    EXPECT_EQ(mctx.current(), &program);
    EXPECT_EQ(mctx.parent(), nullptr);
    EXPECT_EQ(mctx.root(), &program);
    program.add_service(std::make_unique<t_service>(&program, "MagicService"));
  });

  t_program program("path/to/program.thrift");
  source_manager source_mgr;
  diagnostic_results results;
  diagnostic_context ctx(source_mgr, results, diagnostic_params::keep_all());
  auto loc = source_mgr.add_virtual_file(program.path(), "").start;
  program.set_src_range({loc, loc});
  mutator_context mctx;
  mutator(ctx, mctx, program);
  EXPECT_THAT(
      results.diagnostics(),
      UnorderedElementsAre(
          diagnostic(diagnostic_level::info, "test", program.path(), 1)));
  ASSERT_EQ(program.services().size(), 1);
  EXPECT_EQ(program.services().front()->name(), "MagicService");
}

} // namespace
} // namespace apache::thrift::compiler
