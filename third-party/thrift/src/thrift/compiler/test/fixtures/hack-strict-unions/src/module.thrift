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

include "thrift/annotation/hack.thrift"
include "thrift/annotation/thrift.thrift"

@thrift.AllowLegacyMissingUris
package;

@hack.MigrationBlockingAllowInheritance
@hack.MigrationBlockingLegacyJSONSerialization
union Primitive {
  @hack.Adapter{name = '\\TimestampToTimeAdapter'}
  1: i64 i64_;
  2: string string_;
  5: float float5_ = 20;
  4: float float4_ = 30;
  3: float float3_ = 50;
}

union OtherPrimitive {
  @hack.Adapter{name = '\\TimestampToTimeAdapter'}
  1: i64 i64_;
  2: string string_;
  5: float float5_ = 20;
  4: float float4_ = 30;
  3: float float3_ = 50;
}

// for testing rollout mechanism
union Primitive1 {
  1: i64 i64_;
}

union Primitive2 {
  1: i64 i64_;
}

union Primitive3 {
  1: i64 i64_;
}

union Primitive4 {
  1: i64 i64_;
}
