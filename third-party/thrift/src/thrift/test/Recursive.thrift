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
include "thrift/test/Recursive2.thrift"
include "thrift/annotation/cpp.thrift"

@thrift.AllowLegacyMissingUris
package;

namespace py3 thrift.test
namespace cpp2 apache.thrift.test

cpp_include "folly/sorted_vector_types.h"

struct RecTree {
  1: list<RecTree> children;
  2: i16 item;
}

struct RecList {
  @cpp.Ref{type = cpp.RefType.Unique}
  1: optional RecList next;
  3: i16 item;
}

struct CoRec {
  @cpp.Ref{type = cpp.RefType.Unique}
  1: optional CoRec2 other;
}

struct CoRec2 {
  1: CoRec other;
}

struct VectorTest {
  1: list<RecList> lister;
}

struct MyField {
  1: optional i64 some_val;
}

struct MyStruct {
  @cpp.Ref{type = cpp.RefType.Unique}
  1: optional MyField field;
}

struct StructUsingOtherNamespace {
  @cpp.Ref{type = cpp.RefType.Unique}
  1: optional Recursive2.OtherStruct other;
}

struct StructUsingAnnotation {
  @thrift.DeprecatedUnvalidatedAnnotations{items = {"cpp.box": "1"}}
  1: optional MyField field;
}

struct StructUsingThriftBox {
  @thrift.Box
  1: optional MyField field;
}
