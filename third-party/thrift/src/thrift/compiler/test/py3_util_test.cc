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

#include <folly/portability/GTest.h>

#include <thrift/compiler/ast/t_base_type.h>
#include <thrift/compiler/ast/t_field.h>
#include <thrift/compiler/lib/py3/util.h>

using namespace apache::thrift::compiler;

TEST(UtilTest, get_py3_name) {
  EXPECT_EQ("foo", py3::get_py3_name(t_field(t_base_type::t_i32(), "foo")));
  EXPECT_EQ("True_", py3::get_py3_name(t_field(t_base_type::t_i32(), "True")));
  EXPECT_EQ(
      "cpdef_", py3::get_py3_name(t_field(t_base_type::t_i32(), "cpdef")));

  t_field f(t_base_type::t_i32(), "foo");
  f.set_annotation("py3.name", "bar");
  EXPECT_EQ("bar", py3::get_py3_name(f));
}
