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

include "thrift/annotation/thrift.thrift"
include "thrift/annotation/scope.thrift"
include "thrift/lib/thrift/any.thrift"

package "thrift.com/python/schema/ts_bridge"

namespace py3 thrift.lib.python.schema.tests

typedef i64 MyId

typedef Inner InnerAlias

struct Inner {
  1: i32 value;
}

struct Outer {
  1: MyId id; // typedef-to-primitive: erases to i64
  2: Inner inner; // struct reference
  3: InnerAlias aliased; // typedef-to-struct: erases to Inner
  4: list<Inner> inners; // container of a struct reference
}

struct HasAny {
  1: any.Any any_field; // IDL `any` -> ANY primitive (resolved via bundled omnibus)
}

exception MyError {
  1: string message;
  2: i32 code;
}

union MyUnion {
  1: i32 int_field;
  2: string str_field;
}

struct SelfRef {
  @thrift.Box
  1: optional SelfRef next;
}

struct Mutual1 {
  @thrift.Box
  1: optional Mutual2 other;
}

struct Mutual2 {
  @thrift.Box
  1: optional Mutual1 back;
}

struct Presence {
  1: i32 plain; // UNQUALIFIED
  2: optional i32 maybe; // OPTIONAL
  @thrift.TerseWrite
  3: i32 terse; // TERSE
}

enum Color {
  RED = 0,
  GREEN = 1,
  BLUE = 2,
}

// A user-defined structured annotation. @thrift.RuntimeAnnotation keeps it in
// the runtime schema so the bridge can read its applied values.
@thrift.RuntimeAnnotation
@scope.Structured
@scope.Field
struct RecordAnno {
  1: i32 count;
  2: string label;
}

// Carries the annotation at both the definition level and on a field, so the
// bridge populates node.annotations and field.annotations.
@RecordAnno{count = 7, label = "outer"}
struct Annotated {
  @RecordAnno{count = 3, label = "field"}
  1: i32 value;
}

// Fields with IDL custom defaults, exercising the protocol-Value -> record path.
struct HasDefaults {
  1: i32 num = 42;
  2: string label = "hello";
  3: list<i32> nums = [1, 2, 3];
  4: bool flag = true;
}
