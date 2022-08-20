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

namespace cpp2 apache.thrift.test
namespace java thrift.test

enum Gender {
  MALE = 1,
  FEMALE = 2,
}

struct myMapStruct {
  1: map<string, string> stringMap;
  2: map<bool, string> boolMap;
  3: map<byte, string> byteMap;
  4: map<double, string> doubleMap;
  5: map<Gender, string> enumMap;
}
