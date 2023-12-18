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

// Please consult with Thrift team before using any of these structured annotations.

include "thrift/annotation/scope.thrift"
include "thrift/annotation/thrift.thrift"

package "facebook.com/thrift/annotation"

namespace java com.facebook.thrift.annotation_deprecated
namespace android com.facebook.thrift.annotation_deprecated
namespace py.asyncio facebook_thrift_asyncio.annotation.internal
namespace go thrift.annotation.internals
namespace py thrift.annotation.internal

// An annotation that injects fields from `type` to the struct that it annotates.
// Note, we only allow injecting fields from a struct type. For example:
//
//   struct Fields {
//     1: i32 metadata;
//   }
//
//   @internal.InjectMetadataFields{type = "Fields"}
//   struct StructWithInjectedFields {
//     1: i32 field1;
//   }
//
// is equivalent to
//
//   struct StructWithInjectedFields {
//     -1000: i32 metadata;
//     1: i32 field1;
//   }
@scope.Struct
@thrift.Experimental
struct InjectMetadataFields {
  1: string type;
} (thrift.uri = "facebook.com/thrift/annotation/InjectMetadataFields")
