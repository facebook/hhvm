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

// Minimal structs exercising `get_locally_set_fields()`. The thrift_library compiling
// this file passes `thrift_python_options = ["enable_isset_deprecated_unsafe"]`,
// which enables `get_locally_set_fields()` for every struct in the library.

include "thrift/annotation/thrift.thrift"
include "thrift/annotation/python.thrift"

package "facebook.com/thrift/python/test/deprecated"

namespace py3 "thrift.python.test.deprecated"

enum Kind {
  UNKNOWN = 0,
  REGULAR = 8,
}

struct File {
  1: string name;
  2: i32 permissions;
  3: Kind type = Kind.REGULAR;
}

struct OptionalFile {
  1: optional string name;
  3: optional i32 type;
}

exception UnusedError {
  1: string message;
}

union Integers {
  4: i64 large;
}

struct Optionals {
  1: optional list<string> values;
}

struct SimpleStruct {
  1: string name;
  2: i32 value;
  3: string city;
}

struct mixed {
  @thrift.AllowUnsafeOptionalCustomDefaultValue
  1: optional string opt_field = "optional";
  3: string unq_field = "unqualified";
  @python.Name{name = "some_field_"}
  7: optional string some_field;
  @thrift.AllowUnsafeOptionalCustomDefaultValue
  9: optional i32 opt_int = 1;
}

struct Nested {
  1: i32 num;
  2: optional string label;
  // Optional containers of structs, nested recursively, to exercise the
  // shadow walk when an optional list/map/set field is left unset (None).
  3: optional list<Nested> maybe_children;
  4: optional map<i32, Nested> maybe_map;
  // @lint-ignore THRIFTCHECKS bad-key-type
  5: optional set<Nested> maybe_set;
}

// Exercises isset tracking for structs reached through a nested field and
// through `list`/`map`/`set` containers, including nested containers.
struct HasContainers {
  1: Nested nested;
  2: list<Nested> nested_list;
  3: map<i32, Nested> nested_map;
  // @lint-ignore THRIFTCHECKS bad-key-type
  4: set<Nested> nested_set;
  5: list<list<Nested>> nested_list_of_lists;
  6: map<i32, list<Nested>> nested_map_of_lists;
}

@python.EnableUnsafeIssetInspection
struct StructWithIssetInspection {
  1: i32 int_field;
  2: optional string opt_str_field;
  3: bool bool_field;
  4: optional list<i32> opt_list_field;
}

struct StructWithoutIssetInspection {
  1: i32 int_field;
  2: optional string opt_str_field;
}

@python.EnableUnsafeIssetInspection
struct StructWithUnannotatedChild {
  1: StructWithoutIssetInspection child;
}

struct StructWithAnnotatedChild {
  1: StructWithIssetInspection child;
}
