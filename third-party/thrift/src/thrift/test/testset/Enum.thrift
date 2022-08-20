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

package "apache.org/thrift/test/testset"

namespace py3 thrift.test
namespace py thrift.test.Enum
namespace py.asyncio thrift_asyncio.test.Enum
namespace java.swift org.apache.thrift.test
namespace go thrift.test.testset.Enum

enum Standard {
  value_0 = 0,
  value_1 = 1,
  value_2 = 2,
}

enum NoZero {
  value_1 = 1,
  value_2 = 2,
}

enum MissingField {
  value_0 = 0,
  value_1 = 1,
}

enum NameMismatch {
  value_0 = 0,
  value_1 = 1,
  value_3 = 2,
}

enum ValueMismatch {
  value_0 = 0,
  value_1 = 1,
  value_2 = 3,
}

struct StandardEnumStruct {
  1: optional Standard field;
}

struct NoZeroEnumStruct {
  1: optional NoZero field;
}

struct LessFieldEnumStruct {
  1: optional MissingField field;
}

struct DifferentNameEnumStruct {
  1: optional NameMismatch field;
}

struct DifferentValueEnumStruct {
  1: optional ValueMismatch field;
}
