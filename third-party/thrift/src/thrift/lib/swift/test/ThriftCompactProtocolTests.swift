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

/// A minimal hand-written ThriftSerializable used to exercise the runtime
/// without depending on generated code (mirrors what the generator emits).
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
        if fieldType == .i32 { x = try Int32.thriftRead(from: reader) } else { try reader.skip(fieldType) }
      case 2:
        if fieldType == .i32 { y = try Int32.thriftRead(from: reader) } else { try reader.skip(fieldType) }
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
      // Validate the wire type of each known field and skip on mismatch, so a
      // payload with an unexpected type for a known id is skipped rather than
      // misread (mirrors Point.init(from:)).
      switch fieldId {
      case 1:
        if fieldType == .bool { flag = try Bool.thriftRead(from: reader) } else { try reader.skip(fieldType) }
      case 2:
        if fieldType == .byte { byteField = try Int8.thriftRead(from: reader) } else { try reader.skip(fieldType) }
      case 3:
        if fieldType == .i16 { i16Field = try Int16.thriftRead(from: reader) } else { try reader.skip(fieldType) }
      case 4:
        if fieldType == .i64 { i64Field = try Int64.thriftRead(from: reader) } else { try reader.skip(fieldType) }
      case 5:
        if fieldType == .float { floatField = try Float.thriftRead(from: reader) } else { try reader.skip(fieldType) }
      case 6:
        if fieldType == .double { doubleField = try Double.thriftRead(from: reader) } else { try reader.skip(fieldType) }
      case 7:
        if fieldType == .string { blob = try Data.thriftRead(from: reader) } else { try reader.skip(fieldType) }
      case 8:
        if fieldType == .list { ints = try [Int32].thriftRead(from: reader) } else { try reader.skip(fieldType) }
      case 9:
        if fieldType == .set { names = try Set<String>.thriftRead(from: reader) } else { try reader.skip(fieldType) }
      case 10:
        if fieldType == .map { lookup = try [String: Int64].thriftRead(from: reader) } else { try reader.skip(fieldType) }
      case 11:
        if fieldType == .struct { point = try Point.thriftRead(from: reader) } else { try reader.skip(fieldType) }
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

struct ThriftCompactProtocolTests {
  private func roundTrip<T: ThriftSerializable & Equatable>(
    _ value: T
  ) throws
    -> T
  {
    let data = CompactSerializer.serialize(value)
    return try CompactSerializer.deserialize(T.self, from: data)
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

  // MARK: - Malformed input

  @Test func unknownFieldIsSkipped() {
    // Encode AllTheThings, decode as Point: Point only knows fields 1-3,
    // everything else must be skipped without error.
    var a = AllTheThings()
    a.i64Field = 123
    a.names = ["skip", "me"]
    let data = CompactSerializer.serialize(a)
    #expect(throws: Never.self) {
      try CompactSerializer.deserialize(Point.self, from: data)
    }
  }

  @Test func truncatedBufferThrows() {
    var p = Point()
    p.label = "abc"
    var data = CompactSerializer.serialize(p)
    data.removeLast()
    #expect(throws: (any Error).self) {
      try CompactSerializer.deserialize(Point.self, from: data)
    }
  }

  @Test func varintOverflowThrows() {
    // A varint32 whose 5th byte carries value bits beyond the 32-bit range must
    // be rejected, not silently truncated. The 5th byte clears the continuation
    // bit (so no 6th byte is implied) but sets bits above bit 31 (0x10), which
    // must trip the overflow check specifically, not a truncation check.
    let reader = CompactReader(data: Data([0xFF, 0xFF, 0xFF, 0xFF, 0x10]))
    #expect(throws: (any Error).self) {
      try reader.readI32()
    }
  }

  @Test func collectionSizeExceedingBufferThrows() {
    // A list header claiming far more elements than the buffer can hold must
    // throw rather than attempt a huge allocation. Header 0xF5 = (size marker
    // 0xF, i32 element type 0x5); the following varint decodes to a large size
    // with no element bytes after it.
    let reader = CompactReader(
      data: Data([0xF5, 0xFF, 0xFF, 0xFF, 0xFF, 0x0F]))
    #expect(throws: (any Error).self) {
      try reader.readListBegin()
    }
  }

  @Test func deeplyNestedStructSkipThrows() {
    // Skipping a struct nested past the reader's depth limit must throw
    // depthLimitExceeded rather than recurse without bound. Each 0x1C byte is a
    // field header (id delta 1, compact type struct), so a run of them is an
    // unbounded chain of nested structs.
    let nested = [UInt8](
      repeating: 0x1C, count: CompactReader.defaultMaxDepth + 4)
    let reader = CompactReader(data: Data(nested))
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
    // Each 0x1C byte is a field header (id delta 1, compact type struct), so a
    // run of them is an unbounded chain of nested structs.
    let nested = [UInt8](
      repeating: 0x1C, count: CompactReader.defaultMaxDepth + 4)
    let reader = CompactReader(data: Data(nested))
    do {
      let _: Nested = try reader.readStruct()
      Issue.record("Expected read to throw depthLimitExceeded")
    } catch ProtocolError.depthLimitExceeded {
      // expected
    } catch {
      Issue.record("Expected depthLimitExceeded, got \(error)")
    }
  }

  @Test func readBoolRejectsOutOfRangeValue() {
    // Compact encodes a standalone bool as CT_BOOLEAN_TRUE (1) or
    // CT_BOOLEAN_FALSE (2); any other byte is malformed and must be rejected.
    let reader = CompactReader(data: Data([0x03]))
    do {
      _ = try reader.readBool()
      Issue.record("Expected readBool to throw invalidData")
    } catch ProtocolError.invalidData {
      // expected
    } catch {
      Issue.record("Expected invalidData, got \(error)")
    }
  }

  @Test func nestedStructWithinDepthLimitRoundTrips() throws {
    // Symmetry check for the writer/reader depth counters: a chain safely under
    // the limit must serialize and deserialize without tripping either guard,
    // and the defer-based decrement must keep the counters balanced.
    func makeChain(depth: Int) -> Nested {
      let root = Nested()
      var tip = root
      for _ in 1..<depth {
        let next = Nested()
        tip.child = next
        tip = next
      }
      return root
    }
    func chainDepth(_ node: Nested) -> Int {
      var count = 1
      var cur = node
      while let next = cur.child {
        count += 1
        cur = next
      }
      return count
    }
    let data = CompactSerializer.serialize(makeChain(depth: 8))
    let decoded = try CompactSerializer.deserialize(Nested.self, from: data)
    #expect(chainDepth(decoded) == 8)
  }

  @Test func varint64TooLongThrows() {
    // A varint64 that never clears its continuation bit must be rejected once it
    // runs past the 64-bit limit rather than looping unbounded. Ten 0x80 bytes
    // keep signalling "more to come" past what a 64-bit value can hold.
    let reader = CompactReader(data: Data(repeating: 0x80, count: 10))
    #expect(throws: (any Error).self) {
      try reader.readI64()
    }
  }

  @Test func nestedStructFieldIdStack() throws {
    // Skipping a nested struct must save and restore the field-id delta state so
    // the field after the struct decodes relative to the outer struct's last
    // field id, not the inner struct's. Without a restore, field 3 below would
    // be misread as id 2. Encodes: field 1 (i32=100), field 2 (struct { field 1
    // (i32=200) }), field 3 (i32=300), stop.
    let bytes: [UInt8] = [
      0x15,  // field 1: delta 1, i32 (compact 5)
      0xC8, 0x01,  // zigzag(100) = 200
      0x1C,  // field 2: delta 1, struct (compact 12)
      0x15,  // inner field 1: delta 1 from reset 0, i32
      0x90, 0x03,  // zigzag(200) = 400
      0x00,  // inner stop
      0x15,  // field 3: delta 1 from restored 2, i32
      0xD8, 0x04,  // zigzag(300) = 600
      0x00,  // outer stop
    ]
    let reader = CompactReader(bytes: bytes)

    let (t1, id1) = try reader.readFieldBegin()
    #expect(t1 == .i32)
    #expect(id1 == 1)
    #expect(try reader.readI32() == 100)

    let (t2, id2) = try reader.readFieldBegin()
    #expect(t2 == .struct)
    #expect(id2 == 2)

    // Skipping the inner struct must push/pop the field-id state.
    try reader.skip(.struct)

    // Field 3 must be id 3 (delta 1 from restored outer field 2), not id 2.
    let (t3, id3) = try reader.readFieldBegin()
    #expect(t3 == .i32)
    #expect(id3 == 3)
    #expect(try reader.readI32() == 300)

    let (stop, _) = try reader.readFieldBegin()
    #expect(stop == .stop)
  }
}
