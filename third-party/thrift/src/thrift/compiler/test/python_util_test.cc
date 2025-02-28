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

#include <gtest/gtest.h>

#include <thrift/compiler/ast/t_field.h>
#include <thrift/compiler/ast/t_primitive_type.h>
#include <thrift/compiler/generate/python/util.h>

using namespace apache::thrift::compiler;
using Params =
    std::tuple<std::string_view, std::unordered_set<std::string_view>>;

TEST(UtilTest, get_py3_name) {
  EXPECT_EQ(
      "foo", python::get_py3_name(t_field(t_primitive_type::t_i32(), "foo")));
  EXPECT_EQ(
      "True_",
      python::get_py3_name(t_field(t_primitive_type::t_i32(), "True")));
  EXPECT_EQ(
      "cpdef_",
      python::get_py3_name(t_field(t_primitive_type::t_i32(), "cpdef")));

  t_field f(t_primitive_type::t_i32(), "foo");
  f.set_annotation("py3.name", "bar");
  EXPECT_EQ("bar", python::get_py3_name(f));
}

class ExtractModulePathTest : public testing::TestWithParam<Params> {};

TEST_P(ExtractModulePathTest, ExtractModulePath) {
  std::string_view fully_qualified_module_path;
  std::unordered_set<std::string_view> expected_module_paths;
  std::unordered_set<std::string_view> actual_module_paths;
  std::tie(fully_qualified_module_path, expected_module_paths) = GetParam();
  python::extract_modules_and_insert_into(
      fully_qualified_module_path, actual_module_paths);
  EXPECT_EQ(expected_module_paths, actual_module_paths);
}

INSTANTIATE_TEST_CASE_P(
    UtilTest,
    ExtractModulePathTest,
    ::testing::Values(
        Params(
            "typename", std::unordered_set<std::string_view>({})),
        Params(
            "module.path.typename[]",
            std::unordered_set<std::string_view>({"module.path"})),
        Params(
            "dotted.module.path.typename",
            std::unordered_set<std::string_view>({"dotted.module.path"})),
        Params(
            "generic[typename]", std::unordered_set<std::string_view>({})),
        Params(
            "generic[some.other.module.path.typename]",
            std::unordered_set<std::string_view>({"some.other.module.path"})),
        Params(
            "module.path.generic[some.other.module.path.typename]",
            std::unordered_set<std::string_view>(
                {"module.path", "some.other.module.path"})),
        Params(
            "module.path.generic[module.path.generic[some.other.module.path.typename]]",
            std::unordered_set<std::string_view>(
                {"module.path", "some.other.module.path"})),
        Params(
            "module.path.generic[module.path.generic[module2.path2.generic2[typename1, typename2]]]",
            std::unordered_set<std::string_view>(
                {"module.path", "module2.path2"})),
        Params(
            "module.path.generic[module.path.generic[module2.path2.generic2[some.other.module.path.typename]]]",
            std::unordered_set<std::string_view>(
                {"module.path", "module2.path2", "some.other.module.path"}))/*,
        TODO(T208028794) Should we support more complex use cases like these?
        Params(
            "module1.path1.generic1[module2.path2.generic2[typename2], module3.path3.generic3[typename3]]",
            std::unordered_set<std::string_view>(
                {"module1.path1", "module2.path2", "module3.path3"}))*/));
