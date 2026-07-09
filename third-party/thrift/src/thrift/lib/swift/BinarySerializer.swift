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

/// Serializes/deserializes Thrift structs to/from Binary-protocol bytes.
/// Mirrors the per-protocol serializer convention of other Thrift languages.
public enum BinarySerializer {
  /// Serializes `value` to binary-protocol bytes.
  public static func serialize<T: ThriftSerializable>(_ value: T) -> Data {
    let writer = BinaryWriter()
    value.write(to: writer)
    return writer.data
  }

  /// Deserializes a `T` from binary-protocol bytes. Throws if the input has
  /// trailing bytes after the value (a likely framing bug).
  public static func deserialize<T: ThriftSerializable>(
    _ type: T.Type = T.self, from data: Data
  ) throws -> T {
    let reader = BinaryReader(data: data)
    let value = try T(from: reader)
    if !reader.isAtEnd {
      throw ProtocolError.invalidData(
        "Trailing bytes after value of type \(T.self)")
    }
    return value
  }
}
