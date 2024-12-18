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
#include <thrift/compiler/compiler.h>
#include <thrift/compiler/source_location.h>

namespace apache::thrift::compiler::codemod {

namespace {

using ::testing::Eq;
using ::testing::IsTrue;
using ::testing::NotNull;
using ::testing::StrEq;
using std::literals::string_view_literals::operator""sv;

static const std::string kVirtualFileName("virtual/path/file1.thrift");

static const std::string kVirtualFileContents(R"(
package "test.module"

namespace java test.module

struct Foo {
  1: optional i32 bar;
}
)");
} // namespace

class FileManagerTest : public ::testing::Test {
 protected:
  void SetUp() override {
    source_manager_.add_virtual_file(kVirtualFileName, kVirtualFileContents);
    program_bundle_ = parse_and_get_program(
        source_manager_, {"<virtual compiler>", kVirtualFileName});
    ASSERT_THAT(program_bundle_, NotNull());

    file_manager_ = std::make_unique<file_manager>(
        source_manager_, *program_bundle_->root_program());
  }

  source_manager source_manager_;
  std::unique_ptr<t_program_bundle> program_bundle_;
  std::unique_ptr<file_manager> file_manager_;
};

TEST_F(FileManagerTest, old_content) {
  // NOTE: comparison is done via underlying data() using STREQ because the text
  // in the source manager is explicitly null-terminated.
  EXPECT_THAT(file_manager_->old_content().data(), StrEq(kVirtualFileContents));
}

TEST_F(FileManagerTest, add_include) {
  file_manager_->add_include("some/other/file.thrift");

  EXPECT_THAT(file_manager_->get_new_content().data(), StrEq(R"(
package "test.module"

namespace java test.module

include "some/other/file.thrift"

struct Foo {
  1: optional i32 bar;
}
)"));
}

TEST_F(FileManagerTest, add) {
  file_manager_->add(
      {/* .begin_pos= */ 52,
       /* .end_pos= */ 52 + "struct"sv.size(),
       /* .new_content= */ "union"});

  file_manager_->add(
      {/* .begin_pos= */ 69,
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
