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

include "thrift/annotation/scope.thrift"

package "facebook.com/thrift/annotation/go"

namespace java com.facebook.thrift.annotation.go_deprecated
namespace android com.facebook.thrift.annotation.go_deprecated
namespace js thrift.annotation.go
namespace py.asyncio facebook_thrift_asyncio.annotation.go
namespace go thrift.annotation.go
namespace py thrift.annotation.go

// Annotation for overriding Go names (e.g. to avoid codegen name conflicts).
@scope.Field
@scope.Function
@scope.Typedef
struct Name {
  1: string name;
}

// Annotation for overriding Go struct field tags (e.g. json, yaml, serf tags).
@scope.Field
struct Tag {
  1: string tag;
}

// Annotation for declaring a typedef as a new type (rather than an alias).
//
//  type T1 = T2  // alias declaration
//  type T1 T2    // type definition
@scope.Typedef
struct NewType {}
