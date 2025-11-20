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
include "thrift/annotation/cpp.thrift"

namespace cpp2 apache.thrift.test

union Basic {
  2: string str;
  1: i64 int64;
  4: list<i32> list_i32;
}

union DuplicateType {
  1: string str1;
  2: list<i32> list_i32;
  3: string str2;
}

union TreeNode {
  1: list<TreeNode> nodes;
  2: i32 data;
}

union CppRef {
  1: string str;

  @cpp.Ref{type = cpp.RefType.Unique}
  2: CppRef cppref;

  @cpp.Ref{type = cpp.RefType.SharedMutable}
  3: CppRef shared_mutable;

  @thrift.Box
  4: CppRef box;

  @cpp.Ref{type = cpp.RefType.Shared}
  5: string shared_const;
}

@thrift.DeprecatedUnvalidatedAnnotations{
  items = {"cpp.methods": "
   void foo() {}
"},
}
union CppMethods {
  1: i32 field;
}
