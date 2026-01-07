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

@thrift.AllowLegacyMissingUris
package;

@cpp.PackIsset
struct Default {
  1: optional i32 field1;
  2: optional i32 field2;
  3: optional string field3;
  4: optional double field4;
}

@cpp.PackIsset{atomic = false}
struct NonAtomic {
  1: optional i32 field1;
  2: optional i32 field2;
  3: optional string field3;
  4: optional double field4;
}

@cpp.PackIsset{atomic = true}
struct Atomic {
  1: optional i32 field1;
  2: optional i32 field2;
  3: optional string field3;
  4: optional double field4;
}

@cpp.PackIsset{atomic = true}
struct AtomicFoo {
  1: optional i32 field1;
  2: optional i32 field2;
  3: optional string field3;
  4: optional double field4;
}
