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

#include <thrift/compiler/ast/t_program.h>

#include <stdexcept>
#include <string>
#include <vector>

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>
#include <thrift/compiler/ast/t_enum.h>
#include <thrift/compiler/ast/t_struct.h>
#include <thrift/compiler/ast/t_union.h>
#include <thrift/conformance/data/Constants.h>

namespace apache::thrift::compiler {
namespace {

TEST(PackageTest, Empty) {
  t_package pkg;
  EXPECT_TRUE(pkg.empty());
  EXPECT_THAT(pkg.domain(), ::testing::IsEmpty());
  EXPECT_THAT(pkg.path(), ::testing::IsEmpty());
  EXPECT_THAT(pkg.get_uri("hi"), "");
}

TEST(PackageTest, Simple) {
  t_package pkg("test.dev/foo/bar");
  EXPECT_FALSE(pkg.empty());
  EXPECT_THAT(pkg.domain(), ::testing::ElementsAre("test", "dev"));
  EXPECT_THAT(pkg.path(), ::testing::ElementsAre("foo", "bar"));
  EXPECT_THAT(pkg.get_uri("hi"), "test.dev/foo/bar/hi");

  t_package other(pkg.domain(), pkg.path());
  EXPECT_THAT(other.domain(), ::testing::ElementsAre("test", "dev"));
  EXPECT_THAT(other.path(), ::testing::ElementsAre("foo", "bar"));
  EXPECT_THAT(other.get_uri("hi"), "test.dev/foo/bar/hi");
}

TEST(PackageTest, Validation) {
  for (const auto& good : conformance::data::kGoodPackageNames) {
    SCOPED_TRACE(good);
    t_package actual(good);
    EXPECT_EQ(actual, t_package(actual.domain(), actual.path()));
    EXPECT_THROW(t_package({"bad!"}, actual.path()), std::invalid_argument);
    EXPECT_THROW(t_package(actual.domain(), {"bad!"}), std::invalid_argument);
  }

  for (const auto& bad : conformance::data::kBadPackageNames) {
    SCOPED_TRACE(bad);
    EXPECT_THROW(t_package{bad}, std::invalid_argument);
  }
}

TEST(TProgram, GetNamespace) {
  t_program program("");

  const std::string expect_1 = "this.namespace";
  program.set_namespace("java", expect_1);
  program.set_namespace("java.swift", expect_1);

  const std::string expect_2 = "other.namespace";
  program.set_namespace("cpp", expect_2);
  program.set_namespace("py", expect_2);

  const std::string expect_3 = "";

  EXPECT_EQ(expect_1, program.get_namespace("java"));
  EXPECT_EQ(expect_1, program.get_namespace("java.swift"));
  EXPECT_EQ(expect_2, program.get_namespace("cpp"));
  EXPECT_EQ(expect_2, program.get_namespace("py"));
  EXPECT_EQ(expect_3, program.get_namespace("Non existent"));
}

TEST(TProgram, SetIncludePrefix) {
  t_program program("");

  const std::string dir_path_1 = "/this/is/a/dir";
  const std::string dir_path_2 = "/this/is/a/dir/";

  const std::string expect = "/this/is/a/dir/";

  program.set_include_prefix(dir_path_1);
  EXPECT_EQ(expect, program.include_prefix());
  program.set_include_prefix(dir_path_2);
  EXPECT_EQ(expect, program.include_prefix());
}

TEST(TProgram, ComputeNameFromFilePath) {
  t_program program("");

  const std::string expect = "tprogramtest";
  const std::string file_path_1 = expect;
  const std::string file_path_2 = expect + ".thrift";
  const std::string file_path_3 = "/this/is/a/path/" + expect + ".thrift";

  EXPECT_EQ(expect, program.compute_name_from_file_path(file_path_1));
  EXPECT_EQ(expect, program.compute_name_from_file_path(file_path_2));
  EXPECT_EQ(expect, program.compute_name_from_file_path(file_path_3));
}

TEST(TProgram, AddDefinitionUri) {
  t_program program("path/to/program.thrift");
  program.set_package(t_package{"test.dev/foo/bar"});

  EXPECT_EQ(program.name(), "program");
  EXPECT_THAT(
      program.package().domain(), ::testing::ElementsAre("test", "dev"));
  EXPECT_THAT(program.package().path(), ::testing::ElementsAre("foo", "bar"));

  { // Inherits uri and is accessible.
    auto& def = program.add_def(std::make_unique<t_struct>(&program, "Struct"));
    EXPECT_FALSE(def.explicit_uri());
    EXPECT_EQ(def.uri(), "test.dev/foo/bar/Struct");
    EXPECT_EQ(&def, program.scope()->find_by_uri("test.dev/foo/bar/Struct"));
  }
  { // Explicit override.
    auto& def = program.add_def(std::make_unique<t_enum>(&program, "Enum"), "");
    EXPECT_TRUE(def.explicit_uri());
    EXPECT_EQ(def.uri(), "");
  }
  { // Explicit annotation override.
    auto node = std::make_unique<t_union>(&program, "Union");
    node->set_annotation("thrift.uri");
    auto& def = program.add_def(std::move(node));
    EXPECT_TRUE(def.explicit_uri());
    EXPECT_EQ(def.uri(), "");
  }
}

} // namespace
} // namespace apache::thrift::compiler
