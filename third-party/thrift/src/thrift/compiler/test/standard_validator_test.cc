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

#include <thrift/compiler/test/compiler.h>

using apache::thrift::compiler::test::check_compile;

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
