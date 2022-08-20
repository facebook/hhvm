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
  1: HasInt def_field (cpp.ref);
}

struct BasicRefsShared {
  1: HasInt def_field (cpp.ref_type = "shared");
}

typedef Basic (
  cpp.type = "WrappedTypeField<Basic>",
  cpp.indirection,
) BasicIndirection

typedef binary (
  cpp.type = "WrappedTypeField<folly::IOBuf>",
  cpp.indirection,
) t_foo
typedef binary (
  cpp.type = "WrappedTypeField<std::string>",
  cpp.indirection,
) t_bar
typedef binary (
  cpp.type = "WrappedTypeMethod<folly::IOBuf>",
  cpp.indirection,
) t_baz
struct IOBufIndirection {
  1: t_foo foo;
  2: t_bar bar;
  3: t_baz baz;
}

struct HasSmallSortedVector {
  1: set<i32> (cpp.template = "SmallSortedVectorSet") set_field;
  2: map<i32, i32> (cpp.template = "SmallSortedVectorMap") map_field;
}

struct NoexceptMoveStruct {
  1: string string_field;
  2: i32 i32_field;
}

struct CppDataMethod {
  1: i32 foo;
} (cpp.internal.deprecated._data.method)

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
  1: optional HasInt def_field (cpp.ref);
  2: optional HasInt shared_field (cpp.ref_type = "shared");
  3: optional list<HasInt> shared_fields (cpp.ref_type = "shared");
  4: optional HasInt shared_field_const (cpp.ref_type = "shared_const");
  5: optional list<HasInt> shared_fields_const (cpp.ref_type = "shared_const");
  @thrift.Box
  6: optional HasInt boxed_field;
}
