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

namespace cpp2 cpp2

include "thrift/annotation/cpp.thrift"
include "thrift/annotation/thrift.thrift"

@thrift.AllowLegacyMissingUris
package;

// Test struct and union with @cpp.NonOrderable annotation
@cpp.NonOrderable
struct NonOrderableStruct {
  1: i32 field1;
  2: string field2;
}

@cpp.NonOrderable
union NonOrderableUnion {
  1: i32 int_value;
  2: string string_value;
}

// Test struct and union without the annotation (should have operator<)
struct OrderableStruct {
  1: i32 field1;
  2: string field2;
}

union OrderableUnion {
  1: i32 int_value;
  2: string string_value;
}
