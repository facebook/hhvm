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

namespace cpp2 apache.thrift.test

include "thrift/annotation/thrift.thrift"
include "thrift/annotation/cpp.thrift"

@thrift.AllowLegacyMissingUris
package;

cpp_include "thrift/test/StructsExtra.h"

struct Basic {
  1: i32 def_field;
  2: required i32 req_field;
  3: optional i32 opt_field;
}

struct BasicBinaries {
  1: binary def_field;
  2: required binary req_field;
  3: optional binary opt_field;
}

struct HasInt {
  1: required i32 field;
}

struct BasicRefs {
  @cpp.Ref{type = cpp.RefType.Unique}
  @cpp.AllowLegacyNonOptionalRef
  1: HasInt def_field;
}

struct BasicRefsShared {
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  @cpp.AllowLegacyNonOptionalRef
  1: HasInt def_field;
}

struct HasSmallSortedVector {
  @cpp.Type{name = "folly::small_sorted_vector_set<int32_t>"}
  1: set<i32> set_field;

  @cpp.Type{name = "folly::small_sorted_vector_map<int32_t, int32_t>"}
  2: i32_map map_field;
}

struct NoexceptMoveStruct {
  1: string string_field;
  2: i32 i32_field;
}

struct EmptiableOptionalFieldsStruct {
  1: optional i32 int_field;

  @thrift.Box
  2: optional list<i32> int_list_field_ref;
}

struct NotEmptiableStruct {
  1: optional i32 int_field;

  @thrift.Box
  2: optional list<i32> int_list_field_ref;

  3: i64 long_field;
}

struct OptionalFieldsStruct {
  @cpp.Ref{type = cpp.RefType.Unique}
  1: optional HasInt def_field;

  @cpp.Ref{type = cpp.RefType.SharedMutable}
  2: optional HasInt shared_field;

  @cpp.Ref{type = cpp.RefType.SharedMutable}
  3: optional list<HasInt> shared_fields;

  @cpp.Ref{type = cpp.RefType.Shared}
  4: optional HasInt shared_field_const;

  @cpp.Ref{type = cpp.RefType.Shared}
  5: optional list<HasInt> shared_fields_const;

  @thrift.Box
  6: optional HasInt boxed_field;
}

struct NotEligibleForConstexpr {
  @cpp.Ref{type = cpp.RefType.Unique}
  1: optional i32 swap;
}

typedef map<i32, i32> i32_map

struct AllContainersStruct {
  1: list<i32> list_field;
  2: set<i32> set_field;
  3: map<i32, i32> map_field;
  4: optional list<i32> list_field_opt;
  5: optional set<i32> set_field_opt;
  6: optional map<i32, i32> map_field_opt;
}
