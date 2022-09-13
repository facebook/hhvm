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

package "facebook.com/thrift/annotation/java"

namespace java com.facebook.thrift.annotation.java_deprecated
namespace py.asyncio facebook_thrift_asyncio.annotation.java
namespace go thrift.annotation.java
namespace py thrift.annotation.java

// An annotation that is applied to a Typedef or field that maps it a Java type Adapter.
// For example:
// @java.Adapter{adapterClassName="com.facebook.thrift.TimestampAdapter", typeClassName="java.time.Instant"}
// typedef i64 Timestamp
//
@scope.Field
@scope.Typedef
@scope.Structured
struct Adapter {
  // Fully qualified name to a class that implements com.facebook.thrift.adaptor.TypeAdapter
  1: string adapterClassName;

  // Fully qualified name the above implmenantion adapts too
  2: string typeClassName;
} (thrift.uri = "facebook.com/thrift/annotation/java/Adapter")
