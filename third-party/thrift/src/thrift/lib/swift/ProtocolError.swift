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

/// Error thrown when a Thrift protocol error occurs during serialization or
/// deserialization.
public enum ProtocolError: Error, CustomStringConvertible, Equatable {
  /// The wire data is malformed or violates protocol invariants.
  case invalidData(String)
  /// The stream ended before the expected number of bytes could be read.
  case endOfBuffer(String)
  /// A nested structure exceeded the maximum allowed depth while skipping or
  /// reading.
  case depthLimitExceeded(String)

  public var description: String {
    switch self {
    case .invalidData(let m): return "ProtocolError.invalidData: \(m)"
    case .endOfBuffer(let m): return "ProtocolError.endOfBuffer: \(m)"
    case .depthLimitExceeded(let m):
      return "ProtocolError.depthLimitExceeded: \(m)"
    }
  }
}
