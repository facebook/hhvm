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

#pragma once

namespace apache::thrift::type {

enum class BaseType {
  Void = 0,

  // Integer types.
  Bool = 1,
  Byte = 2,
  I16 = 3,
  I32 = 4,
  I64 = 5,

  // Floating point types.
  Float = 6,
  Double = 7,

  // String types.
  String = 8,
  Binary = 9,

  // Enum type class.
  Enum = 10,

  // Structured type classes.
  Struct = 11,
  Union = 12,
  Exception = 13,

  // Container type classes.
  List = 14,
  Set = 15,
  Map = 16
};

}
