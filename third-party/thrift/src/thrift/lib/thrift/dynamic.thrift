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

namespace cpp2 apache.thrift
namespace php thrift
namespace py apache.thrift.dynamic
namespace py.asyncio apache.asyncio.thrift.dynamic
namespace py3 apache.thrift
namespace java.swift org.apache.thrift.dynamic
namespace js thrift

cpp_include "thrift/lib/thrift/SerializableDynamic.h"

union Dynamic {
  1: bool boolean (java.swift.name = "_boolean");
  2: i64 integer;
  3: double doubl;
  4: binary str;
  5: list<Dynamic> arr;
  6: map<string, Dynamic> object;
}

typedef Dynamic (cpp.type = "::apache::thrift::SerializableDynamic") DynamicType
