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
import FBThriftTestsUnions
import Foundation
import Testing

/// Round-trip tests for types generated from unions_test.thrift.
struct GeneratedUnionsTests {
  @Test func enumArmRoundTrip() throws {
    let u = MyUnion.my_enum(.MyValue2)
    let data = CompactSerializer.serialize(u)
    #expect(try CompactSerializer.deserialize(MyUnion.self, from: data) == u)
  }

  @Test func structArmRoundTrip() throws {
    let u = MyUnion.my_data_item(MyDataItem())
    let data = CompactSerializer.serialize(u)
    #expect(try CompactSerializer.deserialize(MyUnion.self, from: data) == u)
  }

  @Test func singleFieldUnionRoundTrip() throws {
    let u = SingleFieldUnion.reserved_field(-99)
    let data = CompactSerializer.serialize(u)
    #expect(
      try CompactSerializer.deserialize(SingleFieldUnion.self, from: data) == u)
  }

  @Test func typedefContainerUnionRoundTrip() throws {
    let listArm = TypedefContainerUnion.int_list_field([1, 2, 3])
    let mapArm = TypedefContainerUnion.string_int_map_field(["a": 1])
    for u in [listArm, mapArm] {
      let data = CompactSerializer.serialize(u)
      #expect(
        try CompactSerializer.deserialize(
          TypedefContainerUnion.self, from: data) == u)
    }
  }

  @Test func emptyUnionDecodesToEmpty() throws {
    // A union payload with only a field-stop has no field set -> empty state
    // (guide 2.3.14/2.3.15), not an error.
    let empty = Data([0])
    let decoded = try CompactSerializer.deserialize(MyUnion.self, from: empty)
    #expect(decoded == ._empty)
  }

  @Test func emptyUnionRoundTrips() throws {
    let u = MyUnion._empty
    let data = CompactSerializer.serialize(u)
    #expect(
      try CompactSerializer.deserialize(MyUnion.self, from: data) == ._empty)
  }

  @Test func hashableAsSetElementAndDictionaryKey() {
    // Guide 2.4.26: unions are Hashable, so they work as Set elements and
    // dictionary keys.
    let a = MyUnion.my_enum(.MyValue1)
    let b = MyUnion.my_enum(.MyValue2)
    let aCopy = MyUnion.my_enum(.MyValue1)

    let set: Set<MyUnion> = [a, b, aCopy, ._empty]
    #expect(set.count == 3)
    #expect(set.contains(a))
    #expect(set.contains(._empty))

    var byUnion: [MyUnion: String] = [:]
    byUnion[a] = "first"
    byUnion[aCopy] = "second"
    byUnion[b] = "third"
    #expect(byUnion[a] == "second")
    #expect(byUnion.count == 2)
  }
}
