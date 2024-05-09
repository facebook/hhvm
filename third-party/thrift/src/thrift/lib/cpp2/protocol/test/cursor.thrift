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

include "thrift/annotation/cpp.thrift"
include "thrift/annotation/thrift.thrift"

cpp_include "thrift/lib/cpp2/protocol/CursorBasedSerializer.h"

@thrift.Experimental
package "apache.org/thrift/test"

struct Qualifiers {
  1: optional i32 opt;
  2: i32 unq = 1;
  @thrift.TerseWrite
  3: i32 terse = 2;
}

struct Cookie {
  1: i16 id = 2;
  2: string fortune = "About time I got out of that cookie!!";
  3: list<i32> lucky_numbers = [508, 493, 425];
}
struct Meal {
  1: i16 appetizer = 1;
  2: i32 drink = -8;
  3: i16 main = 2;
  4: Cookie cookie;
  5: i16 dessert = 3;
}

union Stringish {
  1: string string_field;
  @cpp.Type{name = "folly::IOBuf"}
  2: binary binary_field;
}

enum E {
  UNKNOWN = 0,
  A = 1,
  B = 2,
}
struct Numerics {
  1: optional i16 int16;
  @cpp.Type{name = "uint32_t"}
  2: i32 uint32;
  3: E enm;
  4: float flt;
}

struct Empty {}
@cpp.UseCursorSerialization
typedef Empty EmptyWrapper

service Example {
  EmptyWrapper identity(1: EmptyWrapper empty);
}
