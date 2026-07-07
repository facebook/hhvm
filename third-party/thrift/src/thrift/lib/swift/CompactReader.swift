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

/// Reads Thrift data types using the compact protocol format.
///
/// Uses varint encoding for integers, zigzag encoding for signed types,
/// delta-encoded field IDs, and compressed collection headers. Reads from an
/// in-memory buffer.
public final class CompactReader: ProtocolReader {
  /// Default maximum nesting depth for skip operations. Matches Rust's
  /// DEFAULT_RECURSION_DEPTH.
  public static let defaultMaxDepth = 64

  private let buffer: [UInt8]
  private var position: Int = 0
  private var lastFieldId: Int16 = 0
  private var currentFieldId: Int16?
  private var pendingBoolValue = false
  private var hasPendingBool = false

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
    let header = try readRawByte()
    if header == CompactType.stop.rawValue {
      currentFieldId = nil
      return (.stop, 0)
    }

    guard let compactType = CompactType(rawValue: header & 0x0F) else {
      throw ProtocolError.invalidData(
        "Unknown compact type \(header & 0x0F)\(errorContext)")
    }
    let delta = Int16((header >> 4) & 0x0F)

    let fieldId: Int16
    if delta != 0 {
      // Compute in Int and validate: a crafted short-form delta applied
      // to a large prior field id must fail gracefully, not trap.
      guard let narrowed = Int16(exactly: Int(lastFieldId) + Int(delta))
      else {
        throw ProtocolError.invalidData(
          "Field id delta overflow: \(lastFieldId) + \(delta)"
            + errorContext)
      }
      fieldId = narrowed
    } else {
      fieldId = try readI16()
    }
    lastFieldId = fieldId
    currentFieldId = fieldId

    if compactType == .boolTrue {
      pendingBoolValue = true
      hasPendingBool = true
      return (.bool, fieldId)
    }
    if compactType == .boolFalse {
      pendingBoolValue = false
      hasPendingBool = true
      return (.bool, fieldId)
    }

    return (try compactType.toWireType(), fieldId)
  }

  public func readBool() throws -> Bool {
    if hasPendingBool {
      hasPendingBool = false
      return pendingBoolValue
    }
    return try readRawByte() == CompactType.boolTrue.rawValue
  }

  public func readByte() throws -> Int8 {
    Int8(bitPattern: try readRawByte())
  }

  public func readI16() throws -> Int16 {
    Int16(truncatingIfNeeded: zigzagToInt(try readVarint32()))
  }

  public func readI32() throws -> Int32 {
    zigzagToInt(try readVarint32())
  }

  public func readI64() throws -> Int64 {
    zigzagToLong(try readVarint64())
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

  private func readLengthPrefixedBytes() throws -> [UInt8] {
    let length = Int(try readVarint32())
    if length < 0 {
      throw ProtocolError.invalidData(
        "Negative length \(length)\(errorContext)")
    }
    if stringSizeLimit > 0 && length > stringSizeLimit {
      throw ProtocolError.invalidData(
        "Length \(length) exceeds size limit \(stringSizeLimit)"
          + errorContext)
    }
    if length == 0 {
      return []
    }
    return try readExact(length)
  }

  public func readListBegin() throws
    -> (elemType: WireType, size: Int)
  {
    let header = try readRawByte()
    var size = Int((header >> 4) & 0x0F)
    guard let compactElemType = CompactType(rawValue: header & 0x0F) else {
      throw ProtocolError.invalidData(
        "Unknown compact element type \(header & 0x0F)\(errorContext)")
    }

    if size == 15 {
      size = Int(try readVarint32())
    }

    try validateCollectionSize(size)
    return (try compactElemType.toWireType(), size)
  }

  public func readSetBegin() throws
    -> (elemType: WireType, size: Int)
  {
    try readListBegin()
  }

  public func readMapBegin() throws
    -> (keyType: WireType, valType: WireType, size: Int)
  {
    let size = Int(try readVarint32())
    if size == 0 {
      return (.stop, .stop, 0)
    }

    // Read the 1-byte key/val types header first, then bound the pair bytes
    // against what actually remains (so the size check accounts for this byte).
    let types = try readRawByte()
    guard
      let keyCompactType = CompactType(rawValue: (types >> 4) & 0x0F),
      let valCompactType = CompactType(rawValue: types & 0x0F)
    else {
      throw ProtocolError.invalidData(
        "Unknown compact map type byte \(types)\(errorContext)")
    }

    try validateCollectionSize(size, minBytesPerElement: 2)

    return (
      try keyCompactType.toWireType(),
      try valCompactType.toWireType(),
      size
    )
  }

  public func readStruct<T: ThriftSerializable>() throws -> T {
    // Field-id delta and any pending bool header are per-struct state; save and
    // reset them so a nested struct read starts clean and cannot observe the
    // outer struct's stale pending bool.
    let savedFieldId = lastFieldId
    let savedHasPendingBool = hasPendingBool
    let savedPendingBoolValue = pendingBoolValue
    lastFieldId = 0
    hasPendingBool = false
    pendingBoolValue = false
    defer {
      lastFieldId = savedFieldId
      hasPendingBool = savedHasPendingBool
      pendingBoolValue = savedPendingBoolValue
    }
    return try T(from: self)
  }

  public func skip(_ fieldType: WireType) throws {
    try skip(fieldType, remainingDepth: CompactReader.defaultMaxDepth)
  }

  private func skip(_ fieldType: WireType, remainingDepth: Int) throws {
    if remainingDepth <= 0 {
      throw ProtocolError.depthLimitExceeded(
        "Maximum nesting depth (\(CompactReader.defaultMaxDepth)) "
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
      // Save/reset all per-struct state (mirrors readStruct) so a skipped
      // nested struct cannot observe the outer struct's field-id delta or a
      // stale pending bool.
      let savedFieldId = lastFieldId
      let savedHasPendingBool = hasPendingBool
      let savedPendingBoolValue = pendingBoolValue
      lastFieldId = 0
      hasPendingBool = false
      pendingBoolValue = false
      defer {
        lastFieldId = savedFieldId
        hasPendingBool = savedHasPendingBool
        pendingBoolValue = savedPendingBoolValue
      }
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

  // MARK: - Varint / Zigzag helpers

  private func readVarint32() throws -> UInt32 {
    var result: UInt32 = 0
    var shift: UInt32 = 0
    while true {
      let b = try readRawByte()
      // The 5th byte (shift 28) holds only 4 value bits; reject a byte whose
      // higher bits would otherwise be silently truncated by the shift.
      if shift == 28 && (b & 0x70) != 0 {
        throw ProtocolError.invalidData("Varint32 overflow")
      }
      result |= UInt32(b & 0x7F) << shift
      if (b & 0x80) == 0 {
        break
      }
      shift += 7
      if shift > 28 {
        throw ProtocolError.invalidData("Varint32 too long")
      }
    }
    return result
  }

  private func readVarint64() throws -> UInt64 {
    var result: UInt64 = 0
    var shift: UInt64 = 0
    while true {
      let b = try readRawByte()
      // The 10th byte (shift 63) holds only 1 value bit; reject a byte whose
      // higher bits would otherwise be silently truncated by the shift.
      if shift == 63 && (b & 0x7E) != 0 {
        throw ProtocolError.invalidData("Varint64 overflow")
      }
      result |= UInt64(b & 0x7F) << shift
      if (b & 0x80) == 0 {
        break
      }
      shift += 7
      if shift > 63 {
        throw ProtocolError.invalidData("Varint64 too long")
      }
    }
    return result
  }

  private func zigzagToInt(_ n: UInt32) -> Int32 {
    Int32(bitPattern: (n >> 1)) ^ -Int32(bitPattern: n & 1)
  }

  private func zigzagToLong(_ n: UInt64) -> Int64 {
    Int64(bitPattern: (n >> 1)) ^ -Int64(bitPattern: n & 1)
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

  private func validateCollectionSize(
    _ size: Int, minBytesPerElement: Int = 1
  ) throws {
    if size < 0 {
      throw ProtocolError.invalidData(
        "Negative collection size \(size)\(errorContext)")
    }
    if collectionSizeLimit > 0 && size > collectionSizeLimit {
      throw ProtocolError.invalidData(
        "Collection size \(size) exceeds size limit \(collectionSizeLimit)"
          + errorContext)
    }
    // Overflow-safe lower bound: every element needs at least
    // `minBytesPerElement` bytes, so a size larger than the remaining buffer
    // can hold is malformed. Guards against huge `reserveCapacity` requests.
    let (minRequired, overflow) = size.multipliedReportingOverflow(
      by: minBytesPerElement)
    if overflow || minRequired > remainingBytes {
      throw ProtocolError.invalidData(
        "Collection size \(size) exceeds remaining bytes "
          + "\(remainingBytes)\(errorContext)")
    }
  }
}
