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

import Foundation

/// A value that can be written to a `ProtocolWriter` as an element (of a
/// field, list, set, or map). Conformed by every primitive, generated enum,
/// generated struct/union, and (recursively) by the collection types.
public protocol ThriftWritable {
  /// The neutral wire type used in field/collection headers for this type.
  static var thriftWireType: WireType { get }
  /// Writes `self` as a bare value (no field header).
  func thriftWrite<W: ProtocolWriter>(to writer: W)
}

/// A value that can be read from a `ProtocolReader` as an element. The
/// read-side mirror of `ThriftWritable`.
public protocol ThriftReadable {
  /// The neutral wire type used in field/collection headers for this type.
  static var thriftWireType: WireType { get }
  /// Reads a bare value (no field header) of this type.
  static func thriftRead<R: ProtocolReader>(
    from reader: R
  ) throws
    -> Self
}

/// A Thrift struct, union, or exception that can be serialized and deserialized
/// using any Thrift protocol. Generated types conform to this.
public protocol ThriftSerializable: ThriftWritable, ThriftReadable {
  /// Writes this object's fields, terminated by a field-stop marker.
  func write<W: ProtocolWriter>(to writer: W)
  /// Reads this object's fields from the wire.
  init<R: ProtocolReader>(from reader: R) throws
}

extension ThriftSerializable {
  public static var thriftWireType: WireType { .struct }

  public func thriftWrite<W: ProtocolWriter>(to writer: W) {
    writer.writeStruct(self)
  }

  public static func thriftRead<R: ProtocolReader>(
    from reader: R
  )
    throws -> Self
  {
    try reader.readStruct()
  }
}

/// A generated Thrift enum, backed by an `Int32` wire value.
///
/// Unknown-value preservation (Object Model / bootstrap guide 1.2.13): a value
/// deserialized with an i32 that does not correspond to a declared case is
/// retained as `.unknown(rawValue)` rather than rejected, so it round-trips.
/// `init(rawValue:)` is therefore total (never fails).
public protocol ThriftEnum: ThriftWritable, ThriftReadable, Hashable {
  init(rawValue: Int32)
  var rawValue: Int32 { get }
}

extension ThriftEnum {
  public static var thriftWireType: WireType { .i32 }

  public func thriftWrite<W: ProtocolWriter>(to writer: W) {
    writer.writeI32(self.rawValue)
  }

  public static func thriftRead<R: ProtocolReader>(
    from reader: R
  )
    throws -> Self
  {
    Self(rawValue: try reader.readI32())
  }
}

// MARK: - Primitive conformances

extension Bool: ThriftWritable, ThriftReadable {
  public static var thriftWireType: WireType { .bool }
  public func thriftWrite<W: ProtocolWriter>(to writer: W) {
    writer.writeBool(self)
  }
  public static func thriftRead<R: ProtocolReader>(
    from reader: R
  )
    throws -> Bool
  { try reader.readBool() }
}

extension Int8: ThriftWritable, ThriftReadable {
  public static var thriftWireType: WireType { .byte }
  public func thriftWrite<W: ProtocolWriter>(to writer: W) {
    writer.writeByte(self)
  }
  public static func thriftRead<R: ProtocolReader>(
    from reader: R
  )
    throws -> Int8
  { try reader.readByte() }
}

extension Int16: ThriftWritable, ThriftReadable {
  public static var thriftWireType: WireType { .i16 }
  public func thriftWrite<W: ProtocolWriter>(to writer: W) {
    writer.writeI16(self)
  }
  public static func thriftRead<R: ProtocolReader>(
    from reader: R
  )
    throws -> Int16
  { try reader.readI16() }
}

extension Int32: ThriftWritable, ThriftReadable {
  public static var thriftWireType: WireType { .i32 }
  public func thriftWrite<W: ProtocolWriter>(to writer: W) {
    writer.writeI32(self)
  }
  public static func thriftRead<R: ProtocolReader>(
    from reader: R
  )
    throws -> Int32
  { try reader.readI32() }
}

extension Int64: ThriftWritable, ThriftReadable {
  public static var thriftWireType: WireType { .i64 }
  public func thriftWrite<W: ProtocolWriter>(to writer: W) {
    writer.writeI64(self)
  }
  public static func thriftRead<R: ProtocolReader>(
    from reader: R
  )
    throws -> Int64
  { try reader.readI64() }
}

extension Float: ThriftWritable, ThriftReadable {
  public static var thriftWireType: WireType { .float }
  public func thriftWrite<W: ProtocolWriter>(to writer: W) {
    writer.writeFloat(self)
  }
  public static func thriftRead<R: ProtocolReader>(
    from reader: R
  )
    throws -> Float
  { try reader.readFloat() }
}

extension Double: ThriftWritable, ThriftReadable {
  public static var thriftWireType: WireType { .double }
  public func thriftWrite<W: ProtocolWriter>(to writer: W) {
    writer.writeDouble(self)
  }
  public static func thriftRead<R: ProtocolReader>(
    from reader: R
  )
    throws -> Double
  { try reader.readDouble() }
}

extension String: ThriftWritable, ThriftReadable {
  public static var thriftWireType: WireType { .string }
  public func thriftWrite<W: ProtocolWriter>(to writer: W) {
    writer.writeString(self)
  }
  public static func thriftRead<R: ProtocolReader>(
    from reader: R
  )
    throws -> String
  { try reader.readString() }
}

extension Data: ThriftWritable, ThriftReadable {
  public static var thriftWireType: WireType { .string }
  public func thriftWrite<W: ProtocolWriter>(to writer: W) {
    writer.writeBinary(self)
  }
  public static func thriftRead<R: ProtocolReader>(
    from reader: R
  )
    throws -> Data
  { try reader.readBinary() }
}

// MARK: - Container conformances

extension Array: ThriftWritable where Element: ThriftWritable {
  public static var thriftWireType: WireType { .list }
  public func thriftWrite<W: ProtocolWriter>(to writer: W) {
    writer.writeList(self)
  }
}

extension Array: ThriftReadable where Element: ThriftReadable {
  public static var thriftWireType: WireType { .list }
  public static func thriftRead<R: ProtocolReader>(
    from reader: R
  )
    throws -> [Element]
  { try reader.readList() }
}

extension Set: ThriftWritable where Element: ThriftWritable {
  public static var thriftWireType: WireType { .set }
  public func thriftWrite<W: ProtocolWriter>(to writer: W) {
    writer.writeSet(self)
  }
}

extension Set: ThriftReadable where Element: ThriftReadable {
  public static var thriftWireType: WireType { .set }
  public static func thriftRead<R: ProtocolReader>(
    from reader: R
  )
    throws -> Set<Element>
  { try reader.readSet() }
}

extension Dictionary: ThriftWritable
where Key: ThriftWritable, Value: ThriftWritable {
  public static var thriftWireType: WireType { .map }
  public func thriftWrite<W: ProtocolWriter>(to writer: W) {
    writer.writeMap(self)
  }
}

extension Dictionary: ThriftReadable
where Key: ThriftReadable, Value: ThriftReadable {
  public static var thriftWireType: WireType { .map }
  public static func thriftRead<R: ProtocolReader>(
    from reader: R
  )
    throws -> [Key: Value]
  { try reader.readMap() }
}
