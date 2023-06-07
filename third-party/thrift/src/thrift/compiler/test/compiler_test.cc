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

#include <thrift/compiler/test/compiler.h>

#include <gtest/gtest.h>

using apache::thrift::compiler::test::check_compile;

TEST(CompilerTest, double_package) {
  check_compile(R"(
    package "test.dev/test"
    package "test.dev/test" # expected-error: Package already specified.
  )");
}

TEST(CompilerTest, diagnostic_in_last_line) {
  check_compile(
      R"(#expected-error: Parser error during include pass.
    struct s {
      1: i32 i;
# expected-error: expected type)");
}

TEST(CompilerTest, missing_type_definition) {
  check_compile(
      R"(
      struct s{
        1: i32 i;
        2: myStruct ms; # expected-error: Type `test.myStruct` not defined.
      }
)");
}

TEST(CompilerTest, zero_as_field_id) {
  check_compile(R"(
    struct Foo {
        0: i32 field; #expected-warning: Nonpositive field id (0) differs from what is auto-assigned by thrift. The id must be positive or -1.
                      #expected-warning@3:  No field id specified for `field`, resulting protocol may have conflicts or not be backwards compatible!
        1: list<i32> other;
    }
)");
}

TEST(CompilerTest, zero_as_field_id_neg_keys) {
  check_compile(
      R"(
    struct Foo {
        0: i32 field; #expected-warning: Nonpositive field id (0) differs from what would be auto-assigned by thrift (-1).
                      #expected-error@-1: Zero value (0) not allowed as a field id for `field`
        1: list<i32> other;
    }
)",
      {"--allow-neg-keys"});
}
