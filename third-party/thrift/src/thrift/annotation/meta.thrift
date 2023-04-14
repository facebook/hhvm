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

// This file defines meta-annotations targeted at Thrift framework authors
// rather than general Thrift users.

include "thrift/annotation/scope.thrift"
include "thrift/annotation/thrift.thrift"

package "facebook.com/thrift/annotation/deprecated"

namespace java com.facebook.thrift.annotation_deprecated.deprecated
namespace py.asyncio facebook_thrift_asyncio.annotation.meta
namespace go thrift.annotation.meta
namespace py thrift.annotation.meta

// Calls t_struct::set_generated().
//
// Useful for testing.
// TODO(afuller): Move to internal.thrift.
@scope.Struct
@thrift.Experimental
struct SetGenerated {} (
  thrift.uri = "facebook.com/thrift/annotation/SetGenerated",
)

// TODO(afuller): Delete.
@scope.Struct
@thrift.Deprecated
struct Transitive {} (thrift.uri = "facebook.com/thrift/annotation/Transitive")
