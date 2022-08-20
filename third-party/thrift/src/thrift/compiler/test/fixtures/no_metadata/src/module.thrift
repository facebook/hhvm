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

enum MyEnum {
  MyValue1 = 0,
  MyValue2 = 1,
}

struct MyStruct {
  1: i64 MyIntField;
  2: string MyStringField;
  # use the type before it is defined. Thrift should be able to handle this
  3: MyDataItem MyDataField;
  4: MyEnum myEnum;
}

struct MyDataItem {}

union MyUnion {
  1: MyEnum myEnum;
  2: MyStruct myStruct;
  3: MyDataItem myDataItem;
}

service MyService {
  void ping();
  string getRandomData();
  bool hasDataById(1: i64 id);
  string getDataById(1: i64 id);
  void putDataById(1: i64 id, 2: string data);
  oneway void lobDataById(1: i64 id, 2: string data);
}
