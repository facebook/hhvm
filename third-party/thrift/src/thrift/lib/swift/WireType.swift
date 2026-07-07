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

/// Wire type identifiers used in Thrift binary and compact protocol
/// serialization. These identify the encoding format of a value on the wire
/// (e.g. "this field is encoded as a 32-bit integer"), NOT the Thrift IDL type
/// system.
/// See https://github.com/apache/thrift/blob/master/doc/specs/thrift-binary-protocol.md
public enum WireType: UInt8 {
  case stop = 0
  case void = 1
  case bool = 2
  case byte = 3
  case double = 4
  case i16 = 6
  case i32 = 8
  case i64 = 10
  case string = 11
  case `struct` = 12
  case map = 13
  case set = 14
  case list = 15
  case float = 19
}
