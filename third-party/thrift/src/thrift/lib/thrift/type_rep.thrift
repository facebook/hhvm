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
include "thrift/annotation/rust.thrift"

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
@thrift.Uri{value = "facebook.com/thrift/type/Protocol"}
@rust.Ord
union ProtocolUnion {
  /** A standard protocol, known by all Thrift implementations. */
  1: standard.StandardProtocol standard;

  /** A custom protocol. */
  2: string custom;

  /** An externally stored protocol. */
  @python.Py3Hidden
  3: id.ProtocolId id;

  /**
   * Composition of a protocol + compression.
   */
  @python.Py3Hidden
  @thrift.Box
  4: CompressedProtocolStruct compressed;
}

/** A concrete Thrift type. */
@rust.Ord
@thrift.Uri{value = "facebook.com/thrift/type/Type"}
struct TypeStruct {
  /** The type name. */
  1: standard.TypeName name;
  /** The type params, if appropriate. */
  2: list<TypeStruct> params;
}

@thrift.Uri{value = "facebook.com/thrift/type/CompressedProtocol"}
@rust.Ord
struct CompressedProtocolStruct {
  /** The protocol used to obtain the data that was compressed. */
  1: ProtocolUnion protocol;

  /**
   * Information needed to decompress the data (in order to further deserialize
   * using the `protocol` above.
   */
  2: CompressionSpecStruct compression;
}

@thrift.Uri{value = "facebook.com/thrift/type/CompressionSpec"}
struct CompressionSpecStruct {
  /**
   * An identifier for the algorithm used to compress the serialized Thrift
   * value into `data`. The specific value of this id is domain-specific: each
   * application domain (i.e., codebase or runtime environment) can map values
   * to algorithms of their choice, except that value 0 is reserved and MUST NOT
   * be used to identify an algorithm.
   *
   * Facebook Internal:
   * For the set of ids (and their corresponding algorithms and expected data
   * format) available in Meta codebases, see:
   * https://fburl.com/thrift_any_compression_ids
   */
  1: byte id;

  /**
   * The size, in bytes, of the uncompressed `data`.
   *
   *
   * This field:
   *   1. MAY be required by the (de)compression algorithm corresponding to
   *      `id`.
   *   2. MAY be specified, even if not required for decompression. For example,
   *      this could be used to pre-allocate buffers, impose limits or skip
   *      decompression altogether.
   *   3. MUST NOT be negative.
   */
  2: optional i64 uncompressed_data_size_bytes;

  /**
   * Compression algorithm (i.e., `id`)-specific parameters that may be required
   * to obtain the compressed `data`.
   *
   * For example, such parameters may include the "compression level" used by
   * the data producer (e.g., for Zlib:
   * https://en.wikipedia.org/wiki/Zlib#Resource_use).
   *
   * This field MUST NOT be required for decompression: information required
   * for decompressing `data` must either be present in the data itself or in
   * other properly identified fields (eg. `uncompressed_data_size_bytes`).
   * By extension, clearing this field MUST NOT prevent `data` from being
   * decompressed.
   *
   * This information may be used by "intermediate processors" that wish to
   * decompress, mutate and then re-compress data using identical settings as
   * the initial compression.
   */
  3: optional binary compression_parameters;
}
