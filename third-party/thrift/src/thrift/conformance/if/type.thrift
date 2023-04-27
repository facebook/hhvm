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

namespace cpp2 apache.thrift.conformance
namespace php apache_thrift
namespace py thrift.conformance.type
namespace py.asyncio thrift_asyncio.conformance.type
namespace py3 thrift.conformance
namespace java.swift org.apache.thrift.conformance
namespace go thrift.conformance.type

// Language-independent type information.
struct ThriftTypeInfo {
  // The URI of the type. For example: "facebook.com/thrift/ThriftTypeInfo"
  //
  // The scheme "fbthrift://" is implied, and should not be included.
  1: string uri;

  // The other URIs for this type.
  //
  // The primary URI can be safely changed using the following steps:
  // 1. Add the new URI to altUris.
  // 2. Update all binaries/libraries, so they know about the new
  // URI.
  // 3. Move the old URI to altUris, and the new URI to uri.
  //
  // At this point all references to the old URI will continue to work, and
  // all knew references will use the new URI.
  2: set<string> altUris;

  // The default number of bytes to use in a type hash.
  //
  // 0 indicates a type hash should never be used.
  // Unset indicates that the implementation should decide.
  3: optional byte typeHashBytes;
} (thrift.uri = "facebook.com/thrift/ThriftTypeInfo")
