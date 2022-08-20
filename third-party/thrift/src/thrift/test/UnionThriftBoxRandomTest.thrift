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
include "thrift/annotation/thrift.thrift"

namespace cpp2 apache.thrift.test

union Ref {
  @cpp.Ref{type = cpp.RefType.Unique}
  1: string field_1;

  @cpp.Ref{type = cpp.RefType.SharedMutable}
  2: string field_2;

  @cpp.Ref{type = cpp.RefType.Shared}
  3: string field_3;

  @thrift.Box
  4: string field_4;
}

union NonRef {
  1: string field_1;
  2: string field_2;
  3: string field_3;
  4: string field_4;
}
