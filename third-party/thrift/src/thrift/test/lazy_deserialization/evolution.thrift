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

include "thrift/annotation/cpp.thrift"

namespace cpp2 apache.thrift.test

struct StructWithLessFields {
  @cpp.Lazy
  1: list<string> field1;
  2: list<string> field2;
}

struct StructWithMoreFields {
  @cpp.Lazy
  1: list<string> field1;
  @cpp.Lazy
  2: list<string> field2;
  @cpp.Lazy
  3: list<string> field3;
}
