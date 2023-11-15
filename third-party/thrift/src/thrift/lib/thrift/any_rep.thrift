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
include "thrift/lib/thrift/type.thrift"
include "thrift/lib/thrift/standard.thrift"
include "thrift/annotation/python.thrift"

/**
 * The **underlying representations** for Thrift: Any.
 *
 * The following definitions are provided as unadapted underlying
 * representations for 'public' adapted typedefs defined in 'any.thrift'.
 *
 * These definitions are named after their representations, using the form
 * '{name}{Type}. For example, for a 'public' exception `Foo`, the underlying
 * type would be `exception FooException`.
 */
package "facebook.com/thrift/type"

namespace cpp2 apache.thrift.type
namespace py3 apache.thrift.type
namespace php apache_thrift_type
namespace java.swift com.facebook.thrift.type_swift
namespace py.asyncio apache_thrift_asyncio.any_rep
namespace go thrift.lib.thrift.any_rep
namespace py thrift.lib.thrift.any_rep

/** A struct that can hold any thrift supported value, encoded in any format. */
struct AnyStruct {
  /**
   * The type stored in `data`.
   *
   * Must not be empty.
   */
  @python.Py3Hidden
  1: type.Type type;

  /**
   * The protocol used to encode `data`.
   *
   * Must not be empty.
   */
  @python.Py3Hidden
  2: type.Protocol protocol;

  /** The encoded data. */
  3: standard.ByteBuffer data;
} (thrift.uri = "facebook.com/thrift/type/Any", rust.ord)

/**
 * Like Any, except all fields are mutable and can be empty.
 *
 * Can be upgraded to an Any after all the field are populated.
 */
struct SemiAnyStruct {
  /** The type stored in `data`, if known. */
  @python.Py3Hidden
  1: type.Type type;

  /** The protocol used to encode `data`, if known. */
  @python.Py3Hidden
  2: type.Protocol protocol;

  /** The encoded data. */
  3: standard.ByteBuffer data;
} (thrift.uri = "facebook.com/thrift/type/SemiAny")
