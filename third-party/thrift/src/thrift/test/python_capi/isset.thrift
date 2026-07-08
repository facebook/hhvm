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

// Structs in this library are annotated with
// `@python.EnableUnsafeIssetInspection`, so their thrift-python data holder keeps
// the deprecated isset byte array. This exercises the isset-enabled capi
// marshalling path - in particular the C++ -> Python Constructor - which the
// (isset-disabled) structs elsewhere in this suite do not cover.

include "thrift/annotation/python.thrift"

package "thrift.org/test/python_capi"

@python.EnableUnsafeIssetInspection
struct IssetStruct {
  1: i32 unqualified_int;
  2: optional i32 optional_int;
  3: optional string optional_str;
}

@python.EnableUnsafeIssetInspection
struct IssetEmpty {}
