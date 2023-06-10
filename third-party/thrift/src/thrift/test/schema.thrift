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
// @lint-ignore THRIFTCHECKS used by GenerateRuntimeSchema
include "thrift/lib/thrift/schema.thrift"
include "thrift/annotation/cpp.thrift"

package "facebook.com/thrift/test/schema"

@thrift.GenerateRuntimeSchema
struct Empty {}

@thrift.GenerateRuntimeSchema{name = "RenamedSchema"}
struct Renamed {}

@thrift.GenerateRuntimeSchema
struct Fields {
  1: i32 num;
  3: optional set<string> keyset;
  7: Empty strct;
}

@thrift.GenerateRuntimeSchema
struct Defaults {
  1: i32 none;
  2: i32 some = 42;
}

@thrift.GenerateRuntimeSchema
service EmptyService {
}

@thrift.GenerateRuntimeSchema
union Union {}

@thrift.GenerateRuntimeSchema
enum Enum {
  unspecified = 0,
  test = 22,
}

// @lint-ignore THRIFTCHECKS
@thrift.GenerateRuntimeSchema
exception SimpleException {}

@thrift.GenerateRuntimeSchema
safe transient server exception FancyException {}

@thrift.GenerateRuntimeSchema
const i32 IntConst = 11;

@thrift.GenerateRuntimeSchema
const list<i32> ListConst = [2, 3, 5, 7, IntConst];

// @lint-ignore THRIFTCHECKS
exception NonSchematizedException {}

struct NonSchematizedStruct {
  1: i32 none;
  2: i32 some = 42;
  3: NonSchematizedEnum enm;
}

enum NonSchematizedEnum {
  unspecified = 0,
  test = 22,
}

union NonSchematizedUnion {}

@thrift.GenerateRuntimeSchema
service TestService {
  void noParamsNoReturnNoEx();
  i32 noParamsPrimitiveReturnNoEx();
  void primitiveParamsNoReturnEx(1: i32 param0) throws (
    1: NonSchematizedException ex0,
  );
  NonSchematizedStruct unionParamStructReturnNoEx(
    1: NonSchematizedUnion param0,
  );
}

@thrift.GenerateRuntimeSchema
typedef i32 TD
@thrift.GenerateRuntimeSchema
struct Typedefs {
  1: TD named;
  @cpp.Type{name = "uint32_t"}
  2: i32 unnamed;
}

@thrift.GenerateRuntimeSchema
typedef TD TDTD

@scope.Structured
struct Annot {
  1: i32 val;
}

@thrift.GenerateRuntimeSchema
@Annot{val = 42}
struct Annotated {} (annot_with_val = 2023, annot_without_val)
