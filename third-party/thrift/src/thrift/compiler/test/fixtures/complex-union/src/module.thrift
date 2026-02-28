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

namespace java.swift test.fixtures.complex_union

include "thrift/annotation/cpp.thrift"
include "thrift/annotation/thrift.thrift"

@thrift.AllowLegacyMissingUris
package;

typedef map<i16, string> containerTypedef

union ComplexUnion {
  1: i64 intValue;
  5: string stringValue;
  2: list<i64> intListValue;
  3: list<string> stringListValue;
  9: containerTypedef typedefValue;
  @cpp.Ref{type = cpp.RefType.Unique}
  14: string stringRef;
}

union ListUnion {
  2: list<i64> intListValue;
  3: list<string> stringListValue;
}

union DataUnion {
  1: binary binaryData;
  2: string stringData;
}

struct Val {
  1: string strVal;
  2: i32 intVal;
  9: containerTypedef typedefValue;
}

union ValUnion {
  1: Val v1;
  2: Val v2;
}

union VirtualComplexUnion {
  1: string thingOne;
  2: string thingTwo;
} (cpp.virtual)

struct NonCopyableStruct {
  1: i64 num;
} (cpp.noncopyable)

union NonCopyableUnion {
  1: NonCopyableStruct s;
} (cpp.noncopyable)
