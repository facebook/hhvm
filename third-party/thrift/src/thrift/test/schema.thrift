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

include "thrift/annotation/scope.thrift"
include "thrift/annotation/cpp.thrift"
include "thrift/lib/thrift/patch.thrift"
include "thrift/annotation/thrift.thrift"
include "thrift/lib/thrift/any.thrift"
// @nolint used to test depending on this file

package "facebook.com/thrift/test/schema"

struct Empty {}

struct Renamed {}

struct Fields {
  1: i32 num;
  3: optional set<string> keyset;
  7: Empty strct;
}

struct Defaults {
  1: i32 none;
  2: i32 some = 42;
}

service EmptyService {
}

union Union {}

enum Enum {
  unspecified = 0,
  test = 22,
}

exception SimpleException {}

safe transient server exception FancyException {}

const i32 IntConst = 11;

const list<i32> ListConst = [2, 3, 5, 7, IntConst];

exception NonSchematizedException {}

struct NonSchematizedStruct {
  1: i32 none;
  2: i32 some = 42;
  3: list<NonSchematizedEnum> enm;
}

enum NonSchematizedEnum {
  unspecified = 0,
  test = 22,
}

union NonSchematizedUnion {}

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

@thrift.AllowLegacyTypedefUri
typedef i32 TD
struct Typedefs {
  1: TD named;
  @cpp.Type{name = "uint32_t"}
  2: i32 unnamed;
}

@thrift.AllowLegacyTypedefUri
typedef TD TDTD

@scope.Structured
struct Annot {
  1: i32 val;
}

@Annot{val = 42}
struct Annotated {} (annot_with_val = 2023, annot_without_val)
