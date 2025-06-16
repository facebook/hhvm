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

#include <filesystem>
#include <string>
#include <thrift/compiler/test/compiler.h>

#include <gtest/gtest.h>

using apache::thrift::compiler::test::check_compile;
using apache::thrift::compiler::test::check_compile_options;

// Note: To see a reference on the expected lint message format: see the regex
// in thrift/compiler/test/compiler.cc
TEST(CompilerTest, diagnostic_in_last_line) {
  check_compile(R"(
    struct S {
      1: i32 i;
      # expected-error: expected type)");
}

TEST(CompilerTest, absolute_line_number) {
  check_compile(R"(
    foo
    # expected-error@2: expected definition
    )");
}

TEST(CompilerTest, double_package) {
  check_compile(R"(
    package "test.dev/test"
    package "test.dev/test" # expected-error: Package already specified.
  )");
}

TEST(CompilerTest, missing_type_definition) {
  check_compile(R"(
    struct S {
      1: i32 i;
      2: MyStruct ms; # expected-error: Type `MyStruct` not defined.
    }
  )");
}

TEST(CompilerTest, redefinition) {
  check_compile(R"(
    struct A {}
    struct A {}      # expected-error: redefinition of 'A'
    union A {}       # expected-error: redefinition of 'A'
    exception A {}   # expected-error: redefinition of 'A'
    enum A {}        # expected-error: redefinition of 'A'
    typedef i32 A;   # expected-error: redefinition of 'A'
    service A {}     # expected-error: redefinition of 'A'
    interaction A {} # expected-error: redefinition of 'A'
    const i32 A = 0; # expected-error: redefinition of 'A'
  )");
}

TEST(CompilerTest, zero_as_field_id) {
  check_compile(R"(
    struct Foo {
        0: i32 field; # expected-warning: Nonpositive field id (0) differs from what would be auto-assigned by thrift (if 'allow-neg-keys' was disabled): -1
                      # expected-error@-1: Zero value (0) not allowed as a field id for `field`
      1: list<i32> other;
    }
  )");
}

// NOTE: As of Nov 2024, allow-neg-keys is ignored, so this should be identical
// to the "zero_as_field_id" case above.
TEST(CompilerTest, zero_as_field_id_neg_keys) {
  check_compile(
      R"(
      struct Foo {
        0: i32 field; # expected-warning: Nonpositive field id (0) differs from what would be auto-assigned by thrift (if 'allow-neg-keys' was disabled): -1
                      # expected-error@-1: Zero value (0) not allowed as a field id for `field`
        1: list<i32> other;
      }
      )",
      {"--allow-neg-keys"});
}

TEST(CompilerTest, no_field_id) {
  check_compile(
      R"(
    struct Experimental {} (thrift.uri = "facebook.com/thrift/annotation/Experimental") # expected-warning: The annotation thrift.uri is deprecated. Please use @thrift.Uri instead.
    struct Foo {
      @Experimental
      i32 field2; # expected-error@-1: No field id specified for `field2`
    }

    struct Bar {
      i32 field4; # expected-error: No field id specified for `field4`
    })");
}

TEST(CompilerTest, zero_as_field_id_annotation) {
  check_compile(R"(
    struct Foo {
      0: i32 field (cpp.deprecated_allow_zero_as_field_id);
        # expected-warning@-1: Nonpositive field id (0) differs from what would be auto-assigned by thrift (if 'allow-neg-keys' was disabled): -1

      1: list<i32> other;
    }
  )");
}

TEST(CompilerTest, neg_field_ids) {
  check_compile(
      R"(
      struct Foo {
        i32 f1;  // auto id = -1
          # expected-error@-1: No field id specified for `f1`

        -2: i32 f2; // auto and manual id = -2
        -32: i32 f3; // min value.
          # expected-warning@-1: Nonpositive field id (-32) differs from what would be auto-assigned by thrift (if 'allow-neg-keys' was disabled): -3

        -33: i32 f4; // min value - 1.
          # expected-error@-1: Reserved field id (-33) cannot be used for `f4`.
      }
      )");
}

TEST(CompilerTest, exhausted_neg_field_ids) {
  check_compile(
      R"(
      struct Foo {
        -32: i32 f1; // min value.
          # expected-warning@-1: Nonpositive field id (-32) differs from what would be auto-assigned by thrift (if 'allow-neg-keys' was disabled): -1

        i32 f2; // auto id = -2 or min value - 1
          # expected-error@-1: Cannot allocate an id for `f2`. Automatic field ids are exhausted.
      }
      )");
}

TEST(CompilerTest, exhausted_pos_field_ids) {
  std::string src;
  src += "struct Foo {\n";
  for (int i = 0; i < 33; i++) {
    src += "  i32 field_" + std::to_string(i) + ";\n";
  }
  src +=
      "# expected-error@-1: Cannot allocate an id for `field_32`. "
      "Automatic field ids are exhausted.\n";
  src += "}\n";
  check_compile(src);
}

TEST(CompilerTest, out_of_range_field_ids_overflow) {
  check_compile(R"(
    struct Foo {
      -32768: i32 f1; # expected-warning: Nonpositive field id (-32768) differs from what would be auto-assigned by thrift (if 'allow-neg-keys' was disabled): -1
      32767: i32 f2;
      32768: i32 f3; # expected-error: Integer constant 32768 outside the range of field ids ([-32768, 32767]).
        # expected-error@-1: Field id -32768 for `f3` has already been used.
        # expected-warning@-2: Nonpositive field id (-32768) differs from what would be auto-assigned by thrift (if 'allow-neg-keys' was disabled): 32767
    }
  )");
}

TEST(CompilerTest, out_of_range_field_ids_underflow) {
  check_compile(R"(
    struct Foo {
      -32768: i32 f1; # expected-warning: Nonpositive field id (-32768) differs from what would be auto-assigned by thrift (if 'allow-neg-keys' was disabled): -1
      32767: i32 f2;

      -32769: i32 f3; # expected-error: Integer constant -32769 outside the range of field ids ([-32768, 32767]).
                      # expected-error@-1: Field id 32767 for `f3` has already been used.
    }
  )");
}

TEST(CompilerTest, params) {
  check_compile(R"(
    service MyService {
      void good(1: i32 i);
      void semi(1: i32 i;); # expected-warning: unexpected ';'
      void opt(1: optional i32 i); # expected-warning: 'optional' is not permitted on a parameter
      void req(1: required i32 i); # expected-warning: 'required' is not permitted on a parameter
    }
  )");
}

TEST(CompilerTest, oneway_exception) {
  check_compile(R"(
    exception A {}

    service MyService {
      oneway void foo();
      oneway void baz() throws (1: A ex); # expected-error: oneway function can't throw exceptions
    }
  )");
}

TEST(CompilerTest, oneway_return) {
  check_compile(R"(
    interaction I {}

    service S {
      oneway void foo();
      oneway string bar(); # expected-error: oneway function must return 'void'
      oneway I baz();      # expected-error: oneway function must return 'void'
    }
  )");
}

TEST(CompilerTest, return_sink) {
  check_compile(R"(
    service S {
      sink<i32, i32> getSink();
      i32, sink<i32, i32> getStreamWithInitialResponse();
      sink<i32, i32>, i32 bad(); # expected-error: expected identifier
    }
  )");
}

TEST(CompilerTest, return_stream) {
  check_compile(R"(
    service S {
      stream<i32> getStream();
      i32, stream<i32> getStreamWithInitialResponse();
      stream<i32>, i32 bad(); # expected-error: expected identifier
    }
  )");
}

TEST(CompilerTest, return_interaction) {
  check_compile(R"(
    interaction I {
      void foo();
    }

    service S {
      I, i32 good();
      i32, I bad(); # expected-error: expected 'sink' or 'stream' after the initial response type
    }
  )");
}

TEST(CompilerTest, void_as_initial_response_type) {
  check_compile(R"(
    service S {
      void, stream<i32> getStream(); # expected-error: cannot use 'void' as an initial response type
      void, sink<i32, i32> getSink(); # expected-error: cannot use 'void' as an initial response type
    }
  )");
}

TEST(CompilerTest, enum) {
  check_compile(R"(
    enum Color {
      RED = 1,
      GREEN = 2; # expected-warning: unexpected ';'
      BLUE = 3,
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
      1: Color color = -1; # expected-warning: const `color` is defined as enum `Color` with a value not of that enum
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
      Bar = 2147483647,
      Baz = 2147483648 # expected-error: Integer constant 2147483648 outside the range of enum values ([-2147483648, 2147483647]).
    }
  )");
}

TEST(CompilerTest, enum_underflow) {
  check_compile(R"(
    enum Foo {
      Bar = -2147483648,
      Baz = -2147483649 # expected-error: Integer constant -2147483649 outside the range of enum values ([-2147483648, 2147483647]).
    }
  )");
}

TEST(CompilerTest, integer_overflow_underflow) {
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

TEST(CompilerTest, void_data) {
  check_compile(R"(
    const void v = 42; # expected-error: `void` cannot be used as a data type
    struct S {
      1: void v; # expected-error: `void` cannot be used as a data type
    }
  )");
}

TEST(CompilerTest, byte_initializer) {
  check_compile(R"(
    const byte c1 = 127;
    const byte c2 = 128;
    # expected-error@-1: 128 is out of range for `byte` in initialization of `c2`

    const byte c3 = -128;
    const byte c4 = -129;
    # expected-error@-1: -129 is out of range for `byte` in initialization of `c4`
  )");
}

TEST(CompilerTest, i16_initializer) {
  check_compile(R"(
    const i16 c1 = 32767;
    const i16 c2 = 32768;
    # expected-error@-1: 32768 is out of range for `i16` in initialization of `c2`

    const i16 c3 = -32768;
    const i16 c4 = -32769;
    # expected-error@-1: -32769 is out of range for `i16` in initialization of `c4`
  )");
}

TEST(CompilerTest, i32_initializer) {
  check_compile(R"(
    const i32 c0 = 42;

    const i32 c1 = 4.2;
    # expected-error@-1: cannot convert floating-point number to `i32` in initialization of `c1`

    const i32 c2 = "string typing";
    # expected-error@-1: cannot convert string to `i32` in initialization of `c2`

    const i32 c3 = 2147483647;
    const i32 c4 = 2147483648;
    # expected-error@-1: 2147483648 is out of range for `i32` in initialization of `c4`

    const i32 c5 = -2147483648;
    const i32 c6 = -2147483649;
    # expected-error@-1: -2147483649 is out of range for `i32` in initialization of `c6`
  )");
}

TEST(CompilerTest, float_initializer) {
  check_compile(R"(
    const float c0 = 1e8;

    const float c1 = 3.4028234663852886e+38;
    const float c2 = 3.402823466385289e+38; // max float + 1 double ulp
    # expected-error@-1: 3.402823466385289e+38 is out of range for `float` in initialization of `c2`

    const float c3 = -3.4028234663852886e+38;
    const float c4 = -3.402823466385289e+38; // min float - 1 double ulp
    # expected-error@-1: -3.402823466385289e+38 is out of range for `float` in initialization of `c4`

    const float c5 = 100000001;
    # expected-error@-1: cannot convert 100000001 to `float` in initialization of `c5`
    const float c6 = -100000001;
    # expected-error@-1: cannot convert -100000001 to `float` in initialization of `c6`
  )");
}

TEST(CompilerTest, double_initializer) {
  check_compile(R"(
    const double c0 = 1e8;
    const double c1 = 1.7976931348623157e+308;
    const double c2 = -1.7976931348623157e+308;

    const double c3 = 10000000000000001;
    # expected-error@-1: cannot convert 10000000000000001 to `double` in initialization of `c3`

    const double c4 = -10000000000000001;
    # expected-error@-1: cannot convert -10000000000000001 to `double` in initialization of `c4`
  )");
}

TEST(CompilerTest, binary_initializer) {
  check_compile(R"(
    const binary b0 = "foo";

    const binary b1 = 42;
    # expected-error@-1: cannot convert integer to `binary` in initialization of `b1`
  )");
}

TEST(CompilerTest, struct_initializer) {
  check_compile(R"(
    struct S {}
    const S s1 = {}; # OK

    const S s2 = 42;
    # expected-error@-1: cannot convert integer to `S` in initialization of `s2`

    const S s3 = 4.2;
    # expected-error@-1: cannot convert floating-point number to `S` in initialization of `s3`

    const S s4 = "";
    # expected-error@-1: cannot convert string to `S` in initialization of `s4`

    const S s5 = true;
    # expected-error@-1: cannot convert bool to `S` in initialization of `s5`

    const S s6 = [];
    # expected-error@-1: cannot convert list to `S` in initialization of `s6`

    struct A {}
    struct B {}
    const A a = B{};
    # expected-error@-1: type mismatch: expected test.A, got test.B
  )");
}

TEST(CompilerTest, union_initializer) {
  check_compile(R"(
    union U {
      1: i32 a;
      2: i32 b;
    }

    const U u1 = {}; # OK

    const U u2 = 42;
    # expected-error@-1: cannot convert integer to `U` in initialization of `u2`

    const U u3 = 4.2;
    # expected-error@-1: cannot convert floating-point number to `U` in initialization of `u3`

    const U u4 = "";
    # expected-error@-1: cannot convert string to `U` in initialization of `u4`

    const U u5 = true;
    # expected-error@-1: cannot convert bool to `U` in initialization of `u5`

    const U u6 = [];
    # expected-error@-1: cannot convert list to `U` in initialization of `u6`

    const U u7 = {"a": 1, "b": 2};
    # expected-error@-1: cannot initialize more than one field in union `U`
  )");
}

TEST(CompilerTest, enum_initializer) {
  check_compile(R"(
    enum E {A = 1}

    const E e1 = E.A; # OK

    const E e2 = 42;
    # expected-warning@-1: const `e2` is defined as enum `E` with a value not of that enum

    const E e3 = 4.2;
    # expected-error@-1: cannot convert floating-point number to `E` in initialization of `e3`

    const E e4 = "";
    # expected-error@-1: cannot convert string to `E` in initialization of `e4`

    const E e5 = "E.A";
    # expected-error@-1: cannot convert string to `E` in initialization of `e5`

    const E e6 = true;
    # expected-error@-1: cannot convert bool to `E` in initialization of `e6`

    const E e7 = [];
    # expected-error@-1: cannot convert list to `E` in initialization of `e7`
  )");
}

TEST(CompilerTest, list_initializer) {
  check_compile(R"(
    const list<i32> weirdList = {};
    # expected-warning@-1: converting empty map to `list` in initialization of `weirdList`

    const list<string> badValList = [1];
    # expected-error@-1: cannot convert integer to `string` in initialization of `badValList`
  )");
}

TEST(CompilerTest, set_initializer) {
  check_compile(R"(
    const set<string> wrongSet = {1: 2};
    # expected-error@-1: cannot convert map to `set` in initialization of `wrongSet`

    const set<i32> weirdSet = {};
    # expected-warning@-1: converting empty map to `set` in initialization of `weirdSet`

    const set<string> badValSet = [2];
    # expected-error@-1: cannot convert integer to `string` in initialization of `badValSet`
  )");
}

TEST(CompilerTest, map_initializer) {
  check_compile(R"(
    const map<i32, i32> wrongMap = [1, 32, 3];
    # expected-error@-1: cannot convert list to `map` in initialization of `wrongMap`

    const map<i32, i32> weirdMap = [];
    # expected-warning@-1: converting empty list to `map` in initialization of `weirdMap`

    const map<string, i32> badValMap = {1: "str"};
    # expected-error@-1: cannot convert integer to `string` in initialization of `badValMap`
    # expected-error@-2: cannot convert string to `i32` in initialization of `badValMap`
  )");
}

TEST(CompilerTest, map_item_separator) {
  check_compile(R"(
    const map<i32, i32> m = {
      1: 2,
      3: 4; # expected-error: expected }
      5: 6
    };
  )");
}

TEST(CompilerTest, struct_fields_wrong_type) {
  check_compile(R"(
    struct Annot {
      1: i32 val;
      2: list<string> otherVal;
      3: F structField;
    }
    struct F {}
    struct G {}

    @Annot{val="hi", otherVal=5, structField=G{}}
      # expected-error@-1: cannot convert string to `i32` in initialization of `val`
      # expected-error@-2: cannot convert integer to `list` in initialization of `otherVal`
      # expected-error@-3: type mismatch: expected test.F, got test.G
    struct BadFields {
      1: i32 badInt = "str";
        # expected-error@-1: cannot convert string to `i32` in initialization of `badInt`
      2: F badStruct = G{}; # expected-error: type mismatch: expected test.F, got test.G
    }
  )");
}

TEST(CompilerTest, duplicate_method_name) {
  check_compile(R"(
    service MySBB {
      void lol();
      i32 lol(); # expected-error: Function `lol` is already defined for `MySBB`.
    }
  )");
}

TEST(CompilerTest, undefined_type) {
  check_compile(R"(
    struct S {
      1: bad.Type field; # expected-error: Type `bad.Type` not defined.
    }
  )");
}

TEST(CompilerTest, undefined_const) {
  check_compile(R"(
    const Type c = 42; # expected-error: Type `Type` not defined.
  )");
}

TEST(CompilerTest, undefined_const_external) {
  check_compile(R"(
    const bad.Type d = 42; # expected-error: Type `bad.Type` not defined.
  )");
}

TEST(CompilerTest, undefined_annotation) {
  check_compile(R"(
    @BadAnnotation # expected-error: Type `BadAnnotation` not defined.
    struct S {}
  )");
}

TEST(CompilerTest, undefined_annotation_external) {
  check_compile(R"(
    @bad.Annotation # expected-error: Type `bad.Annotation` not defined.
    struct T {}
  )");
}

TEST(CompilerTest, field_name_uniqueness) {
  check_compile(R"(
    struct S {
      1: i32 a;
      2: i32 b;
      3: i32 a; # expected-error: Field `a` is already defined for `S`.
    }
  )");
}

TEST(CompilerTest, mixin_field_name_uniqueness) {
  check_compile(R"(
    struct A { 1: i32 i; }
    struct B { 2: i64 i; }
    struct C {
      1: A a (cpp.mixin); # expected-warning: The annotation cpp.mixin is deprecated. Please use @thrift.Mixin instead.
      2: B b (cpp.mixin); # expected-error: Field `B.i` and `A.i` can not have same name in `C`.
    } # expected-warning@-1: The annotation cpp.mixin is deprecated. Please use @thrift.Mixin instead.
  )");
  check_compile(R"(
    struct A { 1: i32 i; }

    struct C {
      1: A a (cpp.mixin); # expected-warning: The annotation cpp.mixin is deprecated. Please use @thrift.Mixin instead.
      2: i64 i; # expected-error: Field `C.i` and `A.i` can not have same name in `C`.
    }
  )");
}

TEST(CompilerTest, function_exception_field_name_uniqueness) {
  check_compile(R"(
    exception Ex1 {}
    exception Ex2 {}

    service S {
      void foo() throws (
        1: Ex1 e,
        2: Ex2 e, # expected-error: Exception field `e` is already defined for `foo`.
      );
    }
  )");
}

TEST(CompilerTest, annotation_positions) {
  check_compile(R"(
    struct Type {1: string name} (thrift.uri = "facebook.com/thrift/annotation/Type") # expected-warning: The annotation thrift.uri is deprecated. Please use @thrift.Uri instead.
    typedef set<set<i32> (annot)> T # expected-error: Annotations are not allowed in this position. Extract the type into a named typedef instead.
    const i32 (annot) C = 42 # expected-error: Annotations are not allowed in this position. Extract the type into a named typedef instead.
    service S {
      i32 (annot) foo() # expected-error: Annotations are not allowed in this position. Extract the type into a named typedef instead.
      void bar(1: i32 (annot) p) # expected-error: Annotations are not allowed in this position. Extract the type into a named typedef instead.
      void qux() throws (1: E (annot) e) # expected-error: Annotations are not allowed in this position. Extract the type into a named typedef instead.
      void baz() throws (1: E e (annot)) # expected-error: Annotations are not allowed in this position. Extract the type into a named typedef instead.
    }
    struct Foo {
      1: i32 (annot) f
      # expected-error@-1: Annotations are not allowed in this position. Extract the type into a named typedef instead.
      @Type{name="foo"}
      2: i32 g
    }
    exception E {}
  )");
}

TEST(CompilerTest, annotation_positions_field) {
  check_compile(
      R"(
    struct Foo {
      1: i32 (annot) f # expected-error: Annotations are not allowed in this position. Extract the type into a named typedef instead.
      2: i32 g
    }
  )");
}

TEST(CompilerTest, performs_in_interaction) {
  check_compile(R"(
    interaction J {}
    interaction I {
      performs J; # expected-error: cannot use 'performs' in an interaction
    }
  )");
}

TEST(CompilerTest, interactions) {
  check_compile(R"(
    interaction J {}

    interaction I {
      J foo(); # expected-error: Nested interactions are forbidden: foo
    }

    service T {
      performs I;
      performs I; # expected-error: Function `createI` is already defined for `T`.
    }
  )");
}

TEST(CompilerTest, invalid_performs) {
  check_compile(R"(
    struct S {}

    service T {
      performs S; # expected-error: expected interaction name
    }
  )");
}

TEST(CompilerTest, interactions_as_first_response_type) {
  check_compile(R"(
    interaction I {}
    interaction J {}

    service S {
      I, J foo();                 # expected-error: Invalid first response type: test.J
      I, J, stream<i32> bar();    # expected-error: Invalid first response type: test.J
      I, J, sink<i32, i32> baz(); # expected-error: Invalid first response type: test.J
    }
  )");
}

TEST(CompilerTest, deprecated_annotations) {
  check_compile(R"(
    include "thrift/annotation/cpp.thrift"
    include "thrift/annotation/hack.thrift"
    include "thrift/annotation/thrift.thrift"

    @hack.Attributes{attributes=[]} # expected-error: Duplicate annotations hack.attributes and @hack.Attributes.
    struct A {
        1: optional i64 field (cpp.box) # expected-warning: The annotation cpp.box is deprecated. Please use @thrift.Box instead.
        2: i64 with (py3.name = "w", go.name = "w") # expected-warning: The annotation py3.name is deprecated. Please use @python.Name instead.
        # expected-warning@-1: The annotation go.name is deprecated. Please use @go.Name instead.
    } (hack.attributes = "")

    typedef i64 (cpp.type = "std::uint64_t") T # expected-warning: The annotation cpp.type is deprecated. Please use @cpp.Type instead.

    # This should not produce a warning even though the annotation is currently lowered.
    @cpp.Type{name = "std::uint64_t"}
    typedef i64 T2
  )");
}

TEST(CompilerTest, removed_annotations) {
  check_compile(R"(
    include "thrift/annotation/thrift.thrift"

    enum E1 {} (cpp2.declare_bitwise_ops)
    # expected-error@-1: The annotation cpp2.declare_bitwise_ops has been removed. Please use @thrift.BitmaskEnum instead.

    @thrift.DeprecatedUnvalidatedAnnotations{items = {"cpp2.declare_bitwise_ops": "1"}}
    enum E2 {}
    # expected-error@-2: The annotation cpp2.declare_bitwise_ops has been removed. Please use @thrift.BitmaskEnum instead.

    enum E3 {} (cpp2.deprecated_enum_unscoped)
    # expected-error@-1: invalid annotation cpp2.deprecated_enum_unscoped

    struct A {
      1: i64 removed_unstructured (rust.foo)
      # expected-error@-1: The annotation rust.foo has been removed. Please use a structured annotation from thrift/annotation/rust.thrift instead.
      @thrift.DeprecatedUnvalidatedAnnotations{items = {"rust.foo": "1"}}
      2: i64 removed_catchall
      # expected-error@-2: The annotation rust.foo has been removed. Please use a structured annotation from thrift/annotation/rust.thrift instead.
    }

    typedef map<string, string> (rust.type = "HashMap") HashMap
    # expected-error@-1: The annotation rust.type has been removed. Please use a structured annotation from thrift/annotation/rust.thrift instead.

    struct S {} (cpp.type = "foo") # expected-warning: The annotation cpp.type is deprecated. Please use @cpp.Type instead.
    typedef S T

    service J {
      i64 foo(1: i64 arg (rust.name = "bar")) # expected-error: The annotation rust.name has been removed. Please use a structured annotation from thrift/annotation/rust.thrift instead.
    }
  )");
}

TEST(CompilerTest, invalid_enum_constant) {
  check_compile(R"(
    enum E {}
    const list<E> c = [nonexistent.Value]; # expected-error: use of undeclared identifier 'nonexistent.Value'
  )");
}

TEST(CompilerTest, invalid_struct_constant) {
  check_compile(R"(
    struct S {}
    const list<S> c = [nonexistent.Value]; # expected-error: use of undeclared identifier 'nonexistent.Value'
  )");
}

TEST(CompilerTest, cpp_type_compatibility) {
  check_compile(R"(
    include "thrift/annotation/cpp.thrift"

    @cpp.Adapter{name="Adapter"} # expected-error: Definition `Bar1` cannot have both cpp.type/cpp.template and @cpp.Adapter annotations
    typedef i32 Bar1 (cpp.type = "std::uint32_t") # expected-warning@-1: The annotation cpp.type is deprecated. Please use @cpp.Type instead.

    @cpp.Adapter{name="Adapter"} # expected-error: Definition `A` cannot have both cpp.type/cpp.template and @cpp.Adapter annotations
    struct A {
      1: i32 field;
    } (cpp.type = "CustomA") # expected-warning@-3: The annotation cpp.type is deprecated. Please use @cpp.Type instead.

    struct B {
      @cpp.Adapter{name="Adapter"} # expected-warning: At most one of @cpp.Type/@cpp.Adapter/cpp.type/cpp.template can be specified on a definition.
      1: i32 (cpp.type = "std::uint32_t") field;
      # expected-error@-2: Annotations are not allowed in this position. Extract the type into a named typedef instead.
      @cpp.Adapter{name="Adapter"} # expected-warning: At most one of @cpp.Type/@cpp.Adapter/cpp.type/cpp.template can be specified on a definition.
      @cpp.Type{name="std::uint32_t"}
      2: i32 field2;
    }
  )");
}

TEST(CompilerTest, duplicate_method_name_base_base) {
  std::map<std::string, std::string> name_contents_map;
  name_contents_map["foo.thrift"] = R"(
    service MySBB {
      void lol();
    }
  )";

  name_contents_map["bar.thrift"] = R"(
    include "foo.thrift"

    service MySB extends foo.MySBB {
      void meh();
    }
  )";

  name_contents_map["baz.thrift"] = R"(
    include "bar.thrift"

    service MyS extends bar.MySB {
      void lol(); # expected-error: Function `MyS.lol` redefines `foo.MySBB.lol`.
      void meh(); # expected-error: Function `MyS.meh` redefines `bar.MySB.meh`.
    }
  )";

  check_compile(name_contents_map, "baz.thrift");
}

TEST(CompilerTest, circular_include_dependencies) {
  std::map<std::string, std::string> name_contents_map;
  name_contents_map["foo.thrift"] = R"(
    include "bar.thrift"
  )";
  name_contents_map["bar.thrift"] = R"(
    include "foo.thrift"
      # expected-error@-1: Circular dependency found: file `foo.thrift` is already parsed.
  )";

  check_compile(name_contents_map, "foo.thrift");
}

TEST(CompilerTest, mixins_and_refs) {
  check_compile(R"(
    struct C {
      1: i32 f1 (cpp.mixin); # expected-warning: The annotation cpp.mixin is deprecated. Please use @thrift.Mixin instead.
        # expected-error@-1: Mixin field `f1` type must be a struct or union. Found `i32`.
    }

    struct D { 1: i32 i }
    union E {
      1: D a (cpp.mixin); # expected-warning: The annotation cpp.mixin is deprecated. Please use @thrift.Mixin instead.
        # expected-error@-1: Union `E` cannot contain mixin field `a`.
    }

    struct F {
      1: optional D a (cpp.mixin); # expected-warning: The annotation cpp.mixin is deprecated. Please use @thrift.Mixin instead.
        # expected-error@-1: Mixin field `a` cannot be optional.
    }
  )");
}

TEST(CompilerTest, bitpack_with_tablebased_seriliazation) {
  check_compile(
      R"(
    include "thrift/annotation/cpp.thrift"
    struct A { 1: i32 i }
    @cpp.PackIsset
      # expected-error@-1: Tablebased serialization is incompatible with isset bitpacking for struct `D`  [tablebased-isset-bitpacking-rule]
    struct D { 1: i32 i }
  )",
      {"--gen", "mstch_cpp2:json,tablebased"});
}

TEST(CompilerTest, structured_annotations_uniqueness) {
  std::map<std::string, std::string> name_contents_map;
  name_contents_map["foo.thrift"] = R"( struct Foo {} )";
  name_contents_map["bar.thrift"] = R"(
    include "foo.thrift"
    struct Foo {
        1: i32 count;
    }

    # TODO(afuller): Fix t_scope to not include the locally defined Foo as
    # `foo.Foo`, which override the included foo.Foo definition.

    @foo.Foo
    @Foo{count=1}
    @Foo{count=2}
     # expected-error@-1: Structured annotation `Foo` is already defined for `Annotated`.
    typedef i32 Annotated
)";
  check_compile(name_contents_map, "bar.thrift");
}

TEST(CompilerTest, structured_annotations_type_resolved) {
  check_compile(R"(
    struct Annotation {
        1: i32 count;
        2: TooForward forward;
    }

    @Annotation{count=1, forward=TooForward{name="abc"}}
    struct Annotated {
        1: string name;
    }

    struct TooForward {
      1: string name;
    }
)");
}

TEST(CompilerTest, structured_ref) {
  check_compile(R"(
    include "thrift/annotation/cpp.thrift"

    struct Foo {
      1: optional Foo field1 (cpp.ref);
        # expected-warning@-1: cpp.ref, cpp2.ref are deprecated. Please use @thrift.Box annotation instead in `field1`.

      @cpp.Ref{type = cpp.RefType.Unique}
        # expected-warning@-1: @cpp.Ref{type = cpp.RefType.Unique} is deprecated. Please use @thrift.Box annotation instead in `field2`.
      2: optional Foo field2;

      @cpp.Ref{type = cpp.RefType.Unique}
        # expected-error@-1: The @cpp.Ref annotation cannot be combined with the `cpp.ref` or `cpp.ref_type` annotations. Remove one of the annotations from `field3`.
        # expected-warning@-2: cpp.ref, cpp2.ref are deprecated. Please use @thrift.Box annotation instead in `field3`.
        # expected-warning@-3: @cpp.Ref{type = cpp.RefType.Unique} is deprecated. Please use @thrift.Box annotation instead in `field3`.
      3: optional Foo field3 (cpp.ref);

      @cpp.Ref{type = cpp.RefType.Unique}
      @cpp.Ref{type = cpp.RefType.Unique}
        # expected-warning@-2: @cpp.Ref{type = cpp.RefType.Unique} is deprecated. Please use @thrift.Box annotation instead in `field4`.
        # expected-error@-2: Structured annotation `Ref` is already defined for `field4`.
      4: optional Foo field4;
    }
  )");
}

TEST(CompilerTest, unstructured_and_structured_adapter) {
  check_compile(R"(
    include "thrift/annotation/cpp.thrift"
    include "thrift/annotation/hack.thrift"

    struct MyStruct {
        @cpp.Adapter{} # expected-error: key `name` not found.
        @hack.Adapter{name="MyAdapter"}
        2: i64 my_field2;
        @cpp.Adapter{name="MyAdapter"}
          # expected-error@-1: cpp.ref, cpp2.ref are deprecated. Please use @thrift.Box annotation instead in `my_field3` with @cpp.Adapter.
        @hack.Adapter{name="MyAdapter"}
        3: optional i64 my_field3 (cpp.ref);
        @cpp.Adapter{name="MyAdapter"}
          # expected-error@-1: cpp.ref_type = `unique`, cpp2.ref_type = `unique` are deprecated. Please use @thrift.Box annotation instead in `my_field4` with @cpp.Adapter.
        @hack.Adapter{name="MyAdapter"}
        4: optional i64 my_field4 (cpp.ref_type = "unique");
        @cpp.Adapter{name="MyAdapter"}
          # expected-error@-1: @cpp.Ref{type = cpp.RefType.Unique} is deprecated. Please use @thrift.Box annotation instead in `my_field5` with @cpp.Adapter.
        @hack.Adapter{name="MyAdapter"}
        @cpp.Ref{type = cpp.RefType.Unique}
        5: optional i64 my_field5;
    }
)");
}

TEST(CompilerTest, typedef_adapter) {
  check_compile(R"(
    include "thrift/annotation/cpp.thrift"
    include "thrift/annotation/hack.thrift"

    @cpp.Adapter{}
      # expected-error@-1: key `name` not found.
      # expected-error@-2: key `name` not found.
    @hack.Adapter{}
    typedef i32 MyI32

    @cpp.Adapter{name="MyAdapter"}
    @hack.Adapter{name="MyAdapter"}
    typedef i64 MyI64

    @cpp.Adapter{name="MyAdapter"}
      # expected-error@-1: The `@cpp.Adapter` annotation cannot be annotated more than once in all typedef levels in `DoubleMyI64`.
      # expected-error@-2: The `@hack.Adapter` annotation cannot be annotated more than once in all typedef levels in `DoubleMyI64`.
    @hack.Adapter{name="MyAdapter"}
    typedef MyI64 DoubleMyI64

    @cpp.Adapter{name="MyAdapter"}
    struct Adapted {}

    @cpp.Adapter{name="MyAdapter"}
      # expected-error@-1: The `@cpp.Adapter` annotation cannot be annotated more than once in all typedef levels in `DoubleAdapted`.
    typedef Adapted DoubleAdapted
  )");
}

TEST(CompilerTest, hack_wrapper_adapter) {
  check_compile(R"(
    include "thrift/annotation/hack.thrift"

    @hack.Adapter{name = "\MyAdapter1"}
    typedef i64 i64Adapted

    @hack.Wrapper{name = "\MyTypeIntWrapper"}
    typedef i64 i64WithWrapper

    @hack.Wrapper{name = "\MyTypeIntWrapper"}
    @hack.Adapter{name = "\MyAdapter1"}
    typedef i64 i64Wrapped_andAdapted

    @hack.Wrapper{name = "\MyTypeIntWrapper"}
    typedef i64Adapted i64Wrapped_andAdapted_2

    typedef list<i64Wrapped_andAdapted> list_of_i64Wrapped_andAdapted

    @hack.Adapter{name = "\MyAdapter1"}
      # expected-error@-1: `@hack.Adapter` on `adapted_list_of_i64Wrapped_andAdapted` cannot be combined with `@hack.Wrapper` on `i64Wrapped_andAdapted`.
    typedef list<i64Wrapped_andAdapted> adapted_list_of_i64Wrapped_andAdapted

    typedef StructWithWrapper structWithWrapper_typedf

    @hack.Adapter{name = "\MyAdapter1"}
      # expected-error@-1: `@hack.Adapter` on `adapted_structWithWrapper_typedf` cannot be combined with `@hack.Wrapper` on `StructWithWrapper`.
    typedef StructWithWrapper adapted_structWithWrapper_typedf

    @hack.Wrapper{name = "\MyStructWrapper"}
    struct StructWithWrapper {
    1: i64 int_field;
    }

    @hack.Wrapper{name = "\MyStructWrapper"}
    struct MyNestedStruct {
    @hack.FieldWrapper{name = "\MyFieldWrapper"}
    1: i64WithWrapper double_wrapped_field;
    @hack.FieldWrapper{name = "\MyFieldWrapper"}
      # expected-error@-1: `@hack.Adapter` on `double_wrapped_and_adapted_field` cannot be combined with `@hack.Wrapper` on `i64WithWrapper`.
    @hack.Adapter{name = "\MyFieldAdapter"}
    2: i64WithWrapper double_wrapped_and_adapted_field;
    @hack.FieldWrapper{name = "\MyFieldWrapper"}
    3: StructWithWrapper double_wrapped_struct;
    @hack.FieldWrapper{name = "\MyFieldWrapper"}
    4: map<string, StructWithWrapper> wrapped_map_of_string_to_StructWithWrapper;
    @hack.Adapter{name = "\MyFieldAdapter"}
      # expected-error@-1: `@hack.Adapter` on `adapted_map_of_string_to_StructWithWrapper` cannot be combined with `@hack.Wrapper` on `StructWithWrapper`.
    5: map<string, StructWithWrapper> adapted_map_of_string_to_StructWithWrapper;
    @hack.FieldWrapper{name = "\MyFieldWrapper"}
      # expected-error@-1: `@hack.Adapter` on `adapted_double_wrapped_struct` cannot be combined with `@hack.Wrapper` on `StructWithWrapper`.
    @hack.Adapter{name = "\MyFieldAdapter"}
    6: StructWithWrapper adapted_double_wrapped_struct;
    }
  )");
}

TEST(CompilerTest, invalid_and_too_many_splits) {
  check_compile(
      R"(
    struct Foo { 1: i32 field }
    struct Bar { 1: i32 field }
    exception Baz { 1: i32 field }
    enum E { f = 0 }
    service SumService {
        i32 sum(1: i32 num1, 2: i32 num2);
    }
)",
      {"--gen", "mstch_cpp2:types_cpp_splits=4"});

  check_compile(
      R"(
    # expected-error@-1: `types_cpp_splits=5` is misconfigured: it can not be greater than the number of objects, which is 4. [more-splits-than-objects-rule]
    struct Foo { 1: i32 field }
    struct Bar { 1: i32 field }
    exception Baz { 1: i32 field }
    enum E { f = 0 }
    service SumService {
        i32 sum(1: i32 num1, 2: i32 num2);
    }
)",
      {"--gen", "mstch_cpp2:types_cpp_splits=5"});

  check_compile(
      R"(
    # expected-error@-1: Invalid types_cpp_splits value: `3a`
    struct Foo { 1: i32 field }
    struct Bar { 1: i32 field }
    exception Baz { 1: i32 field }
    enum E { f = 0 }
    service SumService {
        i32 sum(1: i32 num1, 2: i32 num2);
    }
)",
      {"--gen", "mstch_cpp2:types_cpp_splits=3a"});
}

TEST(CompilerTest, invalid_and_too_many_client_splits) {
  check_compile(
      R"(
    service MyService1 {
      i32 func1(1: i32 num);
      i32 func2(1: i32 num);
      i32 func3(1: i32 num);
    }
    service MyService2 {
      i32 func1(1: i32 num);
      i32 func2(1: i32 num);
    }
  )",
      {"--gen", "mstch_cpp2:client_cpp_splits={MyService1:3,MyService2:2}"});

  check_compile(
      R"(
    service MyService1 {
      i32 func1(1: i32 num);
      i32 func2(1: i32 num);
      i32 func3(1: i32 num);
    }
    service MyService2 {
      # expected-error@-1: `client_cpp_splits=3` (For service MyService2) is misconfigured: it can not be greater than the number of functions, which is 2. [more-splits-than-functions-rule]
      i32 func1(1: i32 num);
      i32 func2(1: i32 num);
    }
  )",
      {"--gen", "mstch_cpp2:client_cpp_splits={MyService1:3,MyService2:3}"});
  check_compile(
      R"(
    # expected-error@-1: Invalid pair `MyService1:3:1` in client_cpp_splits value: `MyService1:3:1,MyService2:2`
    service MyService1 {
      i32 func1(1: i32 num);
      i32 func2(1: i32 num);
      i32 func3(1: i32 num);
    }
    service MyService2 {
      i32 func1(1: i32 num);
      i32 func2(1: i32 num);
    }
  )",
      {"--gen", "mstch_cpp2:client_cpp_splits={MyService1:3:1,MyService2:2}"});
}

TEST(CompilerTest, non_beneficial_lazy_fields) {
  check_compile(
      R"(
    typedef double FP
    struct A {
      1: i32 field (cpp.experimental.lazy); # expected-warning: The annotation cpp.experimental.lazy is deprecated. Please use @cpp.Lazy instead.
        # expected-error@-1: Integral field `field` can not be marked as lazy, since doing so won't bring any benefit. [no-lazy-int-float-field-rule]
      2: FP field2 (cpp.experimental.lazy); # expected-warning: The annotation cpp.experimental.lazy is deprecated. Please use @cpp.Lazy instead.
        # expected-error@-1: Floating point field `field2` can not be marked as lazy, since doing so won't bring any benefit. [no-lazy-int-float-field-rule]
    }
  )");
}

TEST(CompilerTest, non_exception_type_in_throws) {
  check_compile(R"(
    struct A {}

    service B {
      void foo() throws (1: A ex) # expected-error: Non-exception type, `A`, in throws.
      stream<i32 throws (1: A ex)> bar() # expected-error: Non-exception type, `A`, in throws.
      sink<i32 throws (1: A ex), # expected-error: Non-exception type, `A`, in throws.
            i32 throws (1: A ex)> baz() # expected-error: Non-exception type, `A`, in throws.
    }
  )");
}

TEST(CompilerTest, boxed_ref_and_optional) {
  check_compile(R"(
    include "thrift/annotation/thrift.thrift"

    struct A {
      1: i64 field (cpp.box)
        # expected-error@-1: The `thrift.box` annotation can only be used with optional fields. Make sure `field` is optional.
        # expected-warning@-2: The annotation cpp.box is deprecated. Please use @thrift.Box instead.
      @thrift.Box
      2: i64 field2
        # expected-error@-2: The `thrift.box` annotation can only be used with optional fields. Make sure `field2` is optional.
    }
  )");

  check_compile(R"(
    include "thrift/annotation/cpp.thrift"
    include "thrift/annotation/thrift.thrift"

    package "apache.org/thrift/test"

    struct MyStruct {
        1: i32 field1 = 1;
    }

    typedef MyStruct MyStruct2

    struct A {
        1: optional i64 field (cpp.ref, thrift.box);
          # expected-error@-1: The `@thrift.Box` annotation cannot be combined with the other reference annotations. Only annotate a single reference annotation from `field`.
          # expected-warning@-2: The annotation thrift.box is deprecated. Please use @thrift.Box instead.

        @thrift.Box
          # expected-error@-1: The `@thrift.InternBox` annotation cannot be combined with the other reference annotations. Only annotate a single reference annotation from `field2`.
          # expected-error@-2: The `@thrift.InternBox` annotation can only be used with a struct field.
          # expected-error@-3: The `@thrift.InternBox` annotation can only be used with unqualified or terse fields. Make sure `field2` is unqualified or annotated with `@thrift.TerseWrite`.
        @thrift.InternBox
        2: optional i64 field2;

        @thrift.InternBox
          # expected-error@-1: The `@thrift.InternBox` annotation currently does not support a field with custom default.
        3: MyStruct field3 = {"field1" : 1};

        @cpp.Ref
          # expected-error@-1: The `@thrift.InternBox` annotation cannot be combined with the other reference annotations. Only annotate a single reference annotation from `field4`.
          # expected-error@-2: The `thrift.box` annotation can only be used with optional fields. Make sure `field4` is optional.
        @thrift.Box
        @thrift.InternBox
        @thrift.TerseWrite
        4: MyStruct field4;

        @thrift.InternBox
        @thrift.TerseWrite
        5: MyStruct field5;

        @thrift.InternBox
        @thrift.TerseWrite
        6: MyStruct2 field6;
    }
  )");
}

TEST(CompilerTest, unique_ref) {
  check_compile(R"(
    include "thrift/annotation/cpp.thrift"

    struct Bar {
      1: optional Foo field1 (cpp.ref);
        # expected-warning@-1: cpp.ref, cpp2.ref are deprecated. Please use @thrift.Box annotation instead in `field1`.

      2: optional Foo field2 (cpp2.ref);
        # expected-warning@-1: cpp.ref, cpp2.ref are deprecated. Please use @thrift.Box annotation instead in `field2`.

      @cpp.Ref{type = cpp.RefType.Unique}
      3: optional Foo field3;
        # expected-warning@-2: @cpp.Ref{type = cpp.RefType.Unique} is deprecated. Please use @thrift.Box annotation instead in `field3`.

      @cpp.Ref{type = cpp.RefType.Shared}
      4: optional Foo field4;

      @cpp.Ref{type = cpp.RefType.SharedMutable}
      5: optional Foo field5;

      6: optional Foo field6 (cpp.ref_type = "unique");
        # expected-warning@-1: cpp.ref_type = `unique`, cpp2.ref_type = `unique` are deprecated. Please use @thrift.Box annotation instead in `field6`.

      7: optional Foo field7 (cpp2.ref_type = "unique");
        # expected-warning@-1: cpp.ref_type = `unique`, cpp2.ref_type = `unique` are deprecated. Please use @thrift.Box annotation instead in `field7`.

      8: optional Foo field8 (cpp.ref_type = "shared");

      9: optional Foo field9 (cpp2.ref_type = "shared");

      10: optional Foo field10 (cpp.ref = "true");
        # expected-warning@-1: cpp.ref, cpp2.ref are deprecated. Please use @thrift.Box annotation instead in `field10`.

      11: optional Foo field11 (cpp2.ref = "true");
        # expected-warning@-1: cpp.ref, cpp2.ref are deprecated. Please use @thrift.Box annotation instead in `field11`.

      12: optional Foo field12;

      13: optional Foo field13;
    }

    struct Foo {}
  )");
}

TEST(CompilerTest, non_optional_ref_and_box_forbidden) {
  check_compile(
      R"(
    include "thrift/annotation/cpp.thrift"
    include "thrift/annotation/thrift.thrift"

    struct Bar {
      1: Foo field1 (cpp.ref);
        # expected-error@-1: Field with @cpp.Ref (or similar) annotation must be optional: `field1` (in `Bar`).

      2: Foo field2 (cpp2.ref);
        # expected-error@-1: Field with @cpp.Ref (or similar) annotation must be optional: `field2` (in `Bar`).

      @cpp.Ref{type = cpp.RefType.Unique}
      3: Foo field3;
        # expected-error@-2: Field with @cpp.Ref (or similar) annotation must be optional: `field3` (in `Bar`).

      @cpp.Ref{type = cpp.RefType.Shared}
      4: Foo field4;
        # expected-error@-2: Field with @cpp.Ref (or similar) annotation must be optional: `field4` (in `Bar`).

      @cpp.Ref{type = cpp.RefType.SharedMutable}
      5: Foo field5;
        # expected-error@-2: Field with @cpp.Ref (or similar) annotation must be optional: `field5` (in `Bar`).

      6: Foo field6 (cpp.ref_type = "unique");
        # expected-error@-1: Field with @cpp.Ref (or similar) annotation must be optional: `field6` (in `Bar`).

      7: Foo field7 (cpp2.ref_type = "unique");
        # expected-error@-1: Field with @cpp.Ref (or similar) annotation must be optional: `field7` (in `Bar`).

      8: Foo field8 (cpp.ref_type = "shared");
        # expected-error@-1: Field with @cpp.Ref (or similar) annotation must be optional: `field8` (in `Bar`).

      9: Foo field9 (cpp2.ref_type = "shared");
        # expected-error@-1: Field with @cpp.Ref (or similar) annotation must be optional: `field9` (in `Bar`).

      10: Foo field10 (cpp.ref = "true");
        # expected-error@-1: Field with @cpp.Ref (or similar) annotation must be optional: `field10` (in `Bar`).

      11: Foo field11 (cpp2.ref = "true");
        # expected-error@-1: Field with @cpp.Ref (or similar) annotation must be optional: `field11` (in `Bar`).

      12: Foo field12;

      @thrift.Box
      13: Foo field13;
        # expected-error@-2: The `thrift.box` annotation can only be used with optional fields. Make sure `field13` is optional.
    }

    struct Foo {}
  )");
}

TEST(CompilerTest, non_optional_ref_legacy_allowed_annotation_on_wrong_field) {
  check_compile(
      R"(
    include "thrift/annotation/cpp.thrift"
    include "thrift/annotation/thrift.thrift"

    struct Bar {
      1: Foo field1 (cpp.ref);
        # expected-error@-1: Field with @cpp.Ref (or similar) annotation must be optional: `field1` (in `Bar`).

      @cpp.AllowLegacyNonOptionalRef
      2: Foo field2;
        # expected-error@-2: Cannot annotate field with @cpp.AllowLegacyNonOptionalRef unless it is a reference field (i.e., @cpp.Ref): `field2`.
    }

    struct Foo {}
  )");
}

TEST(CompilerTest, nonexistent_field_name) {
  check_compile(R"(
    struct Foo {}
    typedef list<Foo> List
    const List l = [{"foo": "bar"}];
    # expected-error@-1: no field named `foo` in `Foo`
  )");
}

TEST(CompilerTest, annotation_scopes) {
  check_compile(
      R"(
    include "thrift/annotation/scope.thrift"

    struct NotAnAnnot {}

    @scope.Struct
    struct StructAnnot{}
    @scope.Field
    struct FieldAnnot{}

    @scope.Struct
    @scope.Field
    struct StructOrFieldAnnot {}

    @scope.Enum
    struct EnumAnnot {}

    @NotAnAnnot
      # expected-warning@-1: Using `NotAnAnnot` as an annotation, even though it has not been enabled for any annotation scope.
    @StructAnnot
    @FieldAnnot
      # expected-error@-1: `FieldAnnot` cannot annotate `TestStruct`
    @StructOrFieldAnnot
    @EnumAnnot
    # expected-error@-1: `EnumAnnot` cannot annotate `TestStruct`
    struct TestStruct {
      @FieldAnnot
      @StructAnnot
        # expected-error@-1: `StructAnnot` cannot annotate `test_field`
      @StructOrFieldAnnot
      1: bool test_field;
    }

    @EnumAnnot
    enum TestEnum { Foo = 0, Bar = 1 }

    typedef StructAnnot AliasedAnnot

    @AliasedAnnot
      # expected-warning@-1: Using `AliasedAnnot` as an annotation, even though it has not been enabled for any annotation scope.
    struct AlsoStruct {}
  )",
      {"--legacy-strict"});
}

TEST(CompilerTest, lazy_struct_compatibility) {
  check_compile(R"(
    struct Foo { # expected-error: cpp.methods is incompatible with lazy deserialization in struct `Foo`
      1: list<i32> field (cpp.experimental.lazy) # expected-warning: The annotation cpp.experimental.lazy is deprecated. Please use @cpp.Lazy instead.
    } (cpp.methods = "")
  )");
}

TEST(CompilerTest, duplicate_field_id) {
  check_compile(R"(
    struct A {
      1: i64 field1;
      1: i64 field2;
        # expected-error@-1: Field id 1 for `field2` has already been used.
    }
  )");
}

TEST(CompilerTest, thrift_uri_uniqueness) {
  std::map<std::string, std::string> name_contents_map;
  name_contents_map["file1.thrift"] = R"(
    struct Foo1 {
    } (thrift.uri = "facebook.com/thrift/annotation/Foo")
  )";
  name_contents_map["file2.thrift"] = R"(
    struct Foo2 {
    } (thrift.uri = "facebook.com/thrift/annotation/Foo")
      # expected-error@-2: Thrift URI `facebook.com/thrift/annotation/Foo` is already defined for `Foo2`.
    struct Bar1 {
    } (thrift.uri = "facebook.com/thrift/annotation/Bar")
  )";
  name_contents_map["main.thrift"] = R"(
    include "file1.thrift"
    include "file2.thrift"
    struct Bar2 {
    } (thrift.uri = "facebook.com/thrift/annotation/Bar") # expected-warning@-1: The annotation thrift.uri is deprecated. Please use @thrift.Uri instead.
      # expected-error@-2: Thrift URI `facebook.com/thrift/annotation/Bar` is already defined for `Bar2`.
    struct Baz1 {
    } (thrift.uri = "facebook.com/thrift/annotation/Baz") # expected-warning@-1: The annotation thrift.uri is deprecated. Please use @thrift.Uri instead.
    struct Baz2 {
    } (thrift.uri = "facebook.com/thrift/annotation/Baz") # expected-warning@-1: The annotation thrift.uri is deprecated. Please use @thrift.Uri instead.
      # expected-error@-2: Thrift URI `facebook.com/thrift/annotation/Baz` is already defined for `Baz2`.
  )";
  check_compile(name_contents_map, "main.thrift");
}

TEST(CompilerTest, terse_write_annotation) {
  check_compile(R"(
    include "thrift/annotation/thrift.thrift"

    @thrift.Experimental
    package "apache.org/thrift/test"

    struct TerseFields {
      @thrift.TerseWrite
      1: i64 field1;
      @thrift.TerseWrite
        # expected-error@-1: `@thrift.TerseWrite` cannot be used with qualified fields
      2: optional i64 field2;
      @thrift.TerseWrite
        # expected-error@-1: `@thrift.TerseWrite` cannot be used with qualified fields
        # expected-warning@-2: The 'required' qualifier is deprecated and ignored by most language implementations. Leave the field unqualified instead.
      3: required i64 field3;
    }

    union TerseUnion {
      @thrift.TerseWrite
        # expected-error@-1: `@thrift.TerseWrite` cannot be applied to union fields (in `TerseUnion`).
      1: i64 field1;
    }
  )");
}

// Time complexity of for_each_transitive_field should be O(1)
TEST(CompilerTest, time_complexity_of_for_each_transitive_field) {
  check_compile(R"(
    struct S_01 { 1: i32 i; }
    struct S_02 { 1: optional S_01 a (thrift.box); 2: optional S_01 b (thrift.box); }
    struct S_03 { 1: optional S_02 a (thrift.box); 2: optional S_02 b (thrift.box); }
    struct S_04 { 1: optional S_03 a (thrift.box); 2: optional S_03 b (thrift.box); }
    struct S_05 { 1: optional S_04 a (thrift.box); 2: optional S_04 b (thrift.box); }
    struct S_06 { 1: optional S_05 a (thrift.box); 2: optional S_05 b (thrift.box); }
    struct S_07 { 1: optional S_06 a (thrift.box); 2: optional S_06 b (thrift.box); }
    struct S_08 { 1: optional S_07 a (thrift.box); 2: optional S_07 b (thrift.box); }
    struct S_09 { 1: optional S_08 a (thrift.box); 2: optional S_08 b (thrift.box); }
    struct S_10 { 1: optional S_09 a (thrift.box); 2: optional S_09 b (thrift.box); }
    struct S_11 { 1: optional S_10 a (thrift.box); 2: optional S_10 b (thrift.box); }
    struct S_12 { 1: optional S_11 a (thrift.box); 2: optional S_11 b (thrift.box); }
    struct S_13 { 1: optional S_12 a (thrift.box); 2: optional S_12 b (thrift.box); }
    struct S_14 { 1: optional S_13 a (thrift.box); 2: optional S_13 b (thrift.box); }
    struct S_15 { 1: optional S_14 a (thrift.box); 2: optional S_14 b (thrift.box); }
    struct S_16 { 1: optional S_15 a (thrift.box); 2: optional S_15 b (thrift.box); }
    struct S_17 { 1: optional S_16 a (thrift.box); 2: optional S_16 b (thrift.box); }
    struct S_18 { 1: optional S_17 a (thrift.box); 2: optional S_17 b (thrift.box); }
    struct S_19 { 1: optional S_18 a (thrift.box); 2: optional S_18 b (thrift.box); }
    struct S_20 { 1: optional S_19 a (thrift.box); 2: optional S_19 b (thrift.box); }
    struct S_21 { 1: optional S_20 a (thrift.box); 2: optional S_20 b (thrift.box); }
    struct S_22 { 1: optional S_21 a (thrift.box); 2: optional S_21 b (thrift.box); }
    struct S_23 { 1: optional S_22 a (thrift.box); 2: optional S_22 b (thrift.box); }
    struct S_24 { 1: optional S_23 a (thrift.box); 2: optional S_23 b (thrift.box); }
    struct S_25 { 1: optional S_24 a (thrift.box); 2: optional S_24 b (thrift.box); }
    struct S_26 { 1: optional S_25 a (thrift.box); 2: optional S_25 b (thrift.box); }
    struct S_27 { 1: optional S_26 a (thrift.box); 2: optional S_26 b (thrift.box); }
    struct S_28 { 1: optional S_27 a (thrift.box); 2: optional S_27 b (thrift.box); }
    struct S_29 { 1: optional S_28 a (thrift.box); 2: optional S_28 b (thrift.box); }
    struct S_30 { 1: optional S_29 a (thrift.box); 2: optional S_29 b (thrift.box); }
    struct S_31 { 1: optional S_30 a (thrift.box); 2: optional S_30 b (thrift.box); }
    struct S_32 { 1: optional S_31 a (thrift.box); 2: optional S_31 b (thrift.box); }
    # expected-warning@3: The annotation thrift.box is deprecated. Please use @thrift.Box instead.
		# expected-warning@3: The annotation thrift.box is deprecated. Please use @thrift.Box instead.
		# expected-warning@4: The annotation thrift.box is deprecated. Please use @thrift.Box instead.
		# expected-warning@4: The annotation thrift.box is deprecated. Please use @thrift.Box instead.
		# expected-warning@5: The annotation thrift.box is deprecated. Please use @thrift.Box instead.
		# expected-warning@5: The annotation thrift.box is deprecated. Please use @thrift.Box instead.
		# expected-warning@6: The annotation thrift.box is deprecated. Please use @thrift.Box instead.
		# expected-warning@6: The annotation thrift.box is deprecated. Please use @thrift.Box instead.
		# expected-warning@7: The annotation thrift.box is deprecated. Please use @thrift.Box instead.
		# expected-warning@7: The annotation thrift.box is deprecated. Please use @thrift.Box instead.
		# expected-warning@8: The annotation thrift.box is deprecated. Please use @thrift.Box instead.
		# expected-warning@8: The annotation thrift.box is deprecated. Please use @thrift.Box instead.
		# expected-warning@9: The annotation thrift.box is deprecated. Please use @thrift.Box instead.
		# expected-warning@9: The annotation thrift.box is deprecated. Please use @thrift.Box instead.
		# expected-warning@10: The annotation thrift.box is deprecated. Please use @thrift.Box instead.
		# expected-warning@10: The annotation thrift.box is deprecated. Please use @thrift.Box instead.
		# expected-warning@11: The annotation thrift.box is deprecated. Please use @thrift.Box instead.
		# expected-warning@11: The annotation thrift.box is deprecated. Please use @thrift.Box instead.
		# expected-warning@12: The annotation thrift.box is deprecated. Please use @thrift.Box instead.
		# expected-warning@12: The annotation thrift.box is deprecated. Please use @thrift.Box instead.
		# expected-warning@13: The annotation thrift.box is deprecated. Please use @thrift.Box instead.
		# expected-warning@13: The annotation thrift.box is deprecated. Please use @thrift.Box instead.
		# expected-warning@14: The annotation thrift.box is deprecated. Please use @thrift.Box instead.
		# expected-warning@14: The annotation thrift.box is deprecated. Please use @thrift.Box instead.
		# expected-warning@15: The annotation thrift.box is deprecated. Please use @thrift.Box instead.
		# expected-warning@15: The annotation thrift.box is deprecated. Please use @thrift.Box instead.
		# expected-warning@16: The annotation thrift.box is deprecated. Please use @thrift.Box instead.
		# expected-warning@16: The annotation thrift.box is deprecated. Please use @thrift.Box instead.
		# expected-warning@17: The annotation thrift.box is deprecated. Please use @thrift.Box instead.
		# expected-warning@17: The annotation thrift.box is deprecated. Please use @thrift.Box instead.
		# expected-warning@18: The annotation thrift.box is deprecated. Please use @thrift.Box instead.
		# expected-warning@18: The annotation thrift.box is deprecated. Please use @thrift.Box instead.
		# expected-warning@19: The annotation thrift.box is deprecated. Please use @thrift.Box instead.
		# expected-warning@19: The annotation thrift.box is deprecated. Please use @thrift.Box instead.
		# expected-warning@20: The annotation thrift.box is deprecated. Please use @thrift.Box instead.
		# expected-warning@20: The annotation thrift.box is deprecated. Please use @thrift.Box instead.
		# expected-warning@21: The annotation thrift.box is deprecated. Please use @thrift.Box instead.
		# expected-warning@21: The annotation thrift.box is deprecated. Please use @thrift.Box instead.
		# expected-warning@22: The annotation thrift.box is deprecated. Please use @thrift.Box instead.
		# expected-warning@22: The annotation thrift.box is deprecated. Please use @thrift.Box instead.
		# expected-warning@23: The annotation thrift.box is deprecated. Please use @thrift.Box instead.
		# expected-warning@23: The annotation thrift.box is deprecated. Please use @thrift.Box instead.
		# expected-warning@24: The annotation thrift.box is deprecated. Please use @thrift.Box instead.
		# expected-warning@24: The annotation thrift.box is deprecated. Please use @thrift.Box instead.
		# expected-warning@25: The annotation thrift.box is deprecated. Please use @thrift.Box instead.
		# expected-warning@25: The annotation thrift.box is deprecated. Please use @thrift.Box instead.
		# expected-warning@26: The annotation thrift.box is deprecated. Please use @thrift.Box instead.
		# expected-warning@26: The annotation thrift.box is deprecated. Please use @thrift.Box instead.
		# expected-warning@27: The annotation thrift.box is deprecated. Please use @thrift.Box instead.
		# expected-warning@27: The annotation thrift.box is deprecated. Please use @thrift.Box instead.
		# expected-warning@28: The annotation thrift.box is deprecated. Please use @thrift.Box instead.
		# expected-warning@28: The annotation thrift.box is deprecated. Please use @thrift.Box instead.
		# expected-warning@29: The annotation thrift.box is deprecated. Please use @thrift.Box instead.
		# expected-warning@29: The annotation thrift.box is deprecated. Please use @thrift.Box instead.
		# expected-warning@30: The annotation thrift.box is deprecated. Please use @thrift.Box instead.
		# expected-warning@30: The annotation thrift.box is deprecated. Please use @thrift.Box instead.
		# expected-warning@31: The annotation thrift.box is deprecated. Please use @thrift.Box instead.
		# expected-warning@31: The annotation thrift.box is deprecated. Please use @thrift.Box instead.
		# expected-warning@32: The annotation thrift.box is deprecated. Please use @thrift.Box instead.
		# expected-warning@32: The annotation thrift.box is deprecated. Please use @thrift.Box instead.
		# expected-warning@33: The annotation thrift.box is deprecated. Please use @thrift.Box instead.
		# expected-warning@33: The annotation thrift.box is deprecated. Please use @thrift.Box instead.
  )");
}

TEST(CompilerTest, inject_metadata_fields_annotation) {
  std::map<std::string, std::string> name_contents_map;
  name_contents_map["foo.thrift"] = R"(
    struct Fields {
      1: i64 field1;
      2: optional i64 field2;
      3: required i64 field3;
    }
  )";
  name_contents_map["bar.thrift"] = R"(
    include "thrift/annotation/internal.thrift"

    typedef i64 MyI64

    union UnionFields {
        1: i64 field1;
        2: i64 field2;
    }

    struct Fields {
        1: i64 field1;
          # expected-error@-1: Field `field1` is already defined for `Injected5`.
        2: optional i64 field2;
        3: required i64 field3;
          # expected-warning@-1: The 'required' qualifier is deprecated and ignored by most language implementations. Leave the field unqualified instead.
          # expected-warning@-2: The 'required' qualifier is deprecated and ignored by most language implementations. Leave the field unqualified instead.
    }

    @internal.InjectMetadataFields{type="foo.Fields"}
      # expected-error@-1: Can not find expected type `foo.Fields` specified in `@internal.InjectMetadataFields` in the current scope. Please check the include.
    struct Injected1 {}

    @internal.InjectMetadataFields # expected-error: key `type` not found.
    struct Injected2 {}

    @internal.InjectMetadataFields{type="UnionFields"}
      # expected-error@-1: `UnionFields` is not a struct type. `@internal.InjectMetadataFields` can be only used with a struct type.
    struct Injected3 {}

    @internal.InjectMetadataFields{type="MyI64"}
      # expected-error@-1: `MyI64` is not a struct type. `@internal.InjectMetadataFields` can be only used with a struct type.
    struct Injected4 {}

    @internal.InjectMetadataFields{type="Fields"}
    struct Injected5 {
        1: i64 field1;
    }

    // If a field is explicitly assigned with field id 0,
    // the field id gets implicitly converted -1.
    struct BoundaryFields {
      -1: i64 underflow;
      1: i64 lower_boundary;
      999: i64 upper_boundary;
      1000: i64 overflow;
    }

    @internal.InjectMetadataFields{type="BoundaryFields"}
      # expected-error@-1: Field id `-1` does not mapped to valid internal id.
      # expected-error@-2: Field id `1000` does not mapped to valid internal id.
    struct Injected6 {}
  )";
  check_compile(name_contents_map, "bar.thrift");
}

TEST(CompilerTest, invalid_field_type_for_hack_codegen) {
  check_compile(
      R"(
      struct S {
        1: i32 field;

        2: set<float> set_of_float;
        # expected-error@-1: `float` cannot be used as a set element in Hack because it is not integer, string, binary or enum

        3: map<float, i32> map_of_float_to_int;
        # expected-error@-1: `float` cannot be used as a map key in Hack because it is not integer, string, binary or enum
      }
      )",
      {"--gen", "hack"});
}

TEST(CompilerTest, invalid_return_type_for_hack_codegen) {
  check_compile(
      R"(
      service Foo {
        set<float> invalid_rpc_return();
        # expected-error@-1: `float` cannot be used as a set element in Hack because it is not integer, string, binary or enum
      }
      )",
      {"--gen", "hack"});
}

TEST(CompilerTest, invalid_param_type_for_hack_codegen) {
  check_compile(
      R"(
      service Foo {
        void invalid_rpc_param(set<float> arg1);
        # expected-error@-1: `float` cannot be used as a set element in Hack because it is not integer, string, binary or enum
      }
      )",
      {"--gen", "hack"});
}

TEST(CompilerTest, undefined_type_include) {
  std::map<std::string, std::string> name_contents_map;
  name_contents_map["header.thrift"] = "";
  name_contents_map["main.thrift"] = R"(
    include "header.thrift"

    service Foo {
      header.Bar func();
        # expected-error@-1: Failed to resolve return type of `func`.
        # expected-error@-2: Type `header.Bar` not defined.
    }
  )";
  check_compile(name_contents_map, "main.thrift");
}

TEST(CompilerTest, adapting_variable) {
  check_compile(R"(
    include "thrift/annotation/cpp.thrift"
    include "thrift/annotation/scope.thrift"
    include "thrift/annotation/thrift.thrift"

    package "facebook.com/thrift/test"

    @cpp.Adapter{name="MyAdapter"}
    @scope.Transitive
    struct Config { 1: string path; }

    @Config{path = "to/my/service"}
    const i32 Foo = 10;
      # expected-error@-2: Using adapters on const `Foo` is only allowed in the experimental mode.

    @Config{path = "to/my/service"}
    const string Bar = "20";
      # expected-error@-2: Using adapters on const `Bar` is only allowed in the experimental mode.

    struct MyStruct { 1: i32 field; }

    @Config{path = "to/my/service"}
    const MyStruct Baz = MyStruct{field=30};
      # expected-error@-2: Using adapters on const `Baz` is only allowed in the experimental mode.

    @cpp.Adapter{name="MyAdapter"}
    const i32 Foo2 = 10;
      # expected-error@-2: Using adapters on const `Foo2` is only allowed in the experimental mode.

    @cpp.Adapter{name="MyAdapter"}
    const string Bar2 = "20";
      # expected-error@-2: Using adapters on const `Bar2` is only allowed in the experimental mode.

    @cpp.Adapter{name="MyAdapter"}
    const MyStruct Baz2 = MyStruct{field=30};
      # expected-error@-2: Using adapters on const `Baz2` is only allowed in the experimental mode.
  )");

  check_compile(R"(
    include "thrift/annotation/cpp.thrift"
    include "thrift/annotation/scope.thrift"
    include "thrift/annotation/thrift.thrift"

    @thrift.Experimental
    package "facebook.com/thrift/test"

    @cpp.Adapter{name="MyAdapter"}
    @scope.Transitive
    struct Config { 1: string path; }

    @Config{path = "to/my/service"}
    const i32 Foo = 10;

    @Config{path = "to/my/service"}
    const string Bar = "20";

    struct MyStruct { 1: i32 field; }

    @Config{path = "to/my/service"}
    const MyStruct Baz = MyStruct{field=30};

    @cpp.Adapter{name="MyAdapter"}
    const i32 Foo2 = 10;

    @cpp.Adapter{name="MyAdapter"}
    const string Bar2 = "20";

    @cpp.Adapter{name="MyAdapter"}
    const MyStruct Baz2 = MyStruct{field=30};
  )");
}

TEST(CompilerTest, reserved_ids) {
  check_compile(R"(
    include "thrift/annotation/thrift.thrift"

    @thrift.ReserveIds{ids = [3, 8]}
      # expected-error@-1: Fields in IdList cannot use reserved ids: 3
    struct IdList {
      1: i64 a;
      3: string bad_field;
    }

    @thrift.ReserveIds{ids = [2], id_ranges = {5: 10, 15: 20}}
      # expected-error@-1: Fields in IdRanges cannot use reserved ids: 9
    struct IdRanges {
      1: i64 a;
      9: string bad_field;
    }

    @thrift.ReserveIds{ids = [3, 8]}
      # expected-error@-1: Enum values in EnumWithBadId cannot use reserved ids: 3
    enum EnumWithBadId {
      A = 0,
      B = 3,
    }

    @thrift.ReserveIds{ids = [3, 8]}
      # expected-error@-1: Fields in UnionWithBadId cannot use reserved ids: 3
    union UnionWithBadId {
      1: i64 a;
      3: string bad_field;
    }

    @thrift.ReserveIds{ids = [3, 8]}
      # expected-error@-1: Fields in ExceptionWithBadId cannot use reserved ids: 3
    safe exception ExceptionWithBadId {
      1: i64 a;
      3: string bad_field;
    }

    @thrift.ReserveIds{id_ranges = {5: 3}}
      # expected-error@-1: For each (start: end) in id_ranges, we must have start < end. Got (5: 3), annotated on InvalidIdRange
    struct InvalidIdRange {
      1: i64 a;
      2: string bad_field;
    }

    @thrift.ReserveIds{id_ranges = {5: 10}}
    struct OkStruct {
      1: i64 a;
      10: string b;
    }

    @thrift.ReserveIds{ids = [-40000, 40000], id_ranges = {-50001: -50000, 50000: 50001}}
      # expected-error@-1: Struct `Message` cannot have reserved id that is out of range: -50001
      # expected-error@-2: Struct `Message` cannot have reserved id that is out of range: -40000
      # expected-error@-3: Struct `Message` cannot have reserved id that is out of range: 40000
      # expected-error@-4: Struct `Message` cannot have reserved id that is out of range: 50000
    struct Message {
      1: string msg;
    }
  )");
}

TEST(CompilerTest, required_key_specified_in_structured_annotation) {
  check_compile(R"(
    include "thrift/annotation/cpp.thrift"

    struct Foo {
      @cpp.FieldInterceptor{name = "MyFieldInterceptor"}
      1: i32 field1;
      @cpp.FieldInterceptor
        # expected-error@-1: `@cpp.FieldInterceptor` cannot be used without `name` specified in `field2`.
      2: i32 field2;
    }

    @cpp.EnumType
      # expected-error@-1: `@cpp.EnumType` cannot be used without `type` specified in `MyEnum1`.
    enum MyEnum1 {
      ZERO = 0,
    }

    @cpp.EnumType{type = cpp.EnumUnderlyingType.I16}
    enum MyEnum2 {
      ZERO = 0,
    }
  )");
}

TEST(CompilerTest, nonexist_type_in_variable) {
  check_compile(R"(
    const map<i8, string> foo = {1: "str"}
      # expected-error@-1: Type `i8` not defined.
  )");
}

TEST(CompilerTest, cyclic_dependency) {
  check_compile(R"(
    # expected-error@-1: Cyclic dependency: A -> B -> A
    struct A {
      1: B field;
    }

    struct B {
      1: A field;
    }
  )");
}

TEST(CompilerTest, invalid_utf8) {
  check_compile(
      "const string s0 = '\x80';\n"
      "# expected-error@-1: invalid UTF-8 start byte '\\x80'\n"
      "const string s1 = '\xC2\x42';\n"
      "# expected-error@-1: invalid UTF-8 continuation byte '\\x42'\n"
      "const string s2 = '\xE2\x80\x43';\n"
      "# expected-error@-1: invalid UTF-8 continuation byte '\\x43'\n"
      "const string s3 = '\xF2\x80\x80\x44';\n"
      "# expected-error@-1: invalid UTF-8 continuation byte '\\x44'\n"
      "const string s4 = '\xF4\x90\x80\x80';\n"
      "# expected-error@-1: invalid UTF-8 continuation byte '\\x90'\n"
      // Overlong sequences:
      "const string over0 = '\xC0';\n"
      "# expected-error@-1: invalid UTF-8 start byte '\\xc0'\n"
      "const string over1 = '\xC1';\n"
      "# expected-error@-1: invalid UTF-8 start byte '\\xc1'\n"
      "const string over2 = '\xE0\x9F\x80';\n"
      "# expected-error@-1: invalid UTF-8 continuation byte '\\x9f'\n"
      "const string over3 = '\xF0\x8F\x80\x80';\n"
      "# expected-error@-1: invalid UTF-8 continuation byte '\\x8f'\n"
      // Invalid surrogates:
      "const string sur = '\xED\xA0\x80';\n"
      "# expected-error@-1: invalid UTF-8 continuation byte '\\xa0'\n");
}

TEST(CompilerTest, invalid_hex_escape) {
  check_compile(R"(
    const string s = "\x";
      # expected-error@-1: invalid `\x` escape sequence
  )");
}

TEST(CompilerTest, invalid_unicode_escape) {
  check_compile(R"(
    const string s = "\u";
      # expected-error@-1: invalid `\u` escape sequence
  )");
}

TEST(CompilerTest, invalid_escape) {
  check_compile(R"(
    const string s = "\*";
      # expected-error@-1: invalid escape sequence `\*`
  )");
}

TEST(CompilerTest, surrogate_in_unicode_escape) {
  check_compile(R"(
    const string s = "\ud800";
      # expected-error@-1: surrogate in `\u` escape sequence
  )");
}

TEST(CompilerTest, qualified_interaction_name) {
  std::map<std::string, std::string> name_contents_map;
  name_contents_map["foo.thrift"] = "interaction I {}";
  name_contents_map["bar.thrift"] = R"(
    include "foo.thrift"
    service S {
      foo.I createI();
    }
  )";
  check_compile(name_contents_map, "bar.thrift");
}

TEST(CompilerTest, py3_enum_invalid_value_names) {
  check_compile(
      R"(
    enum Foo {
      name = 1,
        # expected-error@-1: 'name' should not be used as an enum/union field name in thrift-py3. Use a different name or annotate the field with `(py3.name="<new_py_name>")` [enum-member-union-field-names-rule]
      value = 2 (py3.name = "value_"),
        # expected-warning@-1: The annotation py3.name is deprecated. Please use @python.Name instead.
    }

    enum Bar {
      name = 1 (py3.name = "name_"),
        # expected-warning@-1: The annotation py3.name is deprecated. Please use @python.Name instead.
      value = 2,
        # expected-error@-1: 'value' should not be used as an enum/union field name in thrift-py3. Use a different name or annotate the field with `(py3.name="<new_py_name>")` [enum-member-union-field-names-rule]
    }
  )",
      {"--gen", "mstch_py3"});
}

TEST(CompilerTest, py3_invalid_field_names) {
  check_compile(
      R"(
    union Foo {
      1: string name;
        # expected-error@-1: 'name' should not be used as an enum/union field name in thrift-py3. Use a different name or annotate the field with `(py3.name="<new_py_name>")` [enum-member-union-field-names-rule]
      2: i32 value (py3.name = "value_");
        # expected-warning@-1: The annotation py3.name is deprecated. Please use @python.Name instead.
    }

    union Bar {
      1: string name (py3.name = "name_");
        # expected-warning@-1: The annotation py3.name is deprecated. Please use @python.Name instead.
      2: i32 value;
        # expected-error@-1: 'value' should not be used as an enum/union field name in thrift-py3. Use a different name or annotate the field with `(py3.name="<new_py_name>")` [enum-member-union-field-names-rule]
    }
  )",
      {"--gen", "mstch_py3"});
}

TEST(CompilerTest, error_on_non_explicit_includes) {
  std::map<std::string, std::string> name_contents_map;
  name_contents_map["path/to/upstream.thrift"] = R"(
    struct TransitiveStruct {}
    enum TransitiveEnum {
      BLAH = 2,
    }
    union TransitiveUnion {
      1: i64 a;
      2: double b;
    }
    typedef TransitiveStruct TransitiveTypedef

    service TransitiveService {}
    exception TransitiveException {}
    interaction TransitiveInteraction {}
  )";

  name_contents_map["path/to/direct.thrift"] = R"(
    include "path/to/upstream.thrift"

    struct IncludedStruct {}
    enum IncludedEnum {
      BLAH = 2,
    }
    union IncludedUnion {
      1: i64 a;
      2: double b;
    }
    typedef IncludedStruct IncludedTypedef
  )";

  name_contents_map["path/to/transitive_struct_field.thrift"] = R"(
    # expected-error@3#original[]#replacement[include "path/to/upstream.thrift"\n]@5: Your thrift file depends on a type that it did not include. Please add the following include. [implicit-include]
    include "path/to/direct.thrift"

    struct A {
      1: i64 a;
      2: string b;
      3: binary c;
      4: list<i64> la;
      5: set<double> sd;
      6: map<binary, byte> mbb;
    }

    enum TransitiveEnum {
      E1 = 1,
      E2 = 2,
      E3 = 4,
    }

    struct B {
      1: A a;
      2: TransitiveEnum e;
    }

    struct TransitiveStruct {
      1: upstream.TransitiveStruct c;
      2: direct.IncludedUnion d;
      3: B b;
      4: upstream.TransitiveEnum e;
      5: TransitiveEnum te;
      6: list<upstream.TransitiveEnum> lte;
      7: set<TransitiveEnum> ste;
      8: set<upstream.TransitiveEnum> sute;
      11: map<i64, direct.IncludedEnum> mdie;
      12: map<i64, upstream.TransitiveStruct> muts;
      @upstream.TransitiveStruct
      42: i32 annotated;
    }

    union TransitiveUnion {
      1: upstream.TransitiveUnion u;
      2: direct.IncludedUnion d;
      3: upstream.TransitiveTypedef utt;
      4: direct.IncludedEnum e;
      5: list<upstream.TransitiveTypedef> lutt;
      6: set<direct.IncludedTypedef> sdit;
    }

    typedef B bprime
    typedef map<TransitiveEnum, B> map_te_b
    typedef upstream.TransitiveTypedef utt_prime
    typedef direct.IncludedTypedef dtt_prime
    typedef set<upstream.TransitiveTypedef> sutt_prime

    const upstream.TransitiveStruct c = upstream.TransitiveStruct{};

    service TransitiveService extends upstream.TransitiveService {
      upstream.TransitiveStruct foo(1: upstream.TransitiveStruct x) throws (1: upstream.TransitiveException ex);
      upstream.TransitiveInteraction, stream<upstream.TransitiveStruct> bar();
    }
  )";
  check_compile(name_contents_map, "path/to/transitive_struct_field.thrift");
}

TEST(CompilerTest, include_module_name_collision) {
  std::map<std::string, std::string> name_contents_map;
  name_contents_map["path/to/dep_module.thrift"] = R"(
    struct A {}
  )";
  name_contents_map["other_path/to/dep_module.thrift"] = R"(
    struct B {}
  )";
  name_contents_map["path/to/my_module.thrift"] = R"(
    include "path/to/dep_module.thrift"
    include "other_path/to/dep_module.thrift"
    struct C {
      1: dep_module.A a;
      2: dep_module.B b;
    }
  )";
  check_compile(name_contents_map, "path/to/my_module.thrift");
}

TEST(CompilerTest, alias_enum) {
  check_compile(R"(
    typedef Enum MyEnum
    enum Enum {
      ONE = 1,
    }
    struct MyStruct {
      1: MyEnum field = MyEnum.ONE;
    }
  )");
}

TEST(CompilerTest, alias_enum_in_external_const) {
  std::map<std::string, std::string> name_contents_map;
  name_contents_map["foo.thrift"] = "enum E {Val = 0}";
  name_contents_map["bar.thrift"] = R"(
    include "foo.thrift"
    typedef foo.E MyEnum
    const MyEnum val = MyEnum.Val;
  )";
  name_contents_map["baz.thrift"] = R"(
    include "bar.thrift"
    const bar.MyEnum val = bar.val;
  )";
  check_compile(name_contents_map, "baz.thrift");
}

TEST(CompilerTest, same_named_field) {
  check_compile(R"(
    struct S {
      1: i32 S;
      # expected-warning@-1: Field 'S' has the same name as the containing struct.
    }
    union U {
      1: i32 U;
      # expected-warning@-1: Field 'U' has the same name as the containing union.
    }
    exception E {
      1: i32 E;
      # expected-warning@-1: Field 'E' has the same name as the containing exception.
    }
  )");
}

TEST(CompilerTest, cursor_serialization_adapter) {
  check_compile(R"(
    include "thrift/annotation/cpp.thrift"

    struct Regular {
      @cpp.Adapter{name = "::apache::thrift::CursorSerializationAdapter"} # expected-error: CursorSerializationAdapter is not supported on fields. Place it on the top-level struct/union instead.
      1: i32 field;
      2: Adapted adapted; # expected-error: CursorSerializationAdapter is not supported on fields. Place it on the top-level struct/union instead.
      3: set<Adapted> container; # expected-error: CursorSerializationAdapter is not supported inside containers.
    }

    @cpp.UseCursorSerialization
    struct Adapted {}

    service Service {
      Adapted allowed(1: Adapted adapted);
      Regular not_allowed(
        1: Regular regular,
        2: Adapted adapted # expected-error: CursorSerializationAdapter only supports single-argument functions.
      );
    }
  )");
}

TEST(CompilerTest, exception_invalid_use) {
  check_compile(R"(
    exception E {}
    struct S {
      1: E field; # TODO (T191018859)
    }

    service Svc {
      E return(); # expected-error: Exceptions cannot be used as function return types
      void param(1: E e); # expected-error: Exceptions cannot be used as function arguments
    }

    const E e = E{}; # expected-error: Exceptions cannot be used as const types
  )");
}

TEST(CompilerTest, duplicate_include) {
  check_compile(R"(
    include "thrift/annotation/cpp.thrift"
    include "thrift/annotation/thrift.thrift"
    include "thrift/annotation/cpp.thrift" # expected-warning: Duplicate include of `thrift/annotation/cpp.thrift`
  )");
}

TEST(CompilerTest, not_bundled_annotation) {
  auto options = check_compile_options();
  options.add_standard_includes = false;

  // Change the current directory to a temp directory to avoid annotations
  // being found via relative includes.
  std::filesystem::current_path(std::filesystem::temp_directory_path());

  check_compile(
      R"(
      include "thrift/annotation/baz.thrift"
      # expected-error-1: Could not find include file thrift/annotation/baz.thrift
      )",
      {},
      options);

  // The (hard-coded) bundles should still exist without implicit includes.
  check_compile(
      R"(
      include "thrift/annotation/cpp.thrift"
      include "thrift/lib/thrift/schema.thrift"
      )",
      {},
      options);
}

TEST(CompilerTest, circular_typedef) {
  check_compile(R"(
    typedef Foo Bar # expected-error: Circular typedef: Foo --> Bar --> Foo
    typedef Bar Foo
  )");
}

TEST(CompilerTest, combining_unstructured_annotations) {
  check_compile(R"(
    include "thrift/annotation/thrift.thrift"

    @thrift.DeprecatedUnvalidatedAnnotations{items = {"foo": "bar"}}
    struct Foo {} (hs.foo)

    @thrift.DeprecatedUnvalidatedAnnotations{items = {"foo": "bar"}}
    struct Bar {} (rust.foo)
    # expected-error@-2: Cannot combine @thrift.DeprecatedUnvalidatedAnnotations with legacy annotation syntax.
  )");
}

TEST(Compilertest, custom_default_values) {
  check_compile(
      R"(
    include "thrift/annotation/thrift.thrift"

    union TestUnion {}

    struct TestStruct {}

    exception TestException {}


    struct Widget {
      1: i32 a = 42;

      2: bool b = false;
        # expected-warning@-1: Explicit default value is redundant for (unqualified) field: `b` (in `Widget`).

      3: i32 c = 0;
        # expected-warning@-1: Explicit default value is redundant for (unqualified) field: `c` (in `Widget`).

      4: optional bool d = true;
        # expected-warning@-1: Optional field should not have custom default value: `d` (in `Widget`).

      @thrift.TerseWrite
      5: i32 e = 43;
        # expected-error@-2: Terse field should not have custom default value: `e` (in `Widget`).

      6: TestUnion f = {};
        # expected-warning@-1: Explicit default value is redundant for (unqualified) field: `f` (in `Widget`).

      7: TestStruct g = {};
        # expected-warning@-1: Explicit default value is redundant for (unqualified) field: `g` (in `Widget`).

      8: TestException h = {};
        # expected-warning@-1: Explicit default value is redundant for (unqualified) field: `h` (in `Widget`).

      @thrift.TerseWrite
      9: i32 i = 0;
        # expected-warning@-2: Explicit default value is redundant for (terse) field: `i` (in `Widget`).

      10: optional bool j = false;
        # expected-warning@-1: Explicit default value is redundant for (optional) field: `j` (in `Widget`).
        # expected-warning@-2: Optional field should not have custom default value: `j` (in `Widget`).
    }

    union UnionWidget {
      1: i32 a;

      2: i32 b = 0;
        # expected-warning@-1: Union field is implicitly optional and should not have custom default value: `b` (in union `UnionWidget`).

      3: bool c = true;
        # expected-warning@-1: Union field is implicitly optional and should not have custom default value: `c` (in union `UnionWidget`).
    }
  )",
      {"--extra-validation", "warn_on_redundant_custom_default_values"});
}

TEST(CompilerTest, cpp_deprecated_terse_write) {
  check_compile(R"(
    include "thrift/annotation/cpp.thrift"
    include "thrift/annotation/thrift.thrift"

    struct Bar {}

    struct Foo {
      @cpp.DeprecatedTerseWrite
      1: i32 field1;

      @cpp.DeprecatedTerseWrite
      2: optional i32 field2;
        # expected-error@-2: @cpp.DeprecatedTerseWrite can be only used on unqualified field.

      @cpp.DeprecatedTerseWrite
      3: required i32 field3;
        # expected-warning@-2: The 'required' qualifier is deprecated and ignored by most language implementations. Leave the field unqualified instead.
        # expected-error@-3: @cpp.DeprecatedTerseWrite can be only used on unqualified field.

      @thrift.TerseWrite
      @cpp.DeprecatedTerseWrite
      4: i32 field4;
        # expected-error@-3: @cpp.DeprecatedTerseWrite can be only used on unqualified field.

      @cpp.DeprecatedTerseWrite
      5: Bar field5;
        # expected-error@-2: @cpp.DeprecatedTerseWrite is not supported for structured types.

      @cpp.AllowLegacyNonOptionalRef
      @cpp.DeprecatedTerseWrite
      @cpp.AllowLegacyDeprecatedTerseWritesRef
      @cpp.Ref{type = cpp.RefType.Unique}
      6: Bar field6;
        # expected-warning@-5: Field with @cpp.Ref (or similar) annotation should be optional: `field6` (in `Foo`).
    }
  )");
}

TEST(CompilerTest, cpp_deprecated_terse_write_ref) {
  check_compile(R"(
    include "thrift/annotation/cpp.thrift"
    include "thrift/annotation/thrift.thrift"

    struct Foo {
      @cpp.AllowLegacyNonOptionalRef
      @cpp.DeprecatedTerseWrite
      @cpp.Ref{type = cpp.RefType.Unique}
      1: string field1;
        # expected-warning@-4: Field with @cpp.Ref (or similar) annotation should be optional: `field1` (in `Foo`).
        # expected-error@-5: @cpp.Ref{Unique} can not be applied to `field1` since it's cpp.DeprecatedTerseWrite field.

      @cpp.AllowLegacyNonOptionalRef
      @cpp.DeprecatedTerseWrite
      @cpp.AllowLegacyDeprecatedTerseWritesRef
      @cpp.Ref{type = cpp.RefType.Unique}
      2: string field2;
        # expected-warning@-5: Field with @cpp.Ref (or similar) annotation should be optional: `field2` (in `Foo`).

      @cpp.DeprecatedTerseWrite
      @cpp.AllowLegacyDeprecatedTerseWritesRef
      3: string field3;
        # expected-error@-3: @cpp.AllowLegacyDeprecatedTerseWritesRef can not be applied to `field3` since it's not cpp.Ref{Unique} field.

      @cpp.AllowLegacyNonOptionalRef
      @cpp.AllowLegacyDeprecatedTerseWritesRef
      @cpp.Ref{type = cpp.RefType.Unique}
      4: string field4;
        # expected-warning@-4: Field with @cpp.Ref (or similar) annotation should be optional: `field4` (in `Foo`).
        # expected-error@-5: @cpp.AllowLegacyDeprecatedTerseWritesRef can not be applied to `field4` since it's not cpp.DeprecatedTerseWrite field.
    }
  )");
}

TEST(CompilerTest, base_service_defined_after_use) {
  check_compile(R"(
    typedef Base BaseAlias

    service Derived extends Base {}  # expected-error: Service "Base" has not been defined.
    service DerivedWithAlias extends BaseAlias {}  # expected-error: Service "BaseAlias" has not been defined.

    service Base {}
  )");
}

TEST(CompilerTest, cpp_orderable) {
  check_compile(R"(
include "thrift/annotation/cpp.thrift"
include "thrift/annotation/thrift.thrift"

struct AlwaysOrderable {
  1: set<i32> a;
}

@cpp.EnableCustomTypeOrdering
struct UnnecessaryAnnotation {
  1: set<i32> a;
}

struct UnorderableCustomType {
  @cpp.Type{name = "custom"}
  1: set<i32> a;
}

@thrift.Uri{value = "facebook.com/thrift/ImplicitlyEnabledCustomType"}
# expected-warning@-1: Type `ImplicitlyEnabledCustomType` is implicitly made orderable in C++ because it has a URI. This legacy behavior is being deprecated: enable ordering explicitly by annotating the type with `@cpp.EnableCustomTypeOrdering`.
struct ImplicitlyEnabledCustomType {
  @cpp.Type{name = "custom"}
  1: set<i32> a;
}

@cpp.EnableCustomTypeOrdering
struct ExplicitlyEnabledCustomType {
  @cpp.Type{name = "custom"}
  1: set<i32> a;
}

@thrift.Uri{value = "facebook.com/thrift/ExplicitlyEnabledCustomTypeWithUri"}
@cpp.EnableCustomTypeOrdering
struct ExplicitlyEnabledCustomTypeWithUri {
  @cpp.Type{name = "custom"}
  1: set<i32> a;
}
)");
}

TEST(CompilerTest, cpp_orderable_unnecessary_annotation_error) {
  check_compile(
      R"(
include "thrift/annotation/cpp.thrift"
include "thrift/annotation/thrift.thrift"

@cpp.EnableCustomTypeOrdering
# expected-error@-1: Type `UnnecessaryAnnotation` does not need `@cpp.EnableCustomTypeOrdering` to be orderable in C++: remove the annotation.
struct UnnecessaryAnnotation {
  1: set<i32> a;
}
)",
      {"--extra-validation", "unnecessary_enable_custom_type_ordering=error"});
}

TEST(CompilerTest, cpp_orderable_unnecessary_annotation_warn) {
  check_compile(
      R"(
include "thrift/annotation/cpp.thrift"
include "thrift/annotation/thrift.thrift"

@cpp.EnableCustomTypeOrdering
# expected-warning@-1: Type `UnnecessaryAnnotation` does not need `@cpp.EnableCustomTypeOrdering` to be orderable in C++: remove the annotation.
struct UnnecessaryAnnotation {
  1: set<i32> a;
}
)",
      {"--extra-validation", "unnecessary_enable_custom_type_ordering=warn"});
}

TEST(CompilerTest, invalid_include_alias) {
  std::map<std::string, std::string> name_contents_map;

  name_contents_map["foo.thrift"] = "";
  name_contents_map["bar.thrift"] = "";
  name_contents_map["baz.thrift"] = "";
  name_contents_map["main.thrift"] = R"(
    include "baz.thrift" as const; # expected-error: Invalid include alias 'const'
    include "foo.thrift" as ; # expected-error: Include alias cannot be empty
    include "bar.thrift" as 123; # expected-error: Invalid include alias 'int literal'
  )";

  check_compile(name_contents_map, "main.thrift");
}

TEST(CompilerTest, duplicate_include_alias) {
  std::map<std::string, std::string> name_contents_map;

  name_contents_map["a/b/c/foo.thrift"] = "";
  name_contents_map["d/e/bar.thrift"] = "";
  name_contents_map["main.thrift"] = R"(
    include "a/b/c/foo.thrift" as foo;
    include "d/e/bar.thrift" as foo; # expected-error: 'foo' is already an alias for 'a/b/c/foo.thrift'
    include "a/b/c/foo.thrift" as foo2; # expected-error: Include 'a/b/c/foo.thrift' has multiple aliases: 'foo' vs 'foo2'
  )";

  check_compile(name_contents_map, "main.thrift");
}

TEST(CompilerTest, resolve_enum_typedefs_after_use) {
  std::map<std::string, std::string> name_contents_map;
  name_contents_map["remote.thrift"] = R"(
    typedef Remote LazyRemoteAlias;

    enum Remote {
      FOO = 1,
    }

    typedef Remote RemoteAlias;
  )";
  name_contents_map["main.thrift"] = R"(
    include "remote.thrift"

    typedef Local LazyLocalAlias;

    struct S {
      1: remote.Remote field_v1 = remote.Remote.FOO;
      2: remote.Remote field_v2 = remote.RemoteAlias.FOO;
      3: remote.Remote field_v3 = remote.FOO;
      4: remote.Remote field_v4 = LocalAliasForRemote.FOO;
      5: Local field_v5 = Local.BAR;
      6: Local field_v6 = LocalAlias.BAR;
      7: Local field_v8 = BAR;
    }

    const remote.Remote var_v1 = remote.Remote.FOO;
    const remote.Remote var_v2 = remote.RemoteAlias.FOO;
    const remote.Remote var_v3 = remote.FOO;
    const remote.Remote var_v4 = remote.LazyRemoteAlias.FOO;
    const remote.Remote var_v5 = LocalAliasForRemote.FOO;

    const Local LAZY_BAR_ALIAS = Local.BAR;
    const Local LAZY_BAR_ALIAS2 = main.BAR;

    const Local var_v6 = Local.BAR;
    const Local var_v7 = LocalAlias.BAR;
    const Local var_v8 = BAR;
    const Local var_v9 = LazyLocalAlias.BAR;
    const Local var_v10 = main.LazyLocalAlias.BAR;
    const Local var_v11 = LAZY_BAR_ALIAS;
    const Local var_v12 = main.LAZY_BAR_ALIAS;
    const Local var_v13 = LAZY_BAR_ALIAS2;
    const Local var_v14 = main.LAZY_BAR_ALIAS2;

    enum Local {
      BAR = 2,
    }

    typedef remote.Remote LocalAliasForRemote;
    typedef Local LocalAlias;
  )";
  check_compile(name_contents_map, "main.thrift");
}

TEST(CompilerTest, include_alias) {
  std::map<std::string, std::string> name_contents_map;
  name_contents_map["foo.thrift"] = R"(
    struct FooBar {
      1: string name;
    }
  )";
  name_contents_map["bar.thrift"] = R"(
    struct FooBar {
      1: i32 id;
    }
  )";
  name_contents_map["main.thrift"] = R"(
    include "foo.thrift" as foo_alias
    include "bar.thrift" as bar_alias
  )";
  check_compile(name_contents_map, "main.thrift");
}

// [TEMPORARY] This verifies that unscoped identifiers in injected metadata
// annotations are resolved to the program they're defined in.
TEST(CompilerTest, scope_resolution_for_unscoped_injected_metadata) {
  std::map<std::string, std::string> name_contents_map;
  name_contents_map["module.thrift"] = R"(
    include "thrift/annotation/internal.thrift"

    struct Foo {
      1: map<i16, i32> metadata;
    }

    @internal.InjectMetadataFields{type = "Foo"}
    struct MyInjected {}

    @internal.InjectMetadataFields{type = "module.Foo"}
    struct MyInjected2 {}

  )";
  name_contents_map["main.thrift"] = R"(
    include "module.thrift"

    @module.MyInjected
    struct S1 {
    }

    @module.MyInjected2
    struct S2 {
    }
  )";
  check_compile(name_contents_map, "main.thrift");
}

TEST(CompilerTest, local_vs_global_resolution_sanity) {
  for (const bool use_global_resolution : {true, false}) {
    std::map<std::string, std::string> name_content_map;

    name_content_map["c/foo.thrift"] = R"(
      struct Something {
        1: i32 val;
      }

      struct Bar {
        1: Something field;
      }
    )";

    name_content_map["a/foo.thrift"] = R"(
      include "c/foo.thrift"
      struct Bar {
        1: string field;
      }
    )";

    name_content_map["b/foo.thrift"] = R"(
      include "a/foo.thrift"

      struct Bar {
        1: i32 field;
      }
    )";

    name_content_map["local/foo.thrift"] = R"(
      include "a/foo.thrift"
      include "b/foo.thrift"
      include "c/foo.thrift"

      struct Inner {
        1: string val;
      }

      struct Bar {
        1: Inner field;
      }

      // This should always resolve to the local Bar
      const Bar bar = Bar{field = Inner{val = "hello"}};
    )";

    name_content_map["get_b/foo.thrift"] = R"(
      include "a/foo.thrift"
      include "b/foo.thrift"
      include "c/foo.thrift"

      // This should resolve to b's Bar, as it was parsed last
      // in the hierarchy
      const foo.Bar bar = foo.Bar{field = 12};
    )";

    auto options = check_compile_options();
    options.use_global_resolution = use_global_resolution;

    check_compile(name_content_map, "local/foo.thrift", {}, options);
    check_compile(name_content_map, "get_b/foo.thrift", {}, options);
  }
}

TEST(CompilerTest, include_alias_resolution_priority) {
  std::map<std::string, std::string> name_contents_map;
  name_contents_map["foo.thrift"] = R"(
    struct Bar {
      1: string name;
    }
  )";
  name_contents_map["other/Foo.thrift"] = R"(
    struct Bar {
      1: i32 code;
    }
  )";
  name_contents_map["bar.thrift"] = R"(
    include "other/Foo.thrift"
  )";

  name_contents_map["main.thrift"] = R"(
    include "foo.thrift" as Foo
    // This transitively includes other/Foo.thrift & should overwrite Foo.Bar
    // Which clashes with the alias above
    include "bar.thrift"

    const Foo.Bar MY_BAR = Foo.Bar{name = "hello"};
  )";
  check_compile(name_contents_map, "main.thrift");
}

TEST(CompilerTest, include_alias_resolution_does_not_fallback) {
  std::map<std::string, std::string> name_contents_map;
  name_contents_map["foo.thrift"] = R"(
    struct Annot {
      1: i32 val;
    }

    const i32 MY_VAL = 11;
  )";
  name_contents_map["a/Foo.thrift"] = R"(
    const string MY_VAL = "hello";
  )";

  name_contents_map["bar.thrift"] = R"(
    include "a/Foo.thrift"
  )";

  name_contents_map["main.thrift"] = R"(
    include "foo.thrift" as Foo
    include "bar.thrift"

    @Foo.Annot{ val = Foo.MY_VAL }
    struct S {
    }
  )";
  check_compile(name_contents_map, "main.thrift");
}

TEST(CompilerTest, scope_resolution_duplicate_include_as_alias) {
  std::map<std::string, std::string> name_contents_map;
  name_contents_map["v1/foo.thrift"] = R"(
    struct Bar {
      1: string name;
    }
  )";

  name_contents_map["foo.thrift"] = R"(
    include "v1/foo.thrift"
    include "v1/foo.thrift" as FooV2 # expected-warning: Duplicate include of `v1/foo.thrift`

    struct Bar {
      1: i32 id;
    }

    // The `foo.Bar` identifier is overriden by the local definition, despite an explicit include
    const foo.Bar MY_REMOTE_CONST_V1 = foo.Bar{name = "foo"}; # expected-error: no field named `name` in `Bar`
    const FooV2.Bar MY_REMOTE_CONST_V2 = FooV2.Bar{name = "foo"};
    const Bar MY_LOCAL_CONST = Bar{id = 1};
  )";

  check_compile(name_contents_map, "foo.thrift");
}

TEST(CompilerTest, report_unresolved_identifiers_in_structured_annotations) {
  std::map<std::string, std::string> name_contents_map;
  name_contents_map["foo.thrift"] = R"(
    struct MyAnnot {
      1: string name;
      2: i32 id;
      3: list<float> values;
    }
  )";

  name_contents_map["main.thrift"] = R"(
    include "foo.thrift"

    @foo.MyAnnot{ name = foo.DOES_NOT_EXIST } # expected-error: use of undeclared identifier 'foo.DOES_NOT_EXIST'
    struct S {}

    @foo.MyAnnot{ id = foo.DOES_NOT_EXIST } # expected-error: use of undeclared identifier 'foo.DOES_NOT_EXIST'
    struct S2 {}

    @foo.MyAnnot{ values = [
      foo.DOES_NOT_EXIST, # expected-error: use of undeclared identifier 'foo.DOES_NOT_EXIST'
      ALSO_DOES_NOT_EXIST # expected-error: use of undeclared identifier 'ALSO_DOES_NOT_EXIST'
    ] }
    struct S3 {}
  )";
  check_compile(name_contents_map, "main.thrift");
}

TEST(CompilerTest, reported_unresolved_identifiers_in_constants) {
  std::map<std::string, std::string> name_contents_map;
  name_contents_map["foo.thrift"] = R"(
  )";

  name_contents_map["main.thrift"] = R"(
    include "foo.thrift"

    const i32 MY_CONST = foo.MY_CONST; # expected-error: use of undeclared identifier 'foo.MY_CONST'
    const string MY_CONST2 = foo.MY_CONST; # expected-error: use of undeclared identifier 'foo.MY_CONST'
    const list<i32> MY_CONST3 = foo.MY_CONST; # expected-error: use of undeclared identifier 'foo.MY_CONST'
    const list<i32> MY_CONST4 = [
      foo.MY_CONST, # expected-error: use of undeclared identifier 'foo.MY_CONST'
      foo.MY_CONST2 # expected-error: use of undeclared identifier 'foo.MY_CONST2'
    ];
    const map<i32, i32> MY_CONST5 = foo.MY_CONST; # expected-error: use of undeclared identifier 'foo.MY_CONST'
    const map<i32, i32> MY_CONST6 = { foo.MY_CONST_KEY: # expected-error: use of undeclared identifier 'foo.MY_CONST_KEY'
      foo.MY_CONST_VALUE }; # expected-error: use of undeclared identifier 'foo.MY_CONST_VALUE'
    const set<i32> MY_CONST7 = foo.MY_CONST; # expected-error: use of undeclared identifier 'foo.MY_CONST'
    const set<i32> MY_CONST8 = [ foo.MY_CONST ]; # expected-error: use of undeclared identifier 'foo.MY_CONST'
  )";
  check_compile(name_contents_map, "main.thrift");
}

TEST(CompilerTest, unresolved_struct_in_const) {
  std::map<std::string, std::string> name_contents_map;
  name_contents_map["main.thrift"] = R"(
    struct MyDeclaredStruct {
        1: i64 some_field;
    }
    const MyDeclaredStruct X = MyUndeclaredStruct{};
    # expected-error-1: could not resolve type `MyUndeclaredStruct` (expected `main.MyDeclaredStruct`)
  )";
  check_compile(name_contents_map, "main.thrift");
}
