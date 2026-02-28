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

include "thrift/annotation/thrift.thrift"

@thrift.AllowLegacyMissingUris
package;

namespace cpp2 apache.thrift.conformance
namespace php apache_thrift
namespace py thrift.conformance.protocol
namespace py.asyncio thrift_asyncio.conformance.protocol
namespace py3 thrift.conformance
namespace java.swift org.apache.thrift.conformance
namespace java org.apache.thrift.conformance
namespace go thrift.conformance.protocol

// Standard protocols.
enum StandardProtocol {
  // Indicates a standard protocol is not being used.
  Custom = 0,

  // The standard protocols.
  Binary = 1,
  Compact = 2,
  Json = 3,
  SimpleJson = 4,
}

// A struct representation of a protocol.
// TODO(afuller): Put this in another namespace or pick a better name.
struct ProtocolStruct {
  // The standard protocol or StandardProtocol::Custom.
  1: StandardProtocol standard;
  // The custom protocol, iff standard == StandardProtocol::Custom.
  2: optional string custom;
}
