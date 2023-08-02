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

#include <folly/String.h>
#include <folly/portability/GTest.h>
#include <thrift/compiler/codemod/file_manager.h>
#include <thrift/compiler/test/parser_test_helpers.h>

namespace apache::thrift::compiler {

std::string read_file(const std::string& path) {
  std::string content;
  EXPECT_TRUE(folly::readFile(path.c_str(), content));
  return content;
}

// Testing overloading of < operator in replacement struct.
TEST(FileManagerTest, replacement_less_than) {
  codemod::replacement a{2, 4, ""};
  codemod::replacement b{2, 5, ""};
  codemod::replacement c{3, 5, ""};
  codemod::replacement d{5, 7, ""};

  EXPECT_TRUE(a < b); // Same begin, different end
  EXPECT_TRUE(b < c); // Same end, different begin
  EXPECT_TRUE(a < c); // Overlapping
  EXPECT_TRUE(a < d); // Non-overlapping
}

// Basic test of apply_replacements functionality, without traversing AST.
TEST(FileManagerTest, apply_replacements_test) {
  auto source_mgr = source_manager();
  auto program = dedent_and_parse_to_program(source_mgr, R"(
    struct A {
      1: optional A a (cpp.ref);
    } (cpp.noexcept_move)
  )");

  codemod::file_manager fm(source_mgr, *program);

  fm.add({13, 39, "@cpp.Ref{cpp.RefType.Unique}\n  1: optional string a;"});
  fm.add({41, 61, ""});

  fm.apply_replacements();

  EXPECT_EQ(read_file(program->path()), folly::stripLeftMargin(R"(
    struct A {
      @cpp.Ref{cpp.RefType.Unique}
      1: optional string a;
    }
  )"));
}

TEST(FileManagerTest, namespace_offset) {
  auto source_mgr = source_manager();
  auto program = dedent_and_parse_to_program(
      source_mgr,
      R"(
      // include foo_thrift
      package "test.module"
      namespace java test.module
      namespace cpp2 facebook.thrift.test.module;
      struct A {
      })");

  codemod::file_manager fm(source_mgr, *program);

  fm.set_namespace("hack", "test.module");
  fm.apply_replacements();

  auto file_content = read_file(program->path());
  file_content.erase(
      std::remove(file_content.begin(), file_content.end(), '\r'),
      file_content.end());

  EXPECT_EQ(file_content, folly::stripLeftMargin(R"(
      // include foo_thrift
      package "test.module"
      namespace hack "test.module"
      namespace java test.module
      namespace cpp2 facebook.thrift.test.module;
      struct A {
      })"));
}
} // namespace apache::thrift::compiler
