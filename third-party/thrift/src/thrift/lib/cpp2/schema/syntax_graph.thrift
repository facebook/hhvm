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

namespace cpp2 apache.thrift.syntax_graph
namespace rust syntax_graph
namespace py3 apache.thrift.syntax_graph

include "thrift/annotation/cpp.thrift"
include "thrift/annotation/thrift.thrift"

@thrift.AllowLegacyMissingUris
package;

enum Primitive {
  BOOL = 1,
  BYTE = 2,
  I16 = 3,
  I32 = 4,
  I64 = 5,
  FLOAT = 6,
  DOUBLE = 7,
  STRING = 8,
  BINARY = 9,
}

enum FieldPresenceQualifier {
  UNQUALIFIED = 1,
  @cpp.Name{value = "OPTIONAL_"}
  OPTIONAL = 2,
}
