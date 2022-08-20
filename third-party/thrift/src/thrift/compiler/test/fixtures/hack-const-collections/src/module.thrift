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

struct Foo {
  1: list<string> a;
  2: optional map<string, list<set<i32>>> b;
  3: i64 c = 7;
  4: optional bool d = 0;
}

service Bar {
  string baz(
    1: set<i32> a,
    2: list<map<i32, set<string>>> b,
    3: optional i64 c,
    4: Foo d,
    5: i64 e = 4,
  );
}

exception Baz {
  1: string message;
}
