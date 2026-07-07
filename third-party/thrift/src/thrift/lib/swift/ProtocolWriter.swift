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

/// Protocol-agnostic interface for writing Thrift data.
///
/// Generated code writes exclusively through this interface using neutral
/// `WireType` values, so any conforming protocol implementation (e.g.
/// `CompactWriter`, and a future binary writer) is a drop-in.
///
/// Conforming types must provide the primitive write operations, the collection
/// headers, and `writeStruct`. Container writes (`writeList`/`writeSet`/
/// `writeMap`) are supplied as protocol extensions.
public protocol ProtocolWriter {
  func writeFieldBegin(_ fieldType: WireType, _ fieldId: Int16)
  func writeFieldStop()
  func writeBool(_ value: Bool)
  func writeByte(_ value: Int8)
  func writeI16(_ value: Int16)
  func writeI32(_ value: Int32)
  func writeI64(_ value: Int64)
  func writeFloat(_ value: Float)
  func writeDouble(_ value: Double)
  func writeString(_ value: String)
  func writeBinary(_ value: Data)
  func writeListBegin(_ elemType: WireType, _ size: Int)
  func writeSetBegin(_ elemType: WireType, _ size: Int)
  func writeMapBegin(
    _ keyType: WireType, _ valType: WireType, _ size: Int)
  func writeStruct<T: ThriftSerializable>(_ value: T)
}

extension ProtocolWriter {
  /// Writes a list, including its header and every element.
  public func writeList<T: ThriftWritable>(_ value: [T]) {
    writeListBegin(T.thriftWireType, value.count)
    for elem in value {
      elem.thriftWrite(to: self)
    }
  }

  /// Writes a set, including its header and every element.
  public func writeSet<T: ThriftWritable & Hashable>(_ value: Set<T>) {
    writeSetBegin(T.thriftWireType, value.count)
    for elem in value {
      elem.thriftWrite(to: self)
    }
  }

  /// Writes a map, including its header and every key/value pair.
  public func writeMap<K: ThriftWritable & Hashable, V: ThriftWritable>(
    _ value: [K: V]
  ) {
    writeMapBegin(K.thriftWireType, V.thriftWireType, value.count)
    for (key, val) in value {
      key.thriftWrite(to: self)
      val.thriftWrite(to: self)
    }
  }
}
