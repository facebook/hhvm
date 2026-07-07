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

/// Protocol-agnostic interface for reading Thrift data.
///
/// The mirror of `ProtocolWriter`. Generated code reads exclusively
/// through this interface. Container reads (`readList`/`readSet`/`readMap`) are
/// supplied as protocol extensions.
public protocol ProtocolReader {
  func readFieldBegin() throws -> (fieldType: WireType, fieldId: Int16)
  func readBool() throws -> Bool
  func readByte() throws -> Int8
  func readI16() throws -> Int16
  func readI32() throws -> Int32
  func readI64() throws -> Int64
  func readFloat() throws -> Float
  func readDouble() throws -> Double
  func readString() throws -> String
  func readBinary() throws -> Data
  func readListBegin() throws -> (elemType: WireType, size: Int)
  func readSetBegin() throws -> (elemType: WireType, size: Int)
  func readMapBegin() throws
    -> (keyType: WireType, valType: WireType, size: Int)
  func readStruct<T: ThriftSerializable>() throws -> T
  func skip(_ fieldType: WireType) throws
}

extension ProtocolReader {
  /// Reads a list, including its header and every element.
  public func readList<T: ThriftReadable>() throws -> [T] {
    let (elemType, size) = try readListBegin()
    guard size == 0 || elemType == T.thriftWireType else {
      throw ProtocolError.invalidData(
        "List element wire type \(elemType) does not match expected "
          + "\(T.thriftWireType)")
    }
    var result = [T]()
    result.reserveCapacity(size)
    for _ in 0..<size {
      result.append(try T.thriftRead(from: self))
    }
    return result
  }

  /// Reads a set, including its header and every element.
  public func readSet<T: ThriftReadable & Hashable>() throws -> Set<T> {
    let (elemType, size) = try readSetBegin()
    guard size == 0 || elemType == T.thriftWireType else {
      throw ProtocolError.invalidData(
        "Set element wire type \(elemType) does not match expected "
          + "\(T.thriftWireType)")
    }
    var result = Set<T>(minimumCapacity: size)
    for _ in 0..<size {
      result.insert(try T.thriftRead(from: self))
    }
    return result
  }

  /// Reads a map, including its header and every key/value pair.
  public func readMap<K: ThriftReadable & Hashable, V: ThriftReadable>()
    throws -> [K: V]
  {
    let (keyType, valType, size) = try readMapBegin()
    guard size == 0 || (keyType == K.thriftWireType && valType == V.thriftWireType)
    else {
      throw ProtocolError.invalidData(
        "Map key/value wire types (\(keyType), \(valType)) do not match "
          + "expected (\(K.thriftWireType), \(V.thriftWireType))")
    }
    var result = [K: V](minimumCapacity: size)
    for _ in 0..<size {
      let key = try K.thriftRead(from: self)
      let val = try V.thriftRead(from: self)
      result[key] = val
    }
    return result
  }
}
