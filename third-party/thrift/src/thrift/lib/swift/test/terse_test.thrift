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

namespace swift FBThriftTestsTerse

struct TerseFields {
  @thrift.TerseWrite
  1: i32 terse_int;
  @thrift.TerseWrite
  2: string terse_string;
  // Always-present control field (always serialized, even at default).
  3: i32 normal_int;
  @thrift.TerseWrite
  4: list<i32> terse_list;
}
