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

#include <thrift/compiler/lib/schematizer.h>

#include <memory>
#include <unordered_map>
#include <utility>
#include <folly/portability/GTest.h>
#include <thrift/compiler/ast/t_base_type.h>
#include <thrift/compiler/ast/t_const.h>
#include <thrift/compiler/ast/t_function.h>
#include <thrift/compiler/ast/t_map.h>
#include <thrift/compiler/ast/t_paramlist.h>
#include <thrift/compiler/ast/t_program.h>
#include <thrift/compiler/ast/t_struct.h>

namespace apache::thrift::compiler {

TEST(SchematizerTest, WrapWithProtocolValue) {
  t_const_value str("foo");
  auto value = wrap_with_protocol_value(str, {});
  auto map = value->get_map();
  EXPECT_EQ(map.at(0).first->get_string(), "stringValue");
  EXPECT_EQ(map.at(0).second->get_string(), "foo");
}

} // namespace apache::thrift::compiler
