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

namespace swift FBThriftTestsRecursive

// Directly self-referential struct: `next` requires heap indirection in Swift
// (@thrift.Box). `children` is container recursion, which is naturally indirect.
struct Node {
  1: i32 value;
  @thrift.Box
  2: optional Node next;
  3: list<Node> children;
}
