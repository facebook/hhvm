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

namespace cpp thrift.test.getter_setter

typedef binary (cpp2.type = "std::unique_ptr<folly::IOBuf>") IOBufPtr

struct GetterSetterTest {
  1: optional i32 optionalInt;
  2: i32 defaultInt;
  3: optional list<i32> optionalList;
  4: list<i32> defaultList;
  5: optional IOBufPtr optionalBuf;
  6: IOBufPtr defaultBuf;
}

struct MyStruct {
  1: optional MyEnum enumvalue;
}

enum MyEnum {
  Apple = 0,
  Banana = 1,
  Carrot = 2,
}
