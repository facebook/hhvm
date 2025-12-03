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

#include <string>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <thrift/compiler/ast/t_program_bundle.h>
#include <thrift/compiler/codemod/file_manager.h>
#include <thrift/compiler/parse/parse_ast.h>
#include <thrift/compiler/source_location.h>

using ::testing::NotNull;
using ::testing::StrEq;
using std::literals::string_view_literals::operator""sv;

namespace apache::thrift::compiler::codemod {
namespace {

const std::string test_file_name = "virtual/path/file1.thrift";

const std::string test_file_contents = R"(
package "test.module"

namespace java test.module

struct Foo {
  1: optional i32 bar;
}
)";
} // namespace

class FileManagerTest : public ::testing::Test {
 protected:
  void SetUp() override {
    source_ =
        source_manager_.add_virtual_file(test_file_name, test_file_contents);
    auto diags = make_diagnostics_printer(source_manager_);
    program_bundle_ = parse_ast(source_manager_, diags, test_file_name, {});
    ASSERT_THAT(program_bundle_, NotNull());

    file_manager_ = std::make_unique<file_manager>(
        source_manager_, *program_bundle_->root_program());
  }

  source_manager source_manager_;
  source_view source_;
  std::unique_ptr<t_program_bundle> program_bundle_;
  std::unique_ptr<file_manager> file_manager_;
};

TEST_F(FileManagerTest, old_content) {
  // NOTE: comparison is done via underlying data() using StrEq because the text
  // in the source manager is explicitly null-terminated.
  EXPECT_THAT(file_manager_->old_content().data(), StrEq(test_file_contents));
}

TEST_F(FileManagerTest, add_include) {
  file_manager_->add_include("some/other/file.thrift");

  EXPECT_THAT(file_manager_->get_new_content().data(), StrEq(R"(
include "some/other/file.thrift"

package "test.module"

namespace java test.module

struct Foo {
  1: optional i32 bar;
}
)"));
}

TEST_F(FileManagerTest, add) {
  file_manager_->add({/* .begin_pos= */ 52,
                      /* .end_pos= */ 52 + "struct"sv.size(),
                      /* .new_content= */ "union"});

  file_manager_->add({/* .begin_pos= */ 69,
                      /* .end_pos= */ 69 + "optional "sv.size(),
                      /* .new_content= */ ""});

  EXPECT_THAT(file_manager_->get_new_content().data(), StrEq(R"(
package "test.module"

namespace java test.module

union Foo {
  1: i32 bar;
}
)"));
}

TEST_F(FileManagerTest, set_namespace) {
  file_manager_->set_namespace("hack", "test.module");

  EXPECT_THAT(file_manager_->get_new_content().data(), StrEq(R"(
package "test.module"

namespace hack "test.module"
namespace java test.module

struct Foo {
  1: optional i32 bar;
}
)"));
}

TEST_F(FileManagerTest, get_line_leading_whitespace) {
  {
    const source_range leading_whitespace =
        file_manager_->get_line_leading_whitespace(source_.start + 69);

    EXPECT_EQ(leading_whitespace.begin.offset(), 65);
    EXPECT_EQ(leading_whitespace.end.offset(), 67);
    EXPECT_EQ(source_manager_.get_text_range(leading_whitespace), "  ");
  }

  {
    const source_range leading_whitespace =
        file_manager_->get_line_leading_whitespace(source_.start);

    EXPECT_EQ(leading_whitespace.begin.offset(), 0);
    EXPECT_EQ(leading_whitespace.end.offset(), 0);
    EXPECT_EQ(source_manager_.get_text_range(leading_whitespace), "");
  }
}

// Testing overloading of < operator in replacement struct.
TEST(CodemodReplacementTest, replacement_less_than) {
  replacement a{2, 4, ""};
  replacement b{2, 5, ""};
  replacement c{3, 5, ""};
  replacement d{5, 7, ""};

  EXPECT_TRUE(a < b); // Same begin, different end
  EXPECT_TRUE(b < c); // Same end, different begin
  EXPECT_TRUE(a < c); // Overlapping
  EXPECT_TRUE(a < d); // Non-overlapping
}

} // namespace apache::thrift::compiler::codemod
