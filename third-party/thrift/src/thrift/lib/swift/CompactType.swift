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

/// Compact Protocol type constants. These differ from the neutral
/// `WireType` and from binary protocol type IDs — compact protocol packs
/// types into 4 bits (0-13) for use in field and collection headers.
enum CompactType: UInt8 {
  case stop = 0
  case boolTrue = 1
  case boolFalse = 2
  case byte = 3
  case i16 = 4
  case i32 = 5
  case i64 = 6
  case double = 7
  case binary = 8
  case list = 9
  case set = 10
  case map = 11
  case `struct` = 12
  case float = 13

  /// Converts a neutral wire type to its compact protocol type ID.
  static func from(_ wireType: WireType) -> CompactType {
    switch wireType {
    case .stop: return .stop
    case .bool: return .boolTrue
    case .byte: return .byte
    case .i16: return .i16
    case .i32: return .i32
    case .i64: return .i64
    case .double: return .double
    case .string: return .binary
    case .list: return .list
    case .set: return .set
    case .map: return .map
    case .struct: return .struct
    case .float: return .float
    case .void:
      preconditionFailure(
        "WireType.void has no compact-protocol type; it must never appear as a "
          + "field, element, key, or value type")
    }
  }

  /// Converts a compact protocol type ID to the neutral wire type.
  func toWireType() throws -> WireType {
    switch self {
    case .stop: return .stop
    case .boolTrue, .boolFalse: return .bool
    case .byte: return .byte
    case .i16: return .i16
    case .i32: return .i32
    case .i64: return .i64
    case .double: return .double
    case .binary: return .string
    case .list: return .list
    case .set: return .set
    case .map: return .map
    case .struct: return .struct
    case .float: return .float
    }
  }
}
