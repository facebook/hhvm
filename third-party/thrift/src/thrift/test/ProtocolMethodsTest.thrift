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

namespace cpp2 apache.thrift.test

cpp_include "thrift/test/ProtocolMethodsTestCustomTypes.h"

struct MyStruct {
  1: i32_7453 num;
  2: string_9131 str;
} (no_default_comparators)

// The following were automatically generated and may benefit from renaming.
typedef i32 (cpp.type = "MyInt") i32_7453
typedef string (cpp.type = "MyString") string_9131
