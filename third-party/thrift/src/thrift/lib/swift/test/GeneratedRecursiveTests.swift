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

import FBThrift
import FBThriftTestsRecursive
import Foundation
import Testing

/// Round-trip tests for recursive types generated from recursive_test.thrift:
/// direct self-reference via @thrift.Box, and container recursion.
struct GeneratedRecursiveTests {
  @Test func boxedSelfReferenceRoundTrip() throws {
    var leaf = Node()
    leaf.value = 3
    var root = Node()
    root.value = 1
    root.next = leaf
    let data = CompactSerializer.serialize(root)
    let decoded = try CompactSerializer.deserialize(Node.self, from: data)
    #expect(decoded == root)
    #expect(decoded.next?.value == 3)
    #expect(decoded.next?.next == nil)
  }

  @Test func containerRecursionRoundTrip() throws {
    var childA = Node()
    childA.value = 2
    var childB = Node()
    childB.value = 4
    var root = Node()
    root.value = 1
    root.children = [childA, childB]
    let data = BinarySerializer.serialize(root)
    let decoded = try BinarySerializer.deserialize(Node.self, from: data)
    #expect(decoded == root)
    #expect(decoded.children.map(\.value) == [2, 4])
  }

  @Test func emptyRecursiveDefaults() {
    let n = Node()
    #expect(n.value == 0)
    #expect(n.next == nil)
    #expect(n.children == [])
  }

  @Test func boxedFieldHasValueSemantics() {
    // The point of @Indirect: a struct copy must not alias the original
    // through the shared heap box. Mutating the copy's boxed child must leave
    // the original untouched.
    var original = Node()
    original.value = 1
    var child = Node()
    child.value = 10
    original.next = child

    var copy = original
    if var mutatedChild = copy.next {
      mutatedChild.value = 999
      copy.next = mutatedChild
    }

    #expect(original.next?.value == 10)
    #expect(copy.next?.value == 999)
  }
}
