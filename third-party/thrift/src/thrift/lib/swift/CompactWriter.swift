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

/// Writes Thrift data types using the compact protocol format.
///
/// Uses varint encoding for integers, zigzag encoding for signed types,
/// delta-encoded field IDs, and compressed collection headers. Output is
/// accumulated into an in-memory buffer available via `bytes`/`data`.
public final class CompactWriter: ProtocolWriter {
  /// Default maximum nesting depth for struct-write operations. Matches the
  /// reader's `defaultMaxDepth` (and Rust's DEFAULT_RECURSION_DEPTH), so a
  /// pathologically deep in-memory object graph fails with a clear diagnostic
  /// rather than overflowing the stack mid-serialization.
  public static let defaultMaxDepth = 64

  private var buffer: [UInt8] = []
  private var lastFieldId: Int16 = 0
  private var pendingBool = false
  private var pendingFieldId: Int16 = 0

  /// Current struct-write nesting depth. Bounded by `defaultMaxDepth` (the
  /// write-side mirror of the reader's struct-read depth guard).
  private var depth = 0

  public init() {}

  /// The bytes written so far.
  public var bytes: [UInt8] { buffer }

  /// The bytes written so far, as `Data`.
  public var data: Data { Data(buffer) }

  public func writeFieldBegin(_ fieldType: WireType, _ fieldId: Int16) {
    // A bool field header is deferred until writeBool folds the value into it;
    // any other write before that would emit a header with the stale field id.
    // `precondition` (unlike `assert`) also fires in release builds, so misuse
    // of this public API fails loudly instead of silently corrupting the wire.
    precondition(
      !pendingBool, "writeBool must immediately follow a bool writeFieldBegin")
    if fieldType == .bool {
      pendingBool = true
      pendingFieldId = fieldId
      return
    }
    writeFieldHeader(CompactType.from(fieldType), fieldId)
  }

  private func writeFieldHeader(_ compactType: CompactType, _ fieldId: Int16) {
    let delta = Int(fieldId) - Int(lastFieldId)
    if delta > 0 && delta <= 15 {
      buffer.append(UInt8((delta << 4) | Int(compactType.rawValue)))
    } else {
      buffer.append(compactType.rawValue)
      writeZigzagVarint16(fieldId)
    }
    lastFieldId = fieldId
  }

  public func writeFieldStop() {
    precondition(
      !pendingBool, "writeBool must immediately follow a bool writeFieldBegin")
    buffer.append(CompactType.stop.rawValue)
  }

  public func writeBool(_ value: Bool) {
    let compactType: CompactType = value ? .boolTrue : .boolFalse
    if pendingBool {
      writeFieldHeader(compactType, pendingFieldId)
      pendingBool = false
    } else {
      buffer.append(compactType.rawValue)
    }
  }

  public func writeByte(_ value: Int8) {
    buffer.append(UInt8(bitPattern: value))
  }

  public func writeI16(_ value: Int16) {
    writeVarint32(intToZigzag(Int32(value)))
  }

  public func writeI32(_ value: Int32) {
    writeVarint32(intToZigzag(value))
  }

  public func writeI64(_ value: Int64) {
    writeVarint64(longToZigzag(value))
  }

  public func writeFloat(_ value: Float) {
    appendBigEndian(value.bitPattern)
  }

  public func writeDouble(_ value: Double) {
    appendBigEndian(value.bitPattern)
  }

  public func writeString(_ value: String) {
    let utf8 = Array(value.utf8)
    // No count > UInt32.max check: the cast traps on overflow, and such sizes OOM first.
    writeVarint32(UInt32(utf8.count))
    buffer.append(contentsOf: utf8)
  }

  public func writeBinary(_ value: Data) {
    // No count > UInt32.max check: the cast traps on overflow, and such sizes OOM first.
    writeVarint32(UInt32(value.count))
    buffer.append(contentsOf: value)
  }

  public func writeListBegin(_ elemType: WireType, _ size: Int) {
    precondition(size >= 0, "Collection size must be non-negative, got \(size)")
    let compactElemType = CompactType.from(elemType)
    if size <= 14 {
      buffer.append(UInt8((size << 4) | Int(compactElemType.rawValue)))
    } else {
      buffer.append(0xF0 | compactElemType.rawValue)
      writeVarint32(UInt32(size))
    }
  }

  public func writeSetBegin(_ elemType: WireType, _ size: Int) {
    writeListBegin(elemType, size)
  }

  public func writeMapBegin(
    _ keyType: WireType, _ valType: WireType, _ size: Int
  ) {
    precondition(size >= 0, "Map size must be non-negative, got \(size)")
    writeVarint32(UInt32(size))
    if size == 0 {
      return
    }
    let compactKeyType = CompactType.from(keyType)
    let compactValType = CompactType.from(valType)
    buffer.append(
      (compactKeyType.rawValue << 4) | compactValType.rawValue)
  }

  public func writeStruct<T: ThriftSerializable>(_ value: T) {
    depth += 1
    defer { depth -= 1 }
    // `precondition` (unlike `assert`) also fires in release builds, mirroring
    // the reader's depth guard: the write side traps rather than throws because
    // the whole write API is non-throwing and the input is a trusted in-memory
    // object, so exceeding this is a programming error, not malformed input.
    precondition(
      depth <= CompactWriter.defaultMaxDepth,
      "Maximum nesting depth (\(CompactWriter.defaultMaxDepth)) "
        + "exceeded while writing struct")
    let savedFieldId = lastFieldId
    lastFieldId = 0
    defer { lastFieldId = savedFieldId }
    value.write(to: self)
  }

  // MARK: - Varint / Zigzag helpers

  private func writeVarint32(_ value: UInt32) {
    var n = value
    while (n & ~0x7F) != 0 {
      buffer.append(UInt8((n & 0x7F) | 0x80))
      n >>= 7
    }
    buffer.append(UInt8(n))
  }

  private func writeVarint64(_ value: UInt64) {
    var n = value
    while (n & ~0x7F) != 0 {
      buffer.append(UInt8((n & 0x7F) | 0x80))
      n >>= 7
    }
    buffer.append(UInt8(n))
  }

  private func writeZigzagVarint16(_ value: Int16) {
    writeVarint32(intToZigzag(Int32(value)))
  }

  private func appendBigEndian(_ value: UInt32) {
    buffer.append(UInt8((value >> 24) & 0xFF))
    buffer.append(UInt8((value >> 16) & 0xFF))
    buffer.append(UInt8((value >> 8) & 0xFF))
    buffer.append(UInt8(value & 0xFF))
  }

  private func appendBigEndian(_ value: UInt64) {
    for shift in stride(from: 56, through: 0, by: -8) {
      buffer.append(UInt8((value >> UInt64(shift)) & 0xFF))
    }
  }

  private func intToZigzag(_ n: Int32) -> UInt32 {
    UInt32(bitPattern: (n << 1) ^ (n >> 31))
  }

  private func longToZigzag(_ n: Int64) -> UInt64 {
    UInt64(bitPattern: (n << 1) ^ (n >> 63))
  }
}
