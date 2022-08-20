/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

namespace cpp2 metadata.test.include

include "thrift/lib/cpp2/test/metadata/enum_test.thrift"
include "thrift/lib/cpp2/test/metadata/exception_test.thrift"
include "thrift/lib/cpp2/test/metadata/simple_structs_test.thrift"
include "thrift/lib/cpp2/test/metadata/typedef_test.thrift"

typedef string CoolString

enum Animal {
  Dog = 1,
  Cat = 2,
  Horse = 3,
  Cow = 4,
  Bear = 5,
}

struct Example {
  1: enum_test.Continent includedEnum;
  2: exception_test.RuntimeException includedException;
  3: simple_structs_test.Country includedStruct;
  4: typedef_test.StringMap includedTypedef;
  5: CoolString coolString;
}

service IncludeTestService extends simple_structs_test.SimpleStructsTestService {
  Example getExample(1: Animal animal) throws (
    1: exception_test.RuntimeException ex,
  );
}
