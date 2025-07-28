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

include "thrift/lib/thrift/any_rep.thrift"
include "thrift/annotation/thrift.thrift"

/** The **standard** representations for Thrift: Any. */
package "facebook.com/thrift/type"

namespace cpp2 apache.thrift.type
namespace py3 apache.thrift.type
namespace php apache_thrift_type
namespace java.swift com.facebook.thrift.type_swift
namespace py.asyncio apache_thrift_asyncio.any
namespace go thrift.lib.thrift.any
namespace py thrift.lib.thrift.any

/** A type that can hold 'any' value (including void).
 *
 * Always contains enough information to deserialize the stored value,  if the
 * type/protocol are recognized.
 */
@thrift.DeprecatedUnvalidatedAnnotations{
  items = {"thrift.patch.uri": "facebook.com/thrift/op/AnyPatch"},
}
@thrift.Uri{value = ""}
typedef any_rep.AnyStruct Any

/** A type that can hold any subset of 'any' value.
 *
 * Unlike `Any`, `SemiAny` may not hold enough information to deserialized the
 * stored value.
 */
@thrift.Uri{value = ""}
typedef any_rep.SemiAnyStruct SemiAny

/** A list of SemiAny values, accessible by ValueId. */
typedef list<Any> AnyValueList

/** A list of SemiAny values, accessible by ValueId. */
typedef list<SemiAny> SemiAnyValueList
