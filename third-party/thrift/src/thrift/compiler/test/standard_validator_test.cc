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
        # expected-warning@-1: The 'required' qualifier is deprecated and ignored by most language implementations. Leave the field unqualified instead: `req` (in `Union`).
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

TEST(StandardValidatorTest, ValidatePy3EnableCppAdapter) {
  check_compile(R"(
    include "thrift/annotation/cpp.thrift"
    include "thrift/annotation/python.thrift"

    @python.Py3EnableCppAdapter
    @cpp.Adapter{name = "MyAdapter"}
    typedef list<i32> MyIntList

    @python.Py3EnableCppAdapter # expected-error: The @python.Py3EnableCppAdapter annotation can only be used on containers and strings.
    @cpp.Adapter{name = "MyAdapter"}
    typedef i32 MyInt

    @python.Py3EnableCppAdapter # expected-error: The @python.Py3EnableCppAdapter annotation requires the @cpp.Adapter annotation to be present in the same typedef.
    typedef list<i32> MyIntList2

    @python.Py3EnableCppAdapter # expected-error: The @python.Py3EnableCppAdapter annotation requires the @cpp.Adapter annotation to be present in the same typedef.
      # expected-error@-1: The @python.Py3EnableCppAdapter annotation can only be used on containers and strings.
    typedef i32 MyInt2
  )");
}

TEST(StandardValidatorTest, ConstKeyCollision) {
  check_compile(R"(
    enum FooBar {
      Foo = 1,
      Bar = 2,
    }

    const map<FooBar, string> ENUM_OK = {
      FooBar.Foo: "Foo",
      FooBar.Bar: "Bar"
    }

    const map<FooBar, string> ENUM_DUPE = {
      FooBar.Foo: "Foo",
      FooBar.Bar: "Bar",
      FooBar.Bar: "Bar"
    # expected-warning@-1: Duplicate key in map literal: `Bar`
    }

    const map<i32, string> ENUM_DUPE_COERCE = {
    FooBar.Bar: "Bar",
    FooBar.Foo: "Foo",
    1: "Bar2",
    # expected-error@-1: Duplicate key in map literal: `1`
    2: "Foo2",
    # expected-error@-1: Duplicate key in map literal: `2`
}

    const map<i64, string> USEFUL_DATA = {
      1: "a",
      2: "b",
      1: "c",
    # expected-error@-1: Duplicate key in map literal: `1`
    };

    const string GREETING = "hey";
    const string HELLO = "hello";
    const string SALUTATION = "hey";

    const map<string, string> ARTIFICIAL_INTELLIGENCE = {
      GREETING: "a",
      HELLO: "b",
      SALUTATION: "c",
    # expected-error@-1: Duplicate key in map literal: `hey`
    };

    const list<map<string, i64>> LIST_NESTING = [
      {"str": 1},
      {"foo": 1, "bar": 2, "foo": 3},
    # expected-error@-1: Duplicate key in map literal: `foo`
      {"str": 1},
    ];

    const list<map<i64, string>> CASCADING = [
      {1: "str"},
      // Verify no duplicate error on USEFUL_DATA 
      USEFUL_DATA,
      {1: "str"},
    ];

    struct Building {
      1: map<i64, string> int_str;
    }

    const Building B = Building {
      int_str = {4: "a", 5: "b", 4: "c"},
    # expected-error@-1: Duplicate key in map literal: `4`
    };

    const Building C = Building {
      // Verify no duplicate error on USEFUL_DATA 
      int_str = USEFUL_DATA,
    };


  )");
}

TEST(StandardValidatorTest, FieldDefaultKeyCollision) {
  check_compile(R"(
    enum FooBar {
      Foo = 1,
      Bar = 2,
    }

    const map<i64, string> INT_DUPE = {
      2: "Foo",
      4: "Bar",
      2: "Bar"
    # expected-error@-1: Duplicate key in map literal: `2`
    }

    struct S {
      1: map<FooBar, string> ok_init = {};
      2: map<i64, string> bad_init_no_err = INT_DUPE;
      3: map<FooBar, string> bad_init_should_err = {
        FooBar.Foo: "Foo", FooBar.Bar: "Bar", FooBar.Bar: "Bar"
    # expected-warning@-1: Duplicate key in map literal: `Bar`
      };
      4: map<list<i64>, i64> bad_init_list_key = {
        [1, 1, 2]: 1, [1, 1, 2]: 2
    # expected-error-1: Duplicate key in map literal: `[1, ..., 2]`
      };
    }
  )");
}

TEST(StandardValidatorTest, SetKeyCollision) {
  check_compile(R"(
    const set<i64> SET_DUPE = [
      2,
      4,
      2,
    # expected-warning@-1: Duplicate key in set literal: `2`
      4,
    # expected-warning@-1: Duplicate key in set literal: `4`
    ];

    const list<i64> LIST_DUPE = [2, 2, 2, 4, 2, 4];

    const set<set<i64>> NESTED_SET = [[2], [4], [2]];
    # expected-warning@-1: Duplicate key in set literal: `[2]`

    const list<set<i64>> NESTED_IDENTIFIER = [[2], SET_DUPE];

    struct S {
      1: set<string> ok_init = [];
      2: set<string> dupe_init_set = ["a", "b", "a"];
    # expected-warning@-1: Duplicate key in set literal: `a`
      3: list<string> dupe_init_list = ["a", "b", "a"];
      4: set<i64> set_from_named_const = SET_DUPE;
    }
  )");
}
