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

union Inner {
  1: map<byte, Inner> mapField;
  2: set<string> setField;
  3: list<i16> listField;
  4: bool boolField;
}

enum E {
  A = 0,
  B = 1,
}

struct Outer {
  1: optional double floatField;
  2: Inner unionField;
  3: E enumField;
}
