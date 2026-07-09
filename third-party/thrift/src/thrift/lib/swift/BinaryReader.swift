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

/// Reads Thrift data types using the binary protocol format.
///
/// The mirror of `BinaryWriter`: fixed-width big-endian integers/floats,
/// 3-byte field headers, and 4-byte big-endian collection counts. Reads from an
/// in-memory buffer.
public final class BinaryReader: ProtocolReader {
  /// Default maximum nesting depth for skip and struct-read operations. Matches
  /// Rust's DEFAULT_RECURSION_DEPTH.
  public static let defaultMaxDepth = 64

  private let buffer: [UInt8]
  private var position: Int = 0
  private var currentFieldId: Int16?

  /// Current struct-read nesting depth. Bounded by `defaultMaxDepth` so a
  /// maliciously deep chain of nested structs cannot overflow the stack while
  /// being deserialized (the read-side mirror of the `skip` depth guard).
  private var depth = 0

  /// Maximum allowed size for string and binary reads. When 0 (default), no
  /// configurable limit is enforced, but reads are still validated against
  /// remaining buffer bytes.
  public var stringSizeLimit: Int = 0

  /// Maximum allowed element count for a single list/set/map read. When 0
  /// (default), no configurable limit is enforced, but sizes are still
  /// validated against remaining buffer bytes.
  public var collectionSizeLimit: Int = 0

  public init(bytes: [UInt8]) {
    self.buffer = bytes
  }

  public convenience init(data: Data) {
    self.init(bytes: [UInt8](data))
  }

  private var remainingBytes: Int { buffer.count - position }

  /// True when all input bytes have been consumed. Used to detect trailing
  /// bytes after a top-level value.
  public var isAtEnd: Bool { position >= buffer.count }

  private var errorContext: String {
    currentFieldId.map { " (field id=\($0))" } ?? ""
  }

  public func readFieldBegin() throws
    -> (fieldType: WireType, fieldId: Int16)
  {
    let typeByte = try readRawByte()
    if typeByte == WireType.stop.rawValue {
      currentFieldId = nil
      return (.stop, 0)
    }
    // Clear before the sub-reads so an error thrown here (unknown wire type,
    // truncated field id) is not misattributed to the previous field.
    currentFieldId = nil
    let fieldType = try wireType(typeByte)
    let fieldId = try readI16()
    currentFieldId = fieldId
    return (fieldType, fieldId)
  }

  public func readBool() throws -> Bool {
    // Reject out-of-range encodings (matches the C++ BinaryProtocol reader):
    // a valid bool is exactly 0 or 1 on the wire.
    let byte = try readRawByte()
    switch byte {
    case 0: return false
    case 1: return true
    default:
      throw ProtocolError.invalidData(
        "Invalid bool value \(byte)\(errorContext)")
    }
  }

  public func readByte() throws -> Int8 {
    Int8(bitPattern: try readRawByte())
  }

  public func readI16() throws -> Int16 {
    Int16(bitPattern: try readBigEndian16())
  }

  public func readI32() throws -> Int32 {
    Int32(bitPattern: try readBigEndian32())
  }

  public func readI64() throws -> Int64 {
    Int64(bitPattern: try readBigEndian64())
  }

  public func readFloat() throws -> Float {
    Float(bitPattern: try readBigEndian32())
  }

  public func readDouble() throws -> Double {
    Double(bitPattern: try readBigEndian64())
  }

  public func readString() throws -> String {
    let bytes = try readLengthPrefixedBytes()
    guard let value = String(bytes: bytes, encoding: .utf8) else {
      throw ProtocolError.invalidData(
        "Invalid UTF-8 string\(errorContext)")
    }
    return value
  }

  public func readBinary() throws -> Data {
    Data(try readLengthPrefixedBytes())
  }

  public func readListBegin() throws
    -> (elemType: WireType, size: Int)
  {
    let elemType = try wireType(try readRawByte())
    let size = try readCount()
    try validateCollectionSize(
      size, minBytesPerElement: minEncodedBytes(elemType))
    return (elemType, size)
  }

  public func readSetBegin() throws
    -> (elemType: WireType, size: Int)
  {
    try readListBegin()
  }

  public func readMapBegin() throws
    -> (keyType: WireType, valType: WireType, size: Int)
  {
    let keyType = try wireType(try readRawByte())
    let valType = try wireType(try readRawByte())
    let size = try readCount()
    try validateCollectionSize(
      size,
      minBytesPerElement: minEncodedBytes(keyType) + minEncodedBytes(valType))
    return (keyType, valType, size)
  }

  public func readStruct<T: ThriftSerializable>() throws -> T {
    depth += 1
    defer { depth -= 1 }
    if depth > BinaryReader.defaultMaxDepth {
      throw ProtocolError.depthLimitExceeded(
        "Maximum nesting depth (\(BinaryReader.defaultMaxDepth)) "
          + "exceeded while reading struct")
    }
    return try T(from: self)
  }

  public func skip(_ fieldType: WireType) throws {
    try skip(fieldType, remainingDepth: BinaryReader.defaultMaxDepth)
  }

  private func skip(_ fieldType: WireType, remainingDepth: Int) throws {
    if remainingDepth <= 0 {
      throw ProtocolError.depthLimitExceeded(
        "Maximum nesting depth (\(BinaryReader.defaultMaxDepth)) "
          + "exceeded while skipping type=\(fieldType)")
    }
    switch fieldType {
    case .bool: _ = try readBool()
    case .byte: _ = try readByte()
    case .i16: _ = try readI16()
    case .i32: _ = try readI32()
    case .i64: _ = try readI64()
    case .float: _ = try readFloat()
    case .double: _ = try readDouble()
    case .string:
      // Skip as binary to avoid UTF-8 validation on unknown fields.
      _ = try readBinary()
    case .struct:
      while true {
        let (ft, _) = try readFieldBegin()
        if ft == .stop { break }
        try skip(ft, remainingDepth: remainingDepth - 1)
      }
    case .map:
      let (keyType, valType, size) = try readMapBegin()
      for _ in 0..<size {
        try skip(keyType, remainingDepth: remainingDepth - 1)
        try skip(valType, remainingDepth: remainingDepth - 1)
      }
    case .set, .list:
      let (elemType, size) = try readListBegin()
      for _ in 0..<size {
        try skip(elemType, remainingDepth: remainingDepth - 1)
      }
    case .stop, .void:
      throw ProtocolError.invalidData(
        "Cannot skip type \(fieldType)\(errorContext)")
    }
  }

  // MARK: - Helpers

  private func wireType(_ raw: UInt8) throws -> WireType {
    guard let value = WireType(rawValue: raw) else {
      throw ProtocolError.invalidData(
        "Unknown wire type \(raw)\(errorContext)")
    }
    return value
  }

  private func readCount() throws -> Int {
    let count = Int32(bitPattern: try readBigEndian32())
    if count < 0 {
      throw ProtocolError.invalidData(
        "Negative collection size \(count)\(errorContext)")
    }
    return Int(count)
  }

  private func readLengthPrefixedBytes() throws -> [UInt8] {
    let length = Int32(bitPattern: try readBigEndian32())
    if length < 0 {
      throw ProtocolError.invalidData(
        "Negative length \(length)\(errorContext)")
    }
    let count = Int(length)
    if stringSizeLimit > 0 && count > stringSizeLimit {
      throw ProtocolError.invalidData(
        "Length \(count) exceeds size limit \(stringSizeLimit)"
          + errorContext)
    }
    if count == 0 {
      return []
    }
    return try readExact(count)
  }

  private func readRawByte() throws -> UInt8 {
    if position >= buffer.count {
      throw ProtocolError.endOfBuffer("Unexpected end of buffer")
    }
    let b = buffer[position]
    position += 1
    return b
  }

  private func readExact(_ count: Int) throws -> [UInt8] {
    if position + count > buffer.count {
      throw ProtocolError.endOfBuffer(
        "Unexpected end of buffer, expected \(count) bytes but got "
          + "\(buffer.count - position)")
    }
    let slice = buffer[position..<(position + count)]
    position += count
    return Array(slice)
  }

  private func readBigEndian16() throws -> UInt16 {
    let bytes = try readExact(2)
    return (UInt16(bytes[0]) << 8) | UInt16(bytes[1])
  }

  private func readBigEndian32() throws -> UInt32 {
    let bytes = try readExact(4)
    return (UInt32(bytes[0]) << 24) | (UInt32(bytes[1]) << 16)
      | (UInt32(bytes[2]) << 8) | UInt32(bytes[3])
  }

  private func readBigEndian64() throws -> UInt64 {
    let bytes = try readExact(8)
    var value: UInt64 = 0
    for b in bytes {
      value = (value << 8) | UInt64(b)
    }
    return value
  }

  /// Minimum number of on-wire bytes a single element of `type` can occupy in
  /// the binary protocol. Elements are fixed-width or length-prefixed, so this
  /// lets `validateCollectionSize` reject a header claiming more elements than
  /// the remaining buffer could possibly hold.
  private func minEncodedBytes(_ type: WireType) -> Int {
    switch type {
    case .bool, .byte: return 1
    case .i16: return 2
    case .i32, .float: return 4
    case .i64, .double: return 8
    case .string: return 4  // 4-byte length prefix, possibly empty payload
    case .struct: return 1  // at minimum a stop byte
    case .set, .list: return 5  // element type(1) + count(4)
    case .map: return 6  // key type(1) + value type(1) + count(4)
    case .stop, .void: return 1
    }
  }

  private func validateCollectionSize(
    _ size: Int, minBytesPerElement: Int
  ) throws {
    // `size` comes from `readCount()`, which already rejects negatives.
    if collectionSizeLimit > 0 && size > collectionSizeLimit {
      throw ProtocolError.invalidData(
        "Collection size \(size) exceeds size limit \(collectionSizeLimit)"
          + errorContext)
    }
    let (minRequired, overflow) = size.multipliedReportingOverflow(
      by: minBytesPerElement)
    if overflow || minRequired > remainingBytes {
      throw ProtocolError.invalidData(
        "Collection size \(size) exceeds remaining bytes "
          + "\(remainingBytes)\(errorContext)")
    }
  }
}
