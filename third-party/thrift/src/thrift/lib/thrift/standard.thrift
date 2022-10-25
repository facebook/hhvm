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

cpp_include "<folly/io/IOBuf.h>"
cpp_include "<folly/FBString.h>"

/** The **standard** types all Thrift implementations support. */
@thrift.v1alpha
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

// TODO(afuller): Allow 'void' type for union fields.
enum Void {
  NoValue = 0 (cpp.name = "NoValueVoid"),
}

/**
 * Typedef for binary data which can be represented as a string of 8-bit bytes
 *
 * Each language can map this type into a customized memory efficient object
 */
@java.Adapter{
  adapterClassName = "com.facebook.thrift.adapter.common.UnpooledByteBufTypeAdapter",
  typeClassName = "io.netty.buffer.ByteBuf",
}
typedef binary (cpp2.type = "folly::fbstring") ByteString

/**
 * Typedef for binary data
 *
 * Each language can map this type into a customized memory efficient object
 * May be used for zero-copy slice of data
 */
// TODO(afuller): Consider switching to std::unique_ptr<folly::IOBuf> for c++,
// to make moves cheaper (benchmark to see if this is better).
@java.Adapter{
  adapterClassName = "com.facebook.thrift.adapter.common.UnpooledByteBufTypeAdapter",
  typeClassName = "io.netty.buffer.ByteBuf",
}
typedef binary (cpp2.type = "folly::IOBuf") ByteBuffer

/**
 * A fixed-length span of time, represented as a signed count of seconds and
 * nanoseconds (nanos).
 *
 * Considered 'normal', when `nanos` is in the range 0 to 999'999'999
 * inclusive, or `seconds` is 0 and `nanos` is in the range -999'999'999 to
 * 999'999'999 inclusive.
 */
struct DurationStruct {
  /** The count of seconds. */
  1: i64 seconds;
  /** The count of nanoseconds. */
  // TODO(afuller): Fix to not require a default for terse fields.
  2: i32 nanos = 0;
} (thrift.uri = "facebook.com/thrift/type/Duration")

/**
 * An instant in time encoded as a count of seconds and nanoseconds (nanos)
 * since midnight on January 1, 1970 UTC (i.e. Unix epoch).
 *
 * Considered 'normal', when `nanos` is in the range 0 to 999'999'999 inclusive.
 */
// TODO(afuller): Consider making this a 'strong' typedef of `Duration`, which
// would ensure both a separate URI and native type in all languages.
struct TimeStruct {
  /** The count of seconds. */
  1: i64 seconds;
  /** The count of nanoseconds. */
  2: i32 nanos;
} (thrift.uri = "facebook.com/thrift/type/Time")

/**
 * A integer fraction of the form {numerator} / {denominator}
 *
 * Useful for representing ratios, rates, and metric accumulators.
 *
 * Considered 'normal' when the denominator is positive.
 * Considered 'simple' when `normal` and the greatest common divisor of the
 * and `numerator` and `denominator`, is 1.
 */
struct FractionStruct {
  /** The numerator/dividend/antecedent/upper integer. */
  1: i64 numerator;
  /** The denominator/divisor/consequent/lower integer. */
  2: i64 denominator;
} (thrift.uri = "facebook.com/thrift/type/Fraction")

/**
 * A 'normal' Fraction.
 *
 * This representation is always safe to 'normalize'.
 */
@thrift.Experimental // TODO(afuller): Adapt!
typedef FractionStruct Fraction (thrift.uri = "")

/**
 * A 'simple' Fraction.
 *
 * This representation is always safe to 'simplify'.
 */
@thrift.Experimental // TODO(afuller): Adapt!
typedef FractionStruct SimpleFraction

/**
 * The binary form of a universally unique identifier (UUID).
 *
 * Considered 'valid' if contains exactly 0 or 16 bytes.
 * Considered 'normal' if not all zeros.
 *
 * See rfc4122
 */
@thrift.Experimental // TODO(afuller): Adapt.
typedef binary Uuid

/**
 * The string form of a universally unique identifier (UUID).
 *
 * For example: "6ba7b810-9dad-11d1-80b4-00c04fd430c8". Use `standard.Uuid` for
 * a more compact representation.
 *
 * Considered 'normal', if not all zeros **and** 'urn:uuid:', {', '}' and
 * capital hex letters are not present.
 *
 * See rfc4122
 */
@thrift.Experimental // TODO(afuller): Adapt.
typedef string UuidString

/**
 * A slash('/')-delimitated path.
 */
@thrift.Experimental // TODO(afuller): Adapt.
typedef string Path

/**
 * Parsed path segments.
 *
 * Considered 'simple' if no segment contains a slash('/').
 */
@thrift.Experimental // TODO(afuller): Adapt.
typedef list<string> PathSegments

/**
 * A dot('.')-delimitated domain name.
 *
 * See rfc1034.
 */
@thrift.Experimental // TODO(afuller): Adapt.
typedef string Domain

/**
 * Parsed domain labels.
 *
 * Considered 'simple' if no label contains a dot('.').
 */
@thrift.Experimental // TODO(afuller): Adapt.
typedef list<string> DomainLabels

/**
 * A URI Query string.
 *
 * Of the form {name}={value}&...
 *
 * See rfc3986.
 */
@thrift.Experimental // TODO(afuller): Adapt.
typedef string QueryString

/**
 * A parsed QueryString.
 *
 * Considered 'simple' if no key or value contains a equal('=') or
 * ampersand('&').
 */
@thrift.Experimental // TODO(afuller): Adapt.
typedef map<string, string> QueryArgs

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

/**
 * The 'parsed' form of a `Uri`.
 *
 *   {scheme}://{domain}/{path}?{query}#{fragment}
 */
// TODO(afuller): Add adapters and native classes, e.g. BasicUri, Uri,
// and UriView, which use std::basic_string, std::string, std::string_view
// respectively.
@thrift.Experimental // TODO(afuller): Adapt.
struct UriStruct {
  // The scheme, if present.
  1: string scheme;

  // The parsed domain, for example "meta.com" -> ["meta", "com"]
  2: DomainLabels domain;

  // The parsed path, for example "path/to/file" -> ["path", "to", "file"]
  4: PathSegments path;

  // The parsed query args.
  5: QueryArgs query;

  // The fragment, if present.
  6: string fragment;
} (thrift.uri = "facebook.com/thrift/type/Uri")

// The uri of an IDL defined type.
union TypeUri {
  // The unique Thrift URI for this type.
  1: Uri uri;
  // A prefix of the SHA2-256 hash of the URI.
  2: ByteString typeHashPrefixSha2_256;
}

// Uniquely identifies a Thrift type.
union TypeName {
  // True(1) or False(0)
  1: Void boolType;

  // 8-bit signed integer
  2: Void byteType;

  // 16-bit signed integer
  3: Void i16Type;

  // 32-bit signed integer
  4: Void i32Type;

  // 64-bit signed integer
  5: Void i64Type;

  // 32-bit floating point
  6: Void floatType;

  // 64-bit floating point
  7: Void doubleType;

  // UTF-8 encoded string
  8: Void stringType;

  // Arbitrary byte string
  9: Void binaryType;

  // 32-bit signed integer, with named values.
  10: TypeUri enumType;

  17: TypeUri typedefType;
  11: TypeUri structType;
  12: TypeUri unionType;
  13: TypeUri exceptionType;
  14: Void listType;
  15: Void setType;
  16: Void mapType;
}

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
