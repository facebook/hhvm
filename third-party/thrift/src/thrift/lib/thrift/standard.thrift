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
include "thrift/annotation/java.thrift"
include "thrift/annotation/cpp.thrift"

cpp_include "<folly/io/IOBuf.h>"
cpp_include "<folly/FBString.h>"

/** The **standard** types all Thrift implementations support. */
@thrift.Experimental
package "facebook.com/thrift/type"

namespace cpp2 apache.thrift.type
namespace py3 apache.thrift.type
namespace php apache_thrift_type_standard
namespace java com.facebook.thrift.type
namespace java.swift com.facebook.thrift.standard_type
namespace js apache.thrift.type
namespace py.asyncio apache_thrift_asyncio.standard
namespace go thrift.lib.thrift.standard
namespace py thrift.lib.thrift.standard

enum Void {
  Unused = 0,
}

/**
 * Typedef for binary data which can be represented as a string of 8-bit bytes.
 *
 * Each language can map this type into a customized memory efficient object.
 */
@cpp.Type{name = "folly::fbstring"}
@java.Adapter{
  adapterClassName = "com.facebook.thrift.adapter.common.UnpooledByteBufTypeAdapter",
  typeClassName = "io.netty.buffer.ByteBuf",
}
typedef binary ByteString

/**
 * Typedef for binary data.
 *
 * Each language can map this type into a customized memory efficient object.
 * May be used for zero-copy slice of data.
 */
@cpp.Type{name = "folly::IOBuf"}
@java.Adapter{
  adapterClassName = "com.facebook.thrift.adapter.common.UnpooledByteBufTypeAdapter",
  typeClassName = "io.netty.buffer.ByteBuf",
}
typedef binary ByteBuffer

/**
 * A (scheme-less) URI.
 *
 * Of the form described in RFC 3986, but with every component optional.
 *
 * See rfc3986
 */
// TODO(afuller): Add definition for 'normal' based on unicode + uri specs.
@thrift.Experimental // TODO(afuller): Adapt.
typedef string Uri (thrift.uri = "")

/** The uri of an IDL defined type. */
union TypeUri {
  /** The unique Thrift URI for this type. */
  1: Uri uri;

  /**
   * A prefix of the SHA2-256 hash of the universal name. It is ByteString
   * instead of binary to fit a 16-byte prefix into the inline storage making
   * use of the small string optimization (SSO). In libstdc++ std::string SSO
   * is limited to 15 bytes and would require an allocation.
   */
  2: ByteString typeHashPrefixSha2_256;

  /**
   * The (potentially not unique) scoped name of this type.
   * Format is `filename.typename`, e.g. `standard.TypeUri`.
   * This is a fallback for types that do not have URIs yet.
   * Must be prepared for the active field to switch to `uri` as package statements are rolled out!
   */
  3: string scopedName;
}

/** Uniquely identifies a Thrift type. */
union TypeName {
  /** True(1) or False(0) */
  1: Void boolType;

  /** 8-bit signed integer */
  2: Void byteType;

  /** 16-bit signed integer */
  3: Void i16Type;

  /** 32-bit signed integer */
  4: Void i32Type;

  /** 64-bit signed integer */
  5: Void i64Type;

  /** 32-bit floating point */
  6: Void floatType;

  /** 64-bit floating point */
  7: Void doubleType;

  /** UTF-8 encoded string */
  8: Void stringType;

  /** Arbitrary byte string */
  9: Void binaryType;

  /** 32-bit signed integer, with named values. */
  10: TypeUri enumType;

  /** `typedef` definition */
  17: TypeUri typedefType;
  /** `struct` definition */
  11: TypeUri structType;
  /** `union` definition */
  12: TypeUri unionType;
  /** `exception` definition */
  13: TypeUri exceptionType;

  /** `list<V>` definition */
  14: Void listType;
  /** `set<K>` definition */
  15: Void setType;
  /** `map<K, V>` definition */
  16: Void mapType;
} (rust.ord)

/** The standard Thrift protocols. */
enum StandardProtocol {
  Custom = 0,

  // Standard protocols.
  Binary = 1,
  Compact = 2,

  // Deprecated protocols.
  Json = 3,
  SimpleJson = 4,
}
