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

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>

#include <thrift/compiler/test/compiler.h>

namespace apache::thrift::compiler {

using test::check_compile;

using ::testing::UnorderedElementsAre;

TEST(StandardValidatorTest, InterfaceNamesUniqueNoError) {
  check_compile(R"(
    service Service {
      void bar();
      void baz();
    }

    interaction Interaction {
      void bar();
      void baz();
    }
  )");
}

TEST(StandardValidatorTest, BadPriority) {
  check_compile(R"(
    service Service {
      void foo() (priority = "bad2");
    } (priority = "bad1")
    # expected-error@-3: Bad priority 'bad1'. Choose one of ["HIGH_IMPORTANT", "HIGH", "IMPORTANT", "NORMAL", "BEST_EFFORT"].
    # expected-warning@-4: The annotation priority is deprecated. Please use @thrift.Priority instead.
    # expected-error@-4: Bad priority 'bad2'. Choose one of ["HIGH_IMPORTANT", "HIGH", "IMPORTANT", "NORMAL", "BEST_EFFORT"].
    # expected-warning@-5: The annotation priority is deprecated. Please use @thrift.Priority instead.

    interaction Interaction {
      void foo() (priority = "bad4");
    } (priority = "bad3")
    # expected-error@-3: Bad priority 'bad3'. Choose one of ["HIGH_IMPORTANT", "HIGH", "IMPORTANT", "NORMAL", "BEST_EFFORT"].
    # expected-warning@-4: The annotation priority is deprecated. Please use @thrift.Priority instead.
    # expected-error@-4: Bad priority 'bad4'. Choose one of ["HIGH_IMPORTANT", "HIGH", "IMPORTANT", "NORMAL", "BEST_EFFORT"].
    # expected-warning@-5: The annotation priority is deprecated. Please use @thrift.Priority instead.
  )");
}

TEST(StandardValidatorTest, ReapeatedNamesInService) {
  check_compile(R"(
    service Service {
      void foo();
      void foo(); # expected-error: Function `foo` is already defined for `Service`.
      void bar();
    }

    interaction Interaction {
      void bar();
      void bar(); # expected-error: Function `bar` is already defined for `Interaction`.
      void foo();
    }
  )");
}

TEST(StandardValidatorTest, RepeatedNameInExtendedService) {
  check_compile(R"(
    service Base {
      void bar();
      void baz();
    }

    service Derived extends Base { 
      void foo();
      void baz(); # expected-error: Function `Derived.baz` redefines `test.Base.baz`.
    }
  )");
}

TEST(StandardValidatorTest, EnumErrors) {
  check_compile(R"(
    enum DoesNotContainDuplicate {
      FOO = 1,
      BAR = 2,
    }    

    enum ContainsDuplicateValue {
      FOO = 1,
      BAR = 2,
      BAZ = 1, # expected-error: Duplicate value `BAZ=1` with value `FOO` in enum `ContainsDuplicateValue`.
    }

    enum ContainsDuplicateName {
      FOO = 1,
      BAR = 2,
      FOO = 3, # expected-error: Enum value `FOO` is already defined for `ContainsDuplicateName`.
    }

    enum DoesNotHaveExplicitValues {
      FOO = 1,
      BAR, # expected-error: The enum value, `BAR`, must have an explicitly assigned value.
      BAZ, # expected-error: The enum value, `BAZ`, must have an explicitly assigned value.
    }
  )");
}

TEST(StandardValidatorTest, UnionErrors) {
  check_compile(R"(
    struct Struct {}

    union Union {
      1: required i64 req; # expected-error: Unions cannot contain qualified fields. Remove `required` qualifier from field `req`.
        # expected-warning@-1: The 'required' qualifier is deprecated and ignored by most language implementations. Leave the field unqualified instead.
      2: optional i64 opt; # expected-error: Unions cannot contain qualified fields. Remove `optional` qualifier from field `opt`.
      3: Struct mixin (cpp.mixin); # expected-error: Union `Union` cannot contain mixin field `mixin`.
        # expected-warning@-1: The annotation cpp.mixin is deprecated. Please use @thrift.Mixin instead.
      4: i64 non;
    }
  )");
}

TEST(StandardValidatorTest, MixinFieldType) {
  check_compile(R"(
    struct Struct {}
    union Union {}
    exception Exception {}

    struct Foo {
      1: optional Struct struct_field (cpp.mixin); # expected-error: Mixin field `struct_field` cannot be optional.
        # expected-warning@-1: The annotation cpp.mixin is deprecated. Please use @thrift.Mixin instead.
      2: Union union_field (cpp.mixin);
        # expected-warning@-1: The annotation cpp.mixin is deprecated. Please use @thrift.Mixin instead.
      3: Exception except_field (cpp.mixin); # expected-error: Mixin field `except_field` type must be a struct or union. Found `Exception`.
        # expected-warning@-1: The annotation cpp.mixin is deprecated. Please use @thrift.Mixin instead.
      4: i32 other_field (cpp.mixin); # expected-error: Mixin field `other_field` type must be a struct or union. Found `i32`.
        # expected-warning@-1: The annotation cpp.mixin is deprecated. Please use @thrift.Mixin instead.
    }
  )");
}

TEST(StandardValidatorTest, CustomDefaaultValue) {
  check_compile(R"(
    const byte const_byte = 128; # expected-error: value error: const `const_byte` has an invalid custom default value.
    const i16 const_short = 32768; # expected-error: value error: const `const_short` has an invalid custom default value.
    const i32 const_integer = 2147483648; # expected-error: value error: const `const_integer` has an invalid custom default value.
    const float const_float = 3.402823466385289e+38; # expected-error: value error: const `const_float` has an invalid custom default value.
    const float const_float_precision_loss = 2147483647; # expected-error: value error: const `const_float_precision_loss` cannot be represented precisely as `float` or `double`.
  )");
}

TEST(StandardValidatorTest, ValidateExceptionMessage) {
  check_compile(R"(
    include "thrift/annotation/thrift.thrift"

    exception MyValidException {
      @thrift.ExceptionMessage
      1: string valid_message;
    }

    exception MyInvalidException { # expected-error: member specified as exception 'message' should be of type STRING, 'invalid_message' in 'MyInvalidException' is not
      @thrift.ExceptionMessage
      1: i32 invalid_message; 
    }

    exception MyExceptionWithDuplicatedMessage {
      @thrift.ExceptionMessage
      1: string valid_message;
      @thrift.ExceptionMessage  # expected-error: Duplicate message annotation.
      2: string invalid_message;
    }

    exception MyExceptionWithDuplicatedDeprecatedExceptionMessage { # expected-error: Duplicate message annotation.
      # expected-warning@-1: The annotation message is deprecated. Please use @thrift.ExceptionMessage instead.
      @thrift.ExceptionMessage
      1: string valid_message;
      2: string invalid_message;
    } (message = "invalid_message") 
        
  )");
}

} // namespace apache::thrift::compiler
