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

const std::string kTestFileName = "virtual/path/file1.thrift";

const std::string kTestFileContents = R"(
package "test.module"

namespace java test.module

struct Foo {
  1: optional i32 bar;
}
)";
} // namespace

struct Fixture final {
  std::unique_ptr<source_manager> source_manager_;
  source_view source_;
  std::unique_ptr<t_program_bundle> program_bundle_;
  std::unique_ptr<file_manager> file_manager_;
};

Fixture prepare_fixture(std::string_view contents) {
  auto sourceManager = std::make_unique<source_manager>();
  source_view srcView =
      sourceManager->add_virtual_file(kTestFileName, contents);
  diagnostics_engine diags = make_diagnostics_printer(*sourceManager);
  std::unique_ptr<t_program_bundle> programBundle =
      parse_ast(*sourceManager, diags, kTestFileName, {});

  auto fileManager = std::make_unique<file_manager>(
      *sourceManager, *programBundle->root_program());

  return Fixture{
      .source_manager_ = std::move(sourceManager),
      .source_ = srcView,
      .program_bundle_ = std::move(programBundle),
      .file_manager_ = std::move(fileManager),
  };
}

class FileManagerTest : public ::testing::Test {
 protected:
  void SetUp() override {
    Fixture fixture = prepare_fixture(kTestFileContents);

    source_manager_ = std::move(fixture.source_manager_);
    source_ = fixture.source_;
    program_bundle_ = std::move(fixture.program_bundle_);
    file_manager_ = std::move(fixture.file_manager_);
  }

  std::unique_ptr<source_manager> source_manager_;
  source_view source_;
  std::unique_ptr<t_program_bundle> program_bundle_;
  std::unique_ptr<file_manager> file_manager_;
};

TEST_F(FileManagerTest, old_content) {
  // NOTE: comparison is done via underlying data() using StrEq because the text
  // in the source manager is explicitly null-terminated.
  EXPECT_THAT(file_manager_->old_content().data(), StrEq(kTestFileContents));
}

TEST_F(FileManagerTest, add_include) {
  EXPECT_EQ(file_manager_->add_include("some/other/file.thrift"), 1);

  EXPECT_THAT(file_manager_->get_new_content().data(), StrEq(R"(
include "some/other/file.thrift"

package "test.module"

namespace java test.module

struct Foo {
  1: optional i32 bar;
}
)"));
}

TEST_F(FileManagerTest, add_include_no_op) {
  Fixture fixture = prepare_fixture(R"(
include "thrift/annotation/thrift.thrift"

package "test.package"

struct S {}
)");

  EXPECT_EQ(
      fixture.file_manager_->add_include("thrift/annotation/thrift.thrift"),
      std::nullopt);

  EXPECT_EQ(fixture.file_manager_->get_new_content(), R"(
include "thrift/annotation/thrift.thrift"

package "test.package"

struct S {}
)");
}

TEST_F(FileManagerTest, add_include_no_previous_include) {
  Fixture fixture = prepare_fixture(R"(
package "test.package"

struct S {}
)");

  EXPECT_EQ(fixture.file_manager_->add_include("some/other/file.thrift"), 1);

  EXPECT_EQ(fixture.file_manager_->get_new_content(), R"(
include "some/other/file.thrift"

package "test.package"

struct S {}
)");
}

TEST_F(FileManagerTest, add_include_namespace_before_package) {
  Fixture fixture = prepare_fixture(R"(
namespace cpp2 "test"

package "test.package"

struct S {}
)");

  EXPECT_EQ(fixture.file_manager_->add_include("some/other/file.thrift"), 24);

  EXPECT_EQ(fixture.file_manager_->get_new_content(), R"(
namespace cpp2 "test"

include "some/other/file.thrift"

package "test.package"

struct S {}
)");
}

TEST_F(FileManagerTest, add_include_with_docblock) {
  Fixture fixture = prepare_fixture(R"(
// Not a docblock
/// This comment describes the struct
struct S {}
)");

  fixture.file_manager_->add_include("some/other/file.thrift");

  EXPECT_EQ(fixture.file_manager_->get_new_content(), R"(
// Not a docblock
include "some/other/file.thrift"

/// This comment describes the struct
struct S {}
)");
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
    EXPECT_EQ(source_manager_->get_text_range(leading_whitespace), "  ");
  }

  {
    const source_range leading_whitespace =
        file_manager_->get_line_leading_whitespace(source_.start);

    EXPECT_EQ(leading_whitespace.begin.offset(), 0);
    EXPECT_EQ(leading_whitespace.end.offset(), 0);
    EXPECT_EQ(source_manager_->get_text_range(leading_whitespace), "");
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
