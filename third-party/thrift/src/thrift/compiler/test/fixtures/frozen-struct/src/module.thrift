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

include "thrift/annotation/cpp.thrift"
include "thrift/annotation/thrift.thrift"

include "include1.thrift"
include "include2.thrift"

@thrift.AllowLegacyMissingUris
package;

namespace cpp2 some.ns

struct ModuleA {
  1: i32 i32Field;
  2: string strField;
  3: list<i16> listField;
  4: map<string, i32> mapField;
  5: include1.IncludedA inclAField;
  6: include2.IncludedB inclBField;
}

enum EnumB {
  EMPTY = 1,
}

struct ModuleB {
  1: i32 i32Field;
  2: EnumB inclEnumB;
}

@cpp.Adapter{name = "::my::Adapter"}
struct DirectlyAdapted {
  1: i32 field;
}

struct CppRef {
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  @cpp.AllowLegacyNonOptionalRef
  1: i32 shared_field;
  @cpp.Ref{type = cpp.RefType.Shared}
  @cpp.AllowLegacyNonOptionalRef
  2: i32 shared_const_field;
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  3: optional i32 opt_shared_field;
  @cpp.Ref{type = cpp.RefType.Shared}
  4: optional i32 opt_shared_const_field;
  @thrift.Box
  5: optional i32 boxed_field;
}
