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

service First {
  i32 one();
  i32 two();
}

service Second {
  i32 three();
  i32 four();
}

service Third {
  i32 five();
  i32 six();
}

service Conflicts {
  i32 four();
  i32 five();
}

interaction Thing1 {
  void foo();
}
service Interaction1 {
  performs Thing1;
}

interaction Thing2 {
  void bar();
}
service Interaction2 {
  performs Thing2;
}

struct SomeStruct {}
service SomeService extends Third {
  SomeStruct foo();
}
