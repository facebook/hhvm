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
  1: HasInt def_field;
}

struct BasicRefsShared {
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  1: HasInt def_field;
}

@cpp.Type{name = "WrappedTypeField<Basic>"}
typedef Basic (cpp.indirection) BasicIndirection

@cpp.Type{name = "WrappedTypeField<folly::IOBuf>"}
typedef binary (cpp.indirection) t_foo
@cpp.Type{name = "WrappedTypeField<std::string>"}
typedef binary (cpp.indirection) t_bar
@cpp.Type{name = "WrappedTypeMethod<folly::IOBuf>"}
typedef binary (cpp.indirection) t_baz
struct IOBufIndirection {
  1: t_foo foo;
  2: t_bar bar;
  3: t_baz baz;
}

struct HasSmallSortedVector {
  @cpp.Type{template = "SmallSortedVectorSet"}
  1: set<i32> set_field;
  @cpp.Type{template = "SmallSortedVectorMap"}
  2: map<i32, i32> map_field;
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
