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

/// Writes Thrift data types using the binary protocol format.
///
/// All multi-byte integers and floats are big-endian. A field header is
/// type(1 byte) + field id(2 bytes); list/set headers are element-type(1) +
/// count(4); map headers are key-type(1) + value-type(1) + count(4). Because
/// `WireType`'s raw values are the binary protocol's on-wire TType ids,
/// headers write the wire type directly (no mapping table, unlike Compact).
/// Output is accumulated into an in-memory buffer available via `bytes`/`data`.
public final class BinaryWriter: ProtocolWriter {
  private var buffer: [UInt8] = []

  public init() {}

  /// The bytes written so far.
  public var bytes: [UInt8] { buffer }

  /// The bytes written so far, as `Data`.
  public var data: Data { Data(buffer) }

  public func writeFieldBegin(_ fieldType: WireType, _ fieldId: Int16) {
    buffer.append(fieldType.rawValue)
    appendBigEndian(UInt16(bitPattern: fieldId))
  }

  public func writeFieldStop() {
    buffer.append(WireType.stop.rawValue)
  }

  public func writeBool(_ value: Bool) {
    buffer.append(value ? 1 : 0)
  }

  public func writeByte(_ value: Int8) {
    buffer.append(UInt8(bitPattern: value))
  }

  public func writeI16(_ value: Int16) {
    appendBigEndian(UInt16(bitPattern: value))
  }

  public func writeI32(_ value: Int32) {
    appendBigEndian(UInt32(bitPattern: value))
  }

  public func writeI64(_ value: Int64) {
    appendBigEndian(UInt64(bitPattern: value))
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
    appendBigEndian(UInt32(utf8.count))
    buffer.append(contentsOf: utf8)
  }

  public func writeBinary(_ value: Data) {
    // No count > UInt32.max check: the cast traps on overflow, and such sizes OOM first.
    appendBigEndian(UInt32(value.count))
    buffer.append(contentsOf: value)
  }

  public func writeListBegin(_ elemType: WireType, _ size: Int) {
    precondition(size >= 0, "Collection size must be non-negative, got \(size)")
    buffer.append(elemType.rawValue)
    appendBigEndian(UInt32(size))
  }

  public func writeSetBegin(_ elemType: WireType, _ size: Int) {
    writeListBegin(elemType, size)
  }

  public func writeMapBegin(
    _ keyType: WireType, _ valType: WireType, _ size: Int
  ) {
    precondition(size >= 0, "Map size must be non-negative, got \(size)")
    buffer.append(keyType.rawValue)
    buffer.append(valType.rawValue)
    appendBigEndian(UInt32(size))
  }

  public func writeStruct<T: ThriftSerializable>(_ value: T) {
    // Binary has no field-id delta state to save/restore (unlike Compact); a
    // nested struct is just its own field sequence terminated by a stop byte.
    value.write(to: self)
  }

  // MARK: - Big-endian helpers

  private func appendBigEndian(_ value: UInt16) {
    buffer.append(UInt8((value >> 8) & 0xFF))
    buffer.append(UInt8(value & 0xFF))
  }

  private func appendBigEndian(_ value: UInt32) {
    for shift in stride(from: 24, through: 0, by: -8) {
      buffer.append(UInt8((value >> UInt32(shift)) & 0xFF))
    }
  }

  private func appendBigEndian(_ value: UInt64) {
    for shift in stride(from: 56, through: 0, by: -8) {
      buffer.append(UInt8((value >> UInt64(shift)) & 0xFF))
    }
  }
}
