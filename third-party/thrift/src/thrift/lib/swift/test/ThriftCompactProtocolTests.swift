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
    // be rejected, not silently truncated.
    let reader = CompactReader(data: Data([0xFF, 0xFF, 0xFF, 0xFF, 0xFF]))
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
}
