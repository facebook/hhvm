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

@thrift.AllowLegacyMissingUris
package;

struct MyAnnot {}

const i64 fortyTwo = 42;

typedef map<i64, string> MyMapTypedef

struct MyStruct {
  1: i64 MyIntField;
  @MyAnnot
  2: string MyStringField (annotation);
}

enum MyEnum {
  VALUE1 = 1,
  @MyAnnot
  VALUE2 = 2 (annotation),
}

service MyService {
  void ping();

  @MyAnnot
  void pong() (annotation);
}
