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

package "test.dev/fixtures/python_capi"

include "thrift/annotation/python.thrift"

@python.UseCAPI{serialize = true}
struct SerializedStruct {
  1: string s;
  2: i32 i;
  3: optional string os;
  4: required string rs;
}

@python.UseCAPI{serialize = true}
union SerializedUnion {
  1: string s;
  2: i32 i;
}

@python.UseCAPI{serialize = true}
safe exception SerializedError {
  1: string msg;
  2: optional string os;
  3: required string rs;
}

@python.UseCAPI{}
struct MarshalStruct {
  1: string s;
  2: i32 i;
  3: optional string os;
  4: required string rs;
}

@python.UseCAPI{}
union MarshalUnion {
  1: string s;
  2: i32 i;
}

@python.UseCAPI{}
safe exception MarshalError {
  1: string msg;
  2: optional string os;
  3: required string rs;
}
