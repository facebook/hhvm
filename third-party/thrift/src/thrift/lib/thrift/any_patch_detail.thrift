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

include "thrift/annotation/python.thrift"
include "thrift/annotation/thrift.thrift"
include "thrift/lib/thrift/any_rep.thrift"
include "thrift/lib/thrift/type.thrift"

namespace cpp2 apache.thrift.op
namespace py3 apache.thrift.op
namespace java.swift com.facebook.thrift.op
namespace js apache.thrift.op
namespace py.asyncio apache_thrift_asyncio.any_patch_detail
namespace go thrift.lib.thrift.any_patch_detail
namespace py thrift.lib.thrift.any_patch_detail

@thrift.TerseWrite
@thrift.Experimental
package "facebook.com/thrift/op"

// Since Thrift discourages structured keys, we leverage 'list<TypeToPatchInternalDoNotUse>'
// to represent on wire for 'map<type.Type, list<any_rep.AnyStruct>>'.
struct TypeToPatchInternalDoNotUse {
  @python.Py3Hidden
  1: type.Type type;
  2: list<any_rep.AnyStruct> patches;
}
