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

#include <memory>
#include <string>
#include <vector>

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>

#include <thrift/compiler/ast/t_exception.h>
#include <thrift/compiler/ast/t_function.h>
#include <thrift/compiler/ast/t_interaction.h>
#include <thrift/compiler/ast/t_service.h>
#include <thrift/compiler/ast/t_struct.h>
#include <thrift/compiler/ast/t_union.h>
#include <thrift/compiler/test/parser_test_helpers.h>
#include <thrift/compiler/validator/validator.h>

namespace {

class ValidatorTest : public testing::Test {
 public:
  source_manager source_mgr;
  diagnostics_engine diags;
  source_location loc;
  std::vector<diagnostic> errors;

  ValidatorTest()
      : diags(source_mgr, [this](diagnostic d) { errors.push_back(d); }),
        loc(source_mgr.add_virtual_file("/path/to/file.thrift", "\n\n").start) {
  }
};

} // namespace

TEST_F(ValidatorTest, run_validator) {
  class fake_validator : public validator {
   public:
    using visitor::visit;
    bool visit(t_program* program) final {
      EXPECT_TRUE(validator::visit(program));
      report_error(*program, "sadface");
      return true;
    }
  };

  t_program program("/path/to/file.thrift");
  program.set_src_range({loc, loc});
  run_validator<fake_validator>(diags, &program);
  EXPECT_EQ(1, errors.size());
  EXPECT_EQ("[ERROR:/path/to/file.thrift:1] sadface", errors.front().str());
}
