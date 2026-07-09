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
import Foundation
import Testing

/// A minimal hand-written ThriftSerializable used to exercise the binary
/// runtime without depending on generated code (mirrors what the generator
/// emits, and matches the type used in the Compact tests).
private struct Point: ThriftSerializable, Equatable {
  var x: Int32 = 0
  var y: Int32 = 0
  var label: String = ""

  init() {}

  func write<W: ProtocolWriter>(to writer: W) {
    writer.writeFieldBegin(.i32, 1)
    x.thriftWrite(to: writer)
    writer.writeFieldBegin(.i32, 2)
    y.thriftWrite(to: writer)
    writer.writeFieldBegin(.string, 3)
    label.thriftWrite(to: writer)
    writer.writeFieldStop()
  }

  init<R: ProtocolReader>(from reader: R) throws {
    self.init()
    while true {
      let (fieldType, fieldId) = try reader.readFieldBegin()
      if fieldType == .stop { break }
      switch fieldId {
      case 1:
        if fieldType == .i32 {
          x = try Int32.thriftRead(from: reader)
        } else {
          try reader.skip(fieldType)
        }
      case 2:
        if fieldType == .i32 {
          y = try Int32.thriftRead(from: reader)
        } else {
          try reader.skip(fieldType)
        }
      case 3:
        if fieldType == .string {
          label = try String.thriftRead(from: reader)
        } else {
          try reader.skip(fieldType)
        }
      default:
        try reader.skip(fieldType)
      }
    }
  }
}

private struct AllTheThings: ThriftSerializable, Equatable {
  var flag: Bool = false
  var byteField: Int8 = 0
  var i16Field: Int16 = 0
  var i64Field: Int64 = 0
  var floatField: Float = 0
  var doubleField: Double = 0
  var blob: Data = Data()
  var ints: [Int32] = []
  var names: Set<String> = []
  var lookup: [String: Int64] = [:]
  var point: Point = .init()

  init() {}

  func write<W: ProtocolWriter>(to writer: W) {
    writer.writeFieldBegin(.bool, 1)
    flag.thriftWrite(to: writer)
    writer.writeFieldBegin(.byte, 2)
    byteField.thriftWrite(to: writer)
    writer.writeFieldBegin(.i16, 3)
    i16Field.thriftWrite(to: writer)
    writer.writeFieldBegin(.i64, 4)
    i64Field.thriftWrite(to: writer)
    writer.writeFieldBegin(.float, 5)
    floatField.thriftWrite(to: writer)
    writer.writeFieldBegin(.double, 6)
    doubleField.thriftWrite(to: writer)
    writer.writeFieldBegin(.string, 7)
    blob.thriftWrite(to: writer)
    writer.writeFieldBegin(.list, 8)
    ints.thriftWrite(to: writer)
    writer.writeFieldBegin(.set, 9)
    names.thriftWrite(to: writer)
    writer.writeFieldBegin(.map, 10)
    lookup.thriftWrite(to: writer)
    writer.writeFieldBegin(.struct, 11)
    point.thriftWrite(to: writer)
    writer.writeFieldStop()
  }

  init<R: ProtocolReader>(from reader: R) throws {
    self.init()
    while true {
      let (fieldType, fieldId) = try reader.readFieldBegin()
      if fieldType == .stop { break }
      switch fieldId {
      case 1: flag = try Bool.thriftRead(from: reader)
      case 2: byteField = try Int8.thriftRead(from: reader)
      case 3: i16Field = try Int16.thriftRead(from: reader)
      case 4: i64Field = try Int64.thriftRead(from: reader)
      case 5: floatField = try Float.thriftRead(from: reader)
      case 6: doubleField = try Double.thriftRead(from: reader)
      case 7: blob = try Data.thriftRead(from: reader)
      case 8: ints = try [Int32].thriftRead(from: reader)
      case 9: names = try Set<String>.thriftRead(from: reader)
      case 10: lookup = try [String: Int64].thriftRead(from: reader)
      case 11: point = try Point.thriftRead(from: reader)
      default: try reader.skip(fieldType)
      }
    }
  }
}

/// A self-referential struct used to exercise the read-path depth guard. Modeled
/// as a class because a value type cannot recursively contain itself. Field 1 is
/// an optional child of the same type.
private final class Nested: ThriftSerializable {
  var child: Nested?

  init() {}

  func write<W: ProtocolWriter>(to writer: W) {
    if let child {
      writer.writeFieldBegin(.struct, 1)
      child.thriftWrite(to: writer)
    }
    writer.writeFieldStop()
  }

  init<R: ProtocolReader>(from reader: R) throws {
    while true {
      let (fieldType, fieldId) = try reader.readFieldBegin()
      if fieldType == .stop { break }
      if fieldId == 1, fieldType == .struct {
        child = try Nested.thriftRead(from: reader)
      } else {
        try reader.skip(fieldType)
      }
    }
  }
}

struct ThriftBinaryProtocolTests {
  private func roundTrip<T: ThriftSerializable & Equatable>(
    _ value: T
  ) throws
    -> T
  {
    let data = BinarySerializer.serialize(value)
    return try BinarySerializer.deserialize(T.self, from: data)
  }

  @Test func structRoundTrip() throws {
    var p = Point()
    p.x = 42
    p.y = -7
    p.label = "hello"
    #expect(try roundTrip(p) == p)
  }

  @Test func negativeAndLargeIntegers() throws {
    var p = Point()
    p.x = Int32.min
    p.y = Int32.max
    #expect(try roundTrip(p) == p)
  }

  @Test func allScalarsAndContainers() throws {
    var a = AllTheThings()
    a.flag = true
    a.byteField = -3
    a.i16Field = 12345
    a.i64Field = Int64.min
    a.floatField = 3.5
    a.doubleField = 2.718281828459
    a.blob = Data([0, 1, 2, 255])
    a.ints = [1, -1, 1000000]
    a.names = ["a", "b", "c"]
    a.lookup = ["x": 10, "y": 20]
    a.point = {
      var p = Point()
      p.x = 9
      return p
    }()
    #expect(try roundTrip(a) == a)
  }

  @Test func emptyContainers() throws {
    let a = AllTheThings()
    #expect(try roundTrip(a) == a)
  }

  @Test func unknownFieldIsSkipped() {
    // Encode AllTheThings, decode as Point: Point only knows fields 1-3,
    // everything else must be skipped without error.
    var a = AllTheThings()
    a.i64Field = 123
    a.names = ["skip", "me"]
    let data = BinarySerializer.serialize(a)
    #expect(throws: Never.self) {
      try BinarySerializer.deserialize(Point.self, from: data)
    }
  }

  // MARK: - Malformed input

  @Test func truncatedBufferThrows() {
    var p = Point()
    p.label = "abc"
    var data = BinarySerializer.serialize(p)
    data.removeLast()
    #expect(throws: (any Error).self) {
      try BinarySerializer.deserialize(Point.self, from: data)
    }
  }

  @Test func unknownWireTypeThrows() {
    // A field header carrying a wire type outside the known set must be
    // rejected rather than silently mishandled.
    let reader = BinaryReader(data: Data([0x7F, 0x00, 0x01]))
    #expect(throws: (any Error).self) {
      try reader.readFieldBegin()
    }
  }

  @Test func negativeCollectionSizeThrows() {
    // Binary encodes collection counts as a signed 4-byte big-endian i32, so a
    // negative count is malformed and must be rejected. Header: i32 element
    // type (0x08) followed by a count of 0xFFFFFFFF (-1).
    let reader = BinaryReader(data: Data([0x08, 0xFF, 0xFF, 0xFF, 0xFF]))
    #expect(throws: (any Error).self) {
      try reader.readListBegin()
    }
  }

  @Test func collectionSizeExceedingBufferThrows() {
    // A list header claiming far more elements than the buffer can hold must
    // throw rather than attempt a huge allocation. Header: i32 element type
    // (0x08) then a 4-byte big-endian count of 0x7FFFFFFF, with no elements.
    let reader = BinaryReader(data: Data([0x08, 0x7F, 0xFF, 0xFF, 0xFF]))
    #expect(throws: (any Error).self) {
      try reader.readListBegin()
    }
  }

  @Test func listCountExceedsPerElementCapacityThrows() {
    // The header claims 5 i64 elements (8 bytes each) but only 5 payload bytes
    // follow. The per-element minimum (8) must reject the header up front, even
    // though 5 single-byte elements would nominally fit in the remaining bytes.
    // Header: i64 element type (0x0A) + count 5, then 5 stray bytes.
    let reader = BinaryReader(
      data: Data([0x0A, 0x00, 0x00, 0x00, 0x05, 0, 0, 0, 0, 0]))
    #expect(throws: (any Error).self) {
      try reader.readListBegin()
    }
  }

  @Test func mapPairCountExceedsPerElementCapacityThrows() {
    // The header claims 3 i32->i64 pairs (12 bytes each) but only 6 payload
    // bytes follow. The per-pair minimum (4 + 8) must reject the header up
    // front. Header: i32 key (0x08) + i64 value (0x0A) + count 3, then 6 bytes.
    let reader = BinaryReader(
      data: Data([0x08, 0x0A, 0x00, 0x00, 0x00, 0x03, 0, 0, 0, 0, 0, 0]))
    #expect(throws: (any Error).self) {
      try reader.readMapBegin()
    }
  }

  @Test func unknownListElementTypeThrows() {
    // A list header whose element type is outside the known wire-type set must
    // be rejected. Header: element type 0x7F (unknown) + count 0.
    let reader = BinaryReader(data: Data([0x7F, 0x00, 0x00, 0x00, 0x00]))
    #expect(throws: (any Error).self) {
      try reader.readListBegin()
    }
  }

  @Test func unknownMapKeyOrValueTypeThrows() {
    // A map header whose key or value type is unknown must be rejected before
    // the count is trusted. Header: key type 0x7F (unknown) + value type i64
    // (0x0A) + count 0.
    let reader = BinaryReader(data: Data([0x7F, 0x0A, 0x00, 0x00, 0x00, 0x00]))
    #expect(throws: (any Error).self) {
      try reader.readMapBegin()
    }
  }

  @Test func wellFormedCollectionHeaderAtCapacitySucceeds() throws {
    // Guard against over-tightening: a list of two i32s with exactly eight
    // payload bytes available must pass header validation.
    let reader = BinaryReader(
      data: Data([0x08, 0x00, 0x00, 0x00, 0x02, 0, 0, 0, 0, 0, 0, 0, 0]))
    let (elemType, size) = try reader.readListBegin()
    #expect(elemType == .i32)
    #expect(size == 2)
  }

  @Test func deeplyNestedStructSkipThrows() {
    // Skipping a struct nested past the reader's depth limit must throw
    // depthLimitExceeded rather than recurse without bound. Each 3-byte group
    // (struct wire type + big-endian field id 1) opens another nested struct.
    let level: [UInt8] = [WireType.struct.rawValue, 0x00, 0x01]
    let nested = Array(
      repeating: level, count: BinaryReader.defaultMaxDepth + 4
    ).flatMap { $0 }
    let reader = BinaryReader(data: Data(nested))
    do {
      try reader.skip(.struct)
      Issue.record("Expected skip to throw depthLimitExceeded")
    } catch ProtocolError.depthLimitExceeded {
      // expected
    } catch {
      Issue.record("Expected depthLimitExceeded, got \(error)")
    }
  }

  @Test func deeplyNestedStructReadThrows() {
    // Reading (not just skipping) a struct nested past the depth limit must
    // throw depthLimitExceeded instead of recursing until the stack overflows.
    // Each 3-byte group (struct wire type + big-endian field id 1) opens
    // another nested struct.
    let level: [UInt8] = [WireType.struct.rawValue, 0x00, 0x01]
    let nested = Array(
      repeating: level, count: BinaryReader.defaultMaxDepth + 4
    ).flatMap { $0 }
    let reader = BinaryReader(data: Data(nested))
    do {
      let _: Nested = try reader.readStruct()
      Issue.record("Expected read to throw depthLimitExceeded")
    } catch ProtocolError.depthLimitExceeded {
      // expected
    } catch {
      Issue.record("Expected depthLimitExceeded, got \(error)")
    }
  }

  @Test func readBoolAcceptsCanonicalEncodings() throws {
    #expect(try BinaryReader(data: Data([0x00])).readBool() == false)
    #expect(try BinaryReader(data: Data([0x01])).readBool() == true)
  }

  @Test func readBoolRejectsOutOfRangeValue() {
    // Matches the C++ BinaryProtocol reader: any byte >= 2 is not a valid bool
    // encoding and must be rejected rather than coerced to true.
    let reader = BinaryReader(data: Data([0x02]))
    do {
      _ = try reader.readBool()
      Issue.record("Expected readBool to throw invalidData")
    } catch ProtocolError.invalidData {
      // expected
    } catch {
      Issue.record("Expected invalidData, got \(error)")
    }
  }

  @Test func binaryFieldHeaderLayout() {
    // Binary field header is type(1 byte) + field id(2 bytes, big-endian);
    // WireType raw values are the on-wire binary TType ids.
    var p = Point()
    p.x = 1
    let bytes = [UInt8](BinarySerializer.serialize(p))
    // First field: i32 (raw 8), id 1 -> [0x08, 0x00, 0x01, 0x00,0x00,0x00,0x01]
    #expect(bytes[0] == WireType.i32.rawValue)
    #expect(bytes[1] == 0x00)
    #expect(bytes[2] == 0x01)
    #expect(Array(bytes[3..<7]) == [0x00, 0x00, 0x00, 0x01])
  }

  @Test func distinctFromCompactButSameValue() throws {
    // Binary and Compact are different encodings of the same value; both must
    // round-trip, and (for a non-trivial value) produce different bytes.
    var a = AllTheThings()
    a.i64Field = 1_000_000
    a.names = ["a", "b"]
    let binary = BinarySerializer.serialize(a)
    let compact = CompactSerializer.serialize(a)
    #expect(binary != compact)
    #expect(try BinarySerializer.deserialize(AllTheThings.self, from: binary) == a)
    #expect(try CompactSerializer.deserialize(AllTheThings.self, from: compact) == a)
  }
}
