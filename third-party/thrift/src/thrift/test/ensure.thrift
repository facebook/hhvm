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

package "facebook.com/thrift/test/ensure"

include "thrift/annotation/cpp.thrift"
include "thrift/annotation/thrift.thrift"

cpp_include "thrift/test/AdapterTest.h"

struct FieldRefStruct {
  1: i32 field_i32;
  2: optional i32 optional_i32;
  @thrift.Box
  3: optional i32 boxed_i32;
  @thrift.Experimental
  @thrift.TerseWrite
  4: i32 terse_i32;
}

struct SmartPointerStruct {
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  1: optional i32 shared_i32;
  @cpp.Ref{type = cpp.RefType.Unique}
  2: optional i32 unique_i32;
  @cpp.Ref{type = cpp.RefType.Shared}
  3: optional i32 shared_const_i32;
}
