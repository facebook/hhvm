/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

namespace cpp2 apache.thrift.test

struct Person {
  1: i64 id;
  2: string name;
}

struct SimpleRef {
  1: Person c2s (cpp2.ref_type = "shared");
  2: Person c2u (cpp2.ref_type = "unique");
  3: Person c1r (cpp.ref = "true");
  4: Person c2r (cpp2.ref = "true");
  9: Person c2sc (cpp2.ref_type = "shared_const");
  5: optional Person c2s_opt (cpp2.ref_type = "shared");
  6: optional Person c2u_opt (cpp2.ref_type = "unique");
  7: optional Person c1r_opt (cpp.ref = "true");
  8: optional Person c2r_opt (cpp2.ref = "true");
}

struct Node {
  1: i64 id;
  2: optional Node left (cpp.ref = "true");
  3: optional Node right (cpp.ref = "true");
  4: string content;
}

struct SharedRef {
  1: Person p1 (cpp2.ref_type = "shared");
  2: Person p2 (cpp2.ref_type = "shared");
  5: i64 id;
}

struct SharedRef2 {
  2: Person p2 (cpp2.ref_type = "shared");
  5: i64 id;
}

struct LinkedListNode {
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  1: optional LinkedListNode next;
  2: i64 id;
}
