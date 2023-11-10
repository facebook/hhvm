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

namespace cpp2 apache.thrift.test

struct Person {
  1: i64 id;
  2: string name;
}

struct SimpleRef {
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  1: Person c2s;
  @cpp.Ref{type = cpp.RefType.Unique}
  2: Person c2u;
  @cpp.Ref{type = cpp.RefType.Unique}
  3: Person c1r;
  @cpp.Ref{type = cpp.RefType.Unique}
  4: Person c2r;
  @cpp.Ref{type = cpp.RefType.Shared}
  9: Person c2sc;
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  5: optional Person c2s_opt;
  @cpp.Ref{type = cpp.RefType.Unique}
  6: optional Person c2u_opt;
  @cpp.Ref{type = cpp.RefType.Unique}
  7: optional Person c1r_opt;
  @cpp.Ref{type = cpp.RefType.Unique}
  8: optional Person c2r_opt;
}

struct SimpleBoxed {
  @thrift.Box
  1: optional Person boxed;
}

struct Node {
  1: i64 id;
  @cpp.Ref{type = cpp.RefType.Unique}
  2: optional Node left;
  @cpp.Ref{type = cpp.RefType.Unique}
  3: optional Node right;
  4: string content;
}

struct BoxedNode {
  1: i64 id;
  @thrift.Box
  2: optional BoxedNode left;
  @thrift.Box
  3: optional BoxedNode right;
  4: string content;
}

struct SharedRef {
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  1: Person p1;
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  2: Person p2;
  5: i64 id;
}

struct SharedRef2 {
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  2: Person p2;
  5: i64 id;
}

struct LinkedListNode {
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  1: optional LinkedListNode next;
  2: i64 id;
}
