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

include "thrift/annotation/thrift.thrift"
include "thrift/lib/thrift/id.thrift"
include "thrift/lib/thrift/standard.thrift"
include "thrift/annotation/python.thrift"

/**
 * The **underlying representations** for well-known Thrift types.
 *
 * The following definitions are provided as unadapted underlying
 * representations for 'public' adapted typedefs defined in 'type.thrift'.
 *
 * These definitions are named after their representations, using the form
 * '{name}{Type}. For example, for a 'public' exception `Foo`, the underlying
 * type would be `exception FooException`.
 */
@thrift.Experimental
@thrift.TerseWrite
package "facebook.com/thrift/type"

namespace cpp2 apache.thrift.type
namespace py3 apache.thrift.type
namespace php apache_thrift_type_rep
namespace java.swift com.facebook.thrift.type_swift
namespace js apache.thrift.type
namespace py.asyncio apache_thrift_asyncio.type_rep
namespace go thrift.lib.thrift.type_rep
namespace py thrift.lib.thrift.type_rep

/** A union representation of a protocol. */
union ProtocolUnion {
  /** A standard protocol, known by all Thrift implementations. */
  1: standard.StandardProtocol standard;
  /** A custom protocol. */
  2: string custom;
  /** An externally stored protocol. */
  @python.Py3Hidden
  3: id.ProtocolId id;
} (thrift.uri = "facebook.com/thrift/type/Protocol")

/** A concrete Thrift type. */
struct TypeStruct {
  /** The type name. */
  1: standard.TypeName name;
  /** The type params, if appropriate. */
  2: list<TypeStruct> params;
} (thrift.uri = "facebook.com/thrift/type/Type", rust.ord)
