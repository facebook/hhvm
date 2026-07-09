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
import FBThriftTestsStructs
import Foundation
import Testing

/// Round-trip tests for types generated from structs_test.thrift.
struct GeneratedStructsTests {
  @Test func structRoundTrip() throws {
    var s = MyStruct()
    s.my_int_field = 42
    s.my_string_field = "hello"
    s.my_enum = .MyValue2
    s.oneway = true
    let data = CompactSerializer.serialize(s)
    let decoded = try CompactSerializer.deserialize(MyStruct.self, from: data)
    #expect(decoded == s)
    #expect(decoded.my_int_field == 42)
    #expect(decoded.my_enum == .MyValue2)
  }

  @Test func defaults() {
    let s = MyStruct()
    #expect(s.my_int_field == 0)
    #expect(s.my_string_field == "")
    #expect(s.my_enum == .MyValue1)
    #expect(!s.oneway)
  }

  @Test func unknownEnumValueRoundTrips() throws {
    // Guide 1.2.13: an i32 with no declared case must be preserved, not rejected.
    var s = MyStruct()
    s.my_enum = .unknown(99)
    let data = CompactSerializer.serialize(s)
    let decoded = try CompactSerializer.deserialize(MyStruct.self, from: data)
    #expect(decoded.my_enum == .unknown(99))
    #expect(decoded.my_enum.rawValue == 99)
  }

  @Test func containersRoundTrip() throws {
    var c = Containers()
    c.i32_list = [1, 2, 3]
    c.string_set = ["a", "b"]
    c.string_to_i64_map = ["x": 10, "y": 20]
    let data = CompactSerializer.serialize(c)
    #expect(try CompactSerializer.deserialize(Containers.self, from: data) == c)
  }

  @Test func typedefContainersRoundTrip() throws {
    var c = TypedefContainerStruct()
    c.int_list_field = [4, 5, 6]
    c.string_set_field = ["p", "q"]
    c.string_map_field = ["k": 7]
    let data = CompactSerializer.serialize(c)
    #expect(
      try CompactSerializer.deserialize(
        TypedefContainerStruct.self, from: data) == c)
  }

  @Test func optionalFieldsUnsetOmitted() throws {
    let s = OptionalFieldsStruct()
    let data = CompactSerializer.serialize(s)
    let decoded = try CompactSerializer.deserialize(
      OptionalFieldsStruct.self, from: data)
    #expect(decoded.optional_string == nil)
    #expect(decoded.optional_struct == nil)
    #expect(decoded.optional_list == nil)
  }

  @Test func optionalFieldsSetRoundTrip() throws {
    var s = OptionalFieldsStruct()
    s.optional_string = "present"
    s.optional_list = [9]
    let data = CompactSerializer.serialize(s)
    let decoded = try CompactSerializer.deserialize(
      OptionalFieldsStruct.self, from: data)
    #expect(decoded.optional_string == "present")
    #expect(decoded.optional_list == [9])
    #expect(decoded.optional_struct == nil)
  }

  @Test func hashableAsSetElementAndDictionaryKey() {
    // Guide 2.4.26: structs are Hashable, so they work as Set elements and
    // dictionary keys.
    var a = MyStruct()
    a.my_int_field = 1
    a.my_string_field = "a"
    var b = MyStruct()
    b.my_int_field = 2
    b.my_string_field = "b"
    let aCopy = a

    let set: Set<MyStruct> = [a, b, aCopy]
    #expect(set.count == 2)
    #expect(set.contains(a))
    #expect(set.contains(b))

    var counts: [MyStruct: Int] = [:]
    counts[a, default: 0] += 1
    counts[aCopy, default: 0] += 1
    counts[b, default: 0] += 1
    #expect(counts[a] == 2)
    #expect(counts[b] == 1)
  }

  @Test func exceptionRoundTrip() throws {
    var e = MyException()
    e.my_int_field = -1
    e.my_string_field = "boom"
    let data = CompactSerializer.serialize(e)
    #expect(
      try CompactSerializer.deserialize(MyException.self, from: data) == e)
    // Also usable as a Swift Error.
    let asError: Error = e
    #expect(asError is MyException)
  }

  @Test func clearIgnoresCustomDefaults() {
    // init() honors the IDL defaults (emitted as Swift property initializers).
    var s = StructWithDefaults()
    #expect(s.int_with_default == 42)
    #expect(s.string_with_default == "hello")
    #expect(s.list_with_default == [1, 2, 3])
    // clear() resets every field to its type's intrinsic zero value, ignoring
    // the IDL defaults (guide 2.1.22).
    s.int_with_default = 7
    s.clear()
    #expect(s.int_with_default == 0)
    #expect(s.string_with_default == "")
    #expect(s.list_with_default == [])
  }

  @Test func clearResetsOptionalsToNil() {
    var s = OptionalFieldsStruct()
    s.optional_string = "present"
    s.optional_list = [1, 2]
    s.clear()
    #expect(s.optional_string == nil)
    #expect(s.optional_list == nil)
    #expect(s.optional_struct == nil)
  }
}
