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

TEST(CompilerTest, no_field_id) {
  check_compile(R"(
    struct NoLegacy {} (thrift.uri = "facebook.com/thrift/annotation/NoLegacy")
    struct Testing {} (thrift.uri = "facebook.com/thrift/annotation/Testing")
    struct Experimental {} (thrift.uri = "facebook.com/thrift/annotation/Experimental")

    @NoLegacy
    struct Foo {
        i32 field1; # expected-error: No field id specified for `field1`, resulting protocol may have conflicts or not be backwards compatible!
        @Experimental
        i32 field2; # expected-warning@-1: No field id specified for `field2`, resulting protocol may have conflicts or not be backwards compatible!
        @Testing
        i32 field3; # expected-warning@-1: No field id specified for `field3`, resulting protocol may have conflicts or not be backwards compatible!
    }

    struct Bar {
        i32 field4; # expected-warning: No field id specified for `field4`, resulting protocol may have conflicts or not be backwards compatible!
    }
  )");
}

TEST(CompilerTest, zero_as_field_id_annotation) {
  check_compile(R"(
    struct Foo {
      0: i32 field (cpp.deprecated_allow_zero_as_field_id);
          #expected-warning@-1: Nonpositive field id (0) differs from what is auto-assigned by thrift. The id must be positive or -1.
          #expected-warning@-2: No field id specified for `field`, resulting protocol may have conflicts or not be backwards compatible!
      1: list<i32> other;
    }

  )");
}

TEST(CompilerTest, zero_as_field_id_allow_neg_keys) {
  check_compile(
      R"(
    struct Foo {
      0: i32 field (cpp.deprecated_allow_zero_as_field_id);
          #expected-warning@-1: Nonpositive field id (0) differs from what would be auto-assigned by thrift (-1).
      1: list<i32> other;
    }

  )",
      {"--allow-neg-keys"});
}

TEST(CompilerTest, neg_field_ids) {
  check_compile(
      R"(
    struct Foo {
      i32 f1;  // auto id = -1
        #expected-warning@-1: No field id specified for `f1`, resulting protocol may have conflicts or not be backwards compatible!
      -2: i32 f2; // auto and manual id = -2
      -32: i32 f3; // min value.
        #expected-warning@-1: Nonpositive field id (-32) differs from what would be auto-assigned by thrift (-3).
      -33: i32 f4; // min value - 1.
        #expected-error@-1: Reserved field id (-33) cannot be used for `f4`.
    }
  )",
      {"--allow-neg-keys"});
}

TEST(CompilerTest, exhausted_neg_field_ids) {
  check_compile(
      R"(
    struct Foo {
      -32: i32 f1; // min value.
        #expected-warning@-1: Nonpositive field id (-32) differs from what would be auto-assigned by thrift (-1).
      i32 f2; // auto id = -2 or min value - 1
        #expected-error@-1: Cannot allocate an id for `f2`. Automatic field ids are exhausted.
    }
  )",
      {"--allow-neg-keys"});
}

TEST(CompilerTest, exhausted_pos_field_ids) {
  std::string thrift_struct;
  thrift_struct.reserve(10000);
  thrift_struct += "struct Foo {\n";
  for (int i = 0; i < 33; i++) {
    thrift_struct += "  i32 field_" + std::to_string(i) + ";\n";
  }
  thrift_struct += "}\n";
  thrift_struct +=
      "#expected-error@34: Cannot allocate an id for `field_32`. Automatic field ids are exhausted.";
  std::cout << thrift_struct << std::endl;
  check_compile(thrift_struct);
}

TEST(CompilerTest, out_of_range_field_ids_overflow) {
  check_compile(R"(
    struct Foo {
      -32768: i32 f1; #expected-warning: Nonpositive field id (-32768) differs from what is auto-assigned by thrift. The id must be positive or -1.
      32767: i32 f2;
      32768: i32 f3; #expected-error: Integer constant 32768 outside the range of field ids ([-32768, 32767]).
        #expected-warning@-1: Nonpositive field id (-32768) differs from what is auto-assigned by thrift. The id must be positive or -2.
    }
  )");
}

TEST(CompilerTest, out_of_range_field_ids_underflow) {
  check_compile(R"(
    struct Foo {
      -32768: i32 f1; #expected-warning: Nonpositive field id (-32768) differs from what is auto-assigned by thrift. The id must be positive or -1.
      32767: i32 f2;
      -32769: i32 f3; #expected-error: Integer constant -32769 outside the range of field ids ([-32768, 32767]).
        #expected-error@-1: Field id 32767 for `f3` has already been used.
    }
  )");
}
