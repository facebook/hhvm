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

cpp_include "<deque>"
cpp_include "<list>"
cpp_include "<map>"
cpp_include "<set>"
cpp_include "<unordered_map>"
cpp_include "<unordered_set>"

cpp_include "<folly/container/F14Map.h>"
cpp_include "<folly/container/F14Set.h>"
cpp_include "<folly/sorted_vector_types.h>"

struct Fields {
  1: list<i32> listField;
  2: StructWithListField structField;

  3: list<list<i32>> nestedListField;
  4: list<list<StructWithListField>> nestedStructListField;
  5: map<i32, list<StructWithListField>> nestedStructListMapField;

  @cpp.Ref{type = cpp.RefType.SharedMutable}
  6: list<i32> listFieldRef;
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  7: StructWithListField structFieldRef;
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  8: optional list<i32> optListFieldRef;
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  9: optional StructWithListField optStructFieldRef;
  @thrift.Box
  10: optional list<i32> listFieldBoxRef;
  @thrift.Box
  11: optional StructWithListField structFieldBoxRef;
}

struct StructWithListField {
  1: list<i32> listField;
} (cpp.declare_hash)

struct StructWithListFieldVector {
  1: list<StructWithListField> field;
}

struct StructWithListFieldList {
  @cpp.Type{template = "std::list"}
  1: list<StructWithListField> field;
}

struct StructWithListFieldDeque {
  @cpp.Type{template = "std::deque"}
  1: list<StructWithListField> field;
}

struct StructWithListFieldSet {
  1: set<StructWithListField> field;
}

struct StructWithListFieldUnorderedSet {
  @cpp.Type{template = "std::unordered_set"}
  1: set<StructWithListField> field;
}

struct StructWithListFieldF14FastSet {
  @cpp.Type{template = "folly::F14FastSet"}
  1: set<StructWithListField> field;
}

struct StructWithListFieldF14VectorSet {
  @cpp.Type{template = "folly::F14VectorSet"}
  1: set<StructWithListField> field;
}

struct StructWithListFieldSortedVectorSet {
  @cpp.Type{template = "folly::sorted_vector_set"}
  1: set<StructWithListField> field;
}

struct StructWithListFieldMap {
  1: map<i32, StructWithListField> field;
}

struct StructWithListFieldUnorderedMap {
  @cpp.Type{template = "std::unordered_map"}
  1: map<i32, StructWithListField> field;
}

struct StructWithListFieldF14FastMap {
  @cpp.Type{template = "folly::F14FastMap"}
  1: map<i32, StructWithListField> field;
}

struct StructWithListFieldF14VectorMap {
  @cpp.Type{template = "folly::F14VectorMap"}
  1: map<i32, StructWithListField> field;
}

struct StructWithListFieldSortedVectorMap {
  @cpp.Type{template = "folly::sorted_vector_map"}
  1: map<i32, StructWithListField> field;
}

struct DoubleStructWithListFieldMap {
  1: map<StructWithListField, StructWithListField> field;
}

struct DoubleStructWithListFieldUnorderedMap {
  @cpp.Type{template = "std::unordered_map"}
  1: map<StructWithListField, StructWithListField> field;
}

struct DoubleStructWithListFieldF14FastMap {
  @cpp.Type{template = "folly::F14FastMap"}
  1: map<StructWithListField, StructWithListField> field;
}

struct DoubleStructWithListFieldF14VectorMap {
  @cpp.Type{template = "folly::F14VectorMap"}
  1: map<StructWithListField, StructWithListField> field;
}
