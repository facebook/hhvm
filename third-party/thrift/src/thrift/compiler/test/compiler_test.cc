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

TEST(CompilerTest, oneway_exception) {
  check_compile(R"(
    exception A {}

    service MyService {
        oneway void foo();
        oneway void baz() throws (1: A ex); # expected-error: Oneway methods can't throw exceptions: baz
    }
  )");
}

TEST(CompilerTest, oneway_return) {
  check_compile(R"(
    service MyService {
      oneway void foo();
      oneway string bar(); # expected-error: Oneway methods must have void return type: bar
    }
  )");
}

TEST(CompilerTest, enum_wrong_default_value) {
  check_compile(R"(
    enum Color {
      RED = 1,
      GREEN = 2,
      BLUE = 3,
    }

    struct MyS {
      1: Color color = -1; # expected-warning: type error: const `color` was declared as enum `Color` with a value not of that enum.
    }
  )");
}

TEST(CompilerTest, duplicate_enum_value_name) {
  check_compile(R"(
    enum Foo {
      Bar = 1,
      Bar = 2, # expected-error: Enum value `Bar` is already defined for `Foo`.
    }
  )");
}

TEST(CompilerTest, duplicate_enum_value) {
  check_compile(R"(
    enum Foo {
      Bar = 1,
      Baz = 1, # expected-error: Duplicate value `Baz=1` with value `Bar` in enum `Foo`.
    }
  )");
}

TEST(CompilerTest, unset_enum_value) {
  check_compile(R"(
    enum Foo {
      Foo = 1,
      Bar, # expected-error: The enum value, `Bar`, must have an explicitly assigned value.
      Baz, # expected-error: The enum value, `Baz`, must have an explicitly assigned value.
    }
  )");
}

TEST(CompilerTest, enum_overflow) {
  check_compile(R"(
    enum Foo {
      Bar = 2147483647
      Baz = 2147483648 # expected-error: Integer constant 2147483648 outside the range of enum values ([-2147483648, 2147483647]).
    }
  )");
}

TEST(CompilerTest, enum_underflow) {
  check_compile(R"(
    enum Foo {
      Bar = -2147483648
      Baz = -2147483649 # expected-error: Integer constant -2147483649 outside the range of enum values ([-2147483648, 2147483647]).
    }
  )");
}

TEST(CompilerTest, integer_overflow_underflow) {
  check_compile(R"(
    const i64 overflowInt = 9223372036854775808;  # max int64 + 1
      # expected-error@-1: integer constant 9223372036854775808 is too large
      # expected-warning@-2: 64-bit constant -9223372036854775808 may not work in all languages
  )");
  check_compile(R"(
    const i64 underflowInt = -9223372036854775809; # min int64 - 1
      # expected-error@-1: integer constant -9223372036854775809 is too small
      # expected-warning@-2: 64-bit constant 9223372036854775807 may not work in all languages
  )");
  check_compile(R"(
    # Unsigned Ints
    const i64 overflowUint = 18446744073709551615;  # max uint64
      # expected-error@-1: integer constant 18446744073709551615 is too large
  )");
  check_compile(R"(
    const i64 overflowUint2 = 18446744073709551616;  # max uint64 + 1
      # expected-error@-1: integer constant 18446744073709551616 is too large
  )");
}

TEST(CompilerTest, double_overflow_underflow) {
  check_compile(R"(
    const double overflowConst = 1.7976931348623159e+308;
      # expected-error@-1: floating-point constant 1.7976931348623159e+308 is out of range
  )");
  check_compile(R"(
    const double overflowConst = 1.7976931348623159e+309;
      # expected-error@-1: floating-point constant 1.7976931348623159e+309 is out of range
  )");
  check_compile(R"(
    const double overflowConst = 4.9406564584124654e-325;
      # expected-error@-1: magnitude of floating-point constant 4.9406564584124654e-325 is too small
  )");
  check_compile(R"(
    const double overflowConst = 1e-324;
      # expected-error@-1: magnitude of floating-point constant 1e-324 is too small
  )");
}

TEST(CompilerTest, const_wrong_type) {
  check_compile(R"(
    const i32 wrongInt = "stringVal" # expected-error: type error: const `wrongInt` was declared as i32.
    const set<string> wrongSet = {1: 2}
      # expected-warning@-1: type error: const `wrongSet` was declared as set. This will become an error in future versions of thrift.
    const map<i32, i32> wrongMap = [1,32,3];
      # expected-warning@-1: type error: const `wrongMap` was declared as map. This will become an error in future versions of thrift.
    const map<i32, i32> wierdMap = [];
      # expected-warning@-1: type error: map `wierdMap` initialized with empty list.
    const set<i32> wierdSet = {};
      # expected-warning@-1: type error: set `wierdSet` initialized with empty map.
    const list<i32> wierdList = {};
      # expected-warning@-1: type error: list `wierdList` initialized with empty map.
    const list<string> badValList = [1]
      # expected-error@-1: type error: const `badValList<elem>` was declared as string.
    const set<string> badValSet = [2]
      # expected-error@-1: type error: const `badValSet<elem>` was declared as string.
    const map<string, i32> badValMap = {1: "str"}
      # expected-error@-1: type error: const `badValMap<key>` was declared as string.
      # expected-error@-2: type error: const `badValMap<val>` was declared as i32.
  )");
}

TEST(CompilerTest, struct_fields_wrong_type) {
  check_compile(R"(
    struct Annot {
      1: i32 val
      2: list<string> otherVal
    }

    @Annot{val="hi", otherVal=5}
      #expected-error@-1: type error: const `.val` was declared as i32.
      #expected-warning@-2: type error: const `.otherVal` was declared as list. This will become an error in future versions of thrift.
    struct BadFields {
      1: i32 badInt = "str" # expected-error: type error: const `badInt` was declared as i32.
    }
  )");
}

TEST(CompilerTest, duplicate_method_name) {
  check_compile(R"(
    service MySBB {
      void lol(),
      i32 lol(), # expected-error:  Function `lol` is already defined for `MySBB`.
    }
  )");
}

TEST(CompilerTest, nonexistent_type) {
  check_compile(R"(
    struct S {
      1: Random.Type field # expected-error: Type `Random.Type` not defined.
    }
  )");
}

TEST(CompilerTest, field_names_uniqueness) {
  check_compile(R"(
    struct S {
      1: i32 a;
      2: i32 b;
      3: i32 a; # expected-error: Field `a` is already defined for `S`.
    }
  )");
}

TEST(CompilerTest, mixin_field_names_uniqueness) {
  check_compile(R"(
    struct A { 1: i32 i }
    struct B { 2: i64 i }
    struct C {
      1: A a (cpp.mixin);
      2: B b (cpp.mixin); # expected-error: Field `B.i` and `A.i` can not have same name in `C`.
    }
  )");
  check_compile(R"(
    struct A { 1: i32 i }

    struct C {
      1: A a (cpp.mixin);
      2: i64 i; # expected-error: Field `C.i` and `A.i` can not have same name in `C`.
    }
  )");
}
