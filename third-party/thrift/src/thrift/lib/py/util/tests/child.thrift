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

include "thrift/lib/py/util/tests/parent.thrift"
include "thrift/annotation/thrift.thrift"

namespace py thrift.test.child

exception ChildError {
  @thrift.ExceptionMessage
  1: required string message;
  2: optional i32 errorCode;
}

struct SomeStruct {
  1: string data;
}

enum AnEnum {
  FOO = 1,
  BAR = 2,
}

service ChildService extends parent.ParentService {
  oneway void shoutIntoTheWind(1: string message);

  i32 mightFail(1: string message) throws (
    1: parent.ParentError parent_ex,
    2: ChildError child_ex,
  );

  SomeStruct doSomething(
    19: string message,
    2: SomeStruct input1,
    5: SomeStruct input2,
    3: AnEnum e,
    4: binary data,
  );
}
