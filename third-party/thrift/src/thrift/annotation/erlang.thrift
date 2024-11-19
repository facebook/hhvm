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

package "meta.com/thrift/annotation/erlang"

namespace erl ""
namespace py.asyncio meta_thrift_asyncio.annotation.erlang
namespace go thrift.annotation.erlang
namespace py thrift.annotation.erlang

/**
 * Override the name of the generated struct record or enum value. The name will be
 * an Erlang atom.
 **/
@scope.Definition
struct NameOverride {
  1: string name;
}

/**
 * Choose the default value for an enum. The string must match one of the enum
 * identifiers.
 **/
@scope.Enum
struct DefaultValue {
  1: string value;
}

/**
 * Allows you to override whether we use Erlang records or maps to represent Thrift structs.
 * If you apply this to a service, then it will choose which representation to use for
 * arguments and return values of your function implementations.
 **/
enum StructReprType {
  RECORD = 1,
  MAP = 2,
}

@scope.Struct
@scope.Service
struct StructRepr {
  1: StructReprType repr;
}

/**
 * Override the node type used for IQ parsing.
 **/
enum IqNodeType {
  XMLNODE = 1,
  XMLCDATA = 2,
  XMLATTRIBUTE = 3,
}

@scope.Field
struct Iq {
  1: IqNodeType node_type;
}

/**
 * Skip decoding a field if it exists
 **/
@scope.Field
struct SkipDecode {}
