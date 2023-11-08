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
      2: MyStruct ms; # expected-error: Type `test.MyStruct` not defined.
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
      0: i32 field; # expected-warning: Nonpositive field id (0) differs from what is auto-assigned by thrift. The id must be positive or -1.
                    # expected-warning@-1:  No field id specified for `field`, resulting protocol may have conflicts or not be backwards compatible!
      1: list<i32> other;
    }
  )");
}

TEST(CompilerTest, zero_as_field_id_neg_keys) {
  check_compile(
      R"(
      struct Foo {
        0: i32 field; # expected-warning: Nonpositive field id (0) differs from what would be auto-assigned by thrift (-1).
                      # expected-error@-1: Zero value (0) not allowed as a field id for `field`
        1: list<i32> other;
      }
      )",
      {"--allow-neg-keys"});
}

TEST(CompilerTest, no_field_id) {
  check_compile(R"(
    struct Experimental {} (thrift.uri = "facebook.com/thrift/annotation/Experimental")
    struct Foo {
      @Experimental
      i32 field2; # expected-warning@-1: No field id specified for `field2`, resulting protocol may have conflicts or not be backwards compatible!
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
        # expected-warning@-1: Nonpositive field id (0) differs from what is auto-assigned by thrift. The id must be positive or -1.
        # expected-warning@-2: No field id specified for `field`, resulting protocol may have conflicts or not be backwards compatible!

      1: list<i32> other;
    }
  )");
}

TEST(CompilerTest, zero_as_field_id_allow_neg_keys) {
  check_compile(
      R"(
      struct Foo {
        0: i32 field (cpp.deprecated_allow_zero_as_field_id);
          # expected-warning@-1: Nonpositive field id (0) differs from what would be auto-assigned by thrift (-1).

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
          # expected-warning@-1: No field id specified for `f1`, resulting protocol may have conflicts or not be backwards compatible!

        -2: i32 f2; // auto and manual id = -2
        -32: i32 f3; // min value.
          # expected-warning@-1: Nonpositive field id (-32) differs from what would be auto-assigned by thrift (-3).

        -33: i32 f4; // min value - 1.
          # expected-error@-1: Reserved field id (-33) cannot be used for `f4`.
      }
      )",
      {"--allow-neg-keys"});
}

TEST(CompilerTest, exhausted_neg_field_ids) {
  check_compile(
      R"(
      struct Foo {
        -32: i32 f1; // min value.
          # expected-warning@-1: Nonpositive field id (-32) differs from what would be auto-assigned by thrift (-1).

        i32 f2; // auto id = -2 or min value - 1
          # expected-error@-1: Cannot allocate an id for `f2`. Automatic field ids are exhausted.
      }
      )",
      {"--allow-neg-keys"});
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
      -32768: i32 f1; # expected-warning: Nonpositive field id (-32768) differs from what is auto-assigned by thrift. The id must be positive or -1.
      32767: i32 f2;
      32768: i32 f3; # expected-error: Integer constant 32768 outside the range of field ids ([-32768, 32767]).
                     # expected-warning@-1: Nonpositive field id (-32768) differs from what is auto-assigned by thrift. The id must be positive or -2.
    }
  )");
}

TEST(CompilerTest, out_of_range_field_ids_underflow) {
  check_compile(R"(
    struct Foo {
      -32768: i32 f1; # expected-warning: Nonpositive field id (-32768) differs from what is auto-assigned by thrift. The id must be positive or -1.
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
    const i32 wrongInt = "stringVal"; # expected-error: type error: const `wrongInt` was declared as i32.
    const set<string> wrongSet = {1: 2};
      # expected-warning@-1: type error: const `wrongSet` was declared as set. This will become an error in future versions of thrift.
    const map<i32, i32> wrongMap = [1,32,3];
      # expected-warning@-1: type error: const `wrongMap` was declared as map. This will become an error in future versions of thrift.
    const map<i32, i32> weirdMap = [];
      # expected-warning@-1: type error: map `weirdMap` initialized with empty list.
    const set<i32> weirdSet = {};
      # expected-warning@-1: type error: set `weirdSet` initialized with empty map.
    const list<i32> weirdList = {};
      # expected-warning@-1: type error: list `weirdList` initialized with empty map.
    const list<string> badValList = [1];
      # expected-error@-1: type error: const `badValList<elem>` was declared as string.
    const set<string> badValSet = [2];
      # expected-error@-1: type error: const `badValSet<elem>` was declared as string.
    const map<string, i32> badValMap = {1: "str"};
      # expected-error@-1: type error: const `badValMap<key>` was declared as string.
      # expected-error@-2: type error: const `badValMap<val>` was declared as i32.
  )");
}

TEST(CompilerTest, const_byte_value) {
  check_compile(R"(
    const byte c1 = 127;
    const byte c2 = 128;
    # expected-error@-1: value error: const `c2` has an invalid custom default value.

    const byte c3 = -128;
    const byte c4 = -129;
    # expected-error@-1: value error: const `c4` has an invalid custom default value.
  )");
}

TEST(CompilerTest, const_i16_value) {
  check_compile(R"(
    const i16 c1 = 32767;
    const i16 c2 = 32768;
    # expected-error@-1: value error: const `c2` has an invalid custom default value.

    const i16 c3 = -32768;
    const i16 c4 = -32769;
    # expected-error@-1: value error: const `c4` has an invalid custom default value.
  )");
}

TEST(CompilerTest, const_i32_value) {
  check_compile(R"(
    const i32 c1 = 2147483647;
    const i32 c2 = 2147483648;
    # expected-warning@-1: 64-bit constant 2147483648 may not work in all languages
    # expected-error@-2: value error: const `c2` has an invalid custom default value.

    const i32 c3 = -2147483648;
    const i32 c4 = -2147483649;
    # expected-warning@-1: 64-bit constant -2147483649 may not work in all languages
    # expected-error@-2: value error: const `c4` has an invalid custom default value.
  )");
}

TEST(CompilerTest, const_float_value) {
  check_compile(R"(
    const float c0 = 1e8;

    const float c1 = 3.4028234663852886e+38;
    const float c2 = 3.402823466385289e+38; // max float + 1 double ulp
    # expected-error@-1: value error: const `c2` has an invalid custom default value.

    const float c3 = -3.4028234663852886e+38;
    const float c4 = -3.402823466385289e+38; // min float - 1 double ulp
    # expected-error@-1: value error: const `c4` has an invalid custom default value.

    const float c5 = 100000001;
    # expected-error@-1: value error: const `c5` cannot be represented precisely as `float` or `double`.
    const float c6 = -100000001;
    # expected-error@-1: value error: const `c6` cannot be represented precisely as `float` or `double`.
  )");
}

TEST(CompilerTest, const_double_value) {
  check_compile(R"(
    const double c0 = 1e8;
    const double c1 = 1.7976931348623157e+308;
    const double c2 = -1.7976931348623157e+308;

    const float c3 = 10000000000000001;
    # expected-warning@-1: 64-bit constant 10000000000000001 may not work in all languages
    # expected-error@-2: value error: const `c3` cannot be represented precisely as `float` or `double`.

    const float c4 = -10000000000000001;
    # expected-warning@-1: 64-bit constant -10000000000000001 may not work in all languages
    # expected-error@-2: value error: const `c4` cannot be represented precisely as `float` or `double`.
  )");
}

TEST(CompilerTest, struct_initializer) {
  check_compile(R"(
    struct S {}
    const S s1 = {};   # OK
    const S s2 = 42;   # expected-error: integer is incompatible with `S`
    const S s3 = 4.2;  # expected-error: floating-point number is incompatible with `S`
    const S s4 = "";   # expected-error: string is incompatible with `S`
    const S s5 = true; # expected-error: bool is incompatible with `S`
    const S s6 = [];   # expected-error: list is incompatible with `S`
  )");
}

TEST(CompilerTest, union_initializer) {
  check_compile(R"(
    union U {}
    const U u1 = {};   # OK
    const U u2 = 42;   # expected-error: integer is incompatible with `U`
    const U u3 = 4.2;  # expected-error: floating-point number is incompatible with `U`
    const U u4 = "";   # expected-error: string is incompatible with `U`
    const U u5 = true; # expected-error: bool is incompatible with `U`
    const U u6 = [];   # expected-error: list is incompatible with `U`
  )");
}

TEST(CompilerTest, struct_fields_wrong_type) {
  check_compile(R"(
    struct Annot {
      1: i32 val;
      2: list<string> otherVal;
    }

    @Annot{val="hi", otherVal=5}
      # expected-error@-1: type error: const `.val` was declared as i32.
      # expected-warning@-2: type error: const `.otherVal` was declared as list. This will become an error in future versions of thrift.
    struct BadFields {
      1: i32 badInt = "str"; # expected-error: type error: const `badInt` was declared as i32.
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

TEST(CompilerTest, undefined_annotation) {
  check_compile(R"(
    @BadAnnotation # expected-error: Type `test.BadAnnotation` not defined.
    struct S {}

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
      1: A a (cpp.mixin);
      2: B b (cpp.mixin); # expected-error: Field `B.i` and `A.i` can not have same name in `C`.
    }
  )");
  check_compile(R"(
    struct A { 1: i32 i; }

    struct C {
      1: A a (cpp.mixin);
      2: i64 i; # expected-error: Field `C.i` and `A.i` can not have same name in `C`.
    }
  )");
}

TEST(CompilerTest, annotation_positions) {
  check_compile(R"(
    typedef set<set<i32> (annot)> T # expected-error: Annotations are not allowed in this position. Extract the type into a named typedef instead.
    const i32 (annot) C = 42 # expected-error: Annotations are not allowed in this position. Extract the type into a named typedef instead.
    service S {
      i32 (annot) foo() # expected-error: Annotations are not allowed in this position. Extract the type into a named typedef instead.
      void bar(1: i32 (annot) p) # expected-error: Annotations are not allowed in this position. Extract the type into a named typedef instead.
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
    include "thrift/annotation/hack.thrift"

    @hack.Attributes{attributes=[]} # expected-error: Duplicate annotations hack.attributes and @hack.Attributes.
    struct A {
        1: optional i64 field (cpp.box) # expected-warning: The annotation cpp.box is deprecated. Please use @thrift.Box instead.
        2: i64 with (py3.name = "w", go.name = "w") # expected-warning: The annotation py3.name is deprecated. Please use @python.Name instead.
        # expected-warning@-1: The annotation go.name is deprecated. Please use @go.Name instead.
    } (hack.attributes = "")
  )");
}

TEST(CompilerTest, invalid_enum_constant) {
  check_compile(R"(
    enum E {}
    const list<E> c = [nonexistant.Value]; # expected-error: type error: no matching constant: nonexistant.Value
    # expected-error@-1: type error: const `c<elem>` was declared as enum.
    # expected-warning@-2: The identifier 'nonexistant.Value' is not defined yet. Constants and enums should be defined before using them as default values.
    # expected-warning@-3: type error: const `c<elem>` was declared as enum `E` with a value not of that enum.
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
      1: i32 (cpp.type = "std::uint32_t") field; # expected-warning@-1: The cpp.type/cpp.template annotations are deprecated, use @cpp.Type instead
      @cpp.Adapter{name="Adapter"} # expected-warning: At most one of @cpp.Type/@cpp.Adapter/cpp.type/cpp.template can be specified on a definition.
      @cpp.Type{name="std::uint32_t"}
      2: i32 field2;
    }
  )");
}
