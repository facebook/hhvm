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

package com.facebook.thrift.compression;

import com.facebook.thrift.compression.lz4.Lz4Compressor;
import com.facebook.thrift.compression.zlib.ZlibCompressor;
import com.facebook.thrift.compression.zstd.ZstdCompressor;
import io.netty.buffer.ByteBuf;
import io.netty.buffer.ByteBufAllocator;
import org.apache.thrift.CompressionAlgorithm;
import org.apache.thrift.TTransform;

/**
 * Central registry that maps THeader transform IDs and CompressionAlgorithm values to {@link
 * ThriftCompressor} instances. Mirrors C++ CompressionManager + CompressionAlgorithmSelector.
 *
 * <p>TTransform IDs are the wire-format values from THeader (defined in RpcMetadata.thrift).
 * CompressionAlgorithm values are the Rocket metadata values (different numbering).
 */
public final class CompressionManager {

  private CompressionManager() {}

  /**
   * Maps a TTransform wire ID to a CompressionAlgorithm. Mirrors C++
   * CompressionAlgorithmSelector::fromTTransform().
   */
  public static CompressionAlgorithm fromTTransform(int transformId) {
    TTransform transform = TTransform.fromInteger(transformId);
    if (transform == null) {
      throw new IllegalArgumentException("Unknown TTransform ID: " + transformId);
    }
    switch (transform) {
      case NONE:
        return CompressionAlgorithm.NONE;
      case ZLIB:
        return CompressionAlgorithm.ZLIB;
      case ZSTD:
        return CompressionAlgorithm.ZSTD;
      case LZ4:
        return CompressionAlgorithm.LZ4;
      case CUSTOM:
        return CompressionAlgorithm.CUSTOM;
      case ZLIB_LESS:
        return CompressionAlgorithm.ZLIB_LESS;
      case ZSTD_LESS:
        return CompressionAlgorithm.ZSTD_LESS;
      case LZ4_LESS:
        return CompressionAlgorithm.LZ4_LESS;
      case ZLIB_MORE:
        return CompressionAlgorithm.ZLIB_MORE;
      case ZSTD_MORE:
        return CompressionAlgorithm.ZSTD_MORE;
      case LZ4_MORE:
        return CompressionAlgorithm.LZ4_MORE;
      default:
        throw new IllegalArgumentException("Unknown TTransform: " + transform);
    }
  }

  /**
   * Returns the compressor for a given CompressionAlgorithm. For decompression, the level variant
   * (LESS/MORE) doesn't matter — all ZSTD variants use the same decompressor, etc.
   */
  public static ThriftCompressor getCompressor(CompressionAlgorithm algorithm) {
    switch (algorithm) {
      case ZLIB:
      case ZLIB_LESS:
      case ZLIB_MORE:
        return ZlibCompressor.INSTANCE;
      case ZSTD:
      case ZSTD_LESS:
      case ZSTD_MORE:
        return ZstdCompressor.INSTANCE;
      case LZ4:
      case LZ4_LESS:
      case LZ4_MORE:
        return Lz4Compressor.INSTANCE;
      case NONE:
        throw new IllegalArgumentException("Cannot get compressor for NONE");
      case CUSTOM:
        throw new UnsupportedOperationException("CUSTOM compression is not supported");
      default:
        throw new IllegalArgumentException("Unknown compression algorithm: " + algorithm);
    }
  }

  /**
   * Decompresses data using the algorithm indicated by the THeader transform ID. Takes ownership of
   * the input buffer.
   *
   * @param transformId the TTransform wire ID from the THeader frame
   * @param allocator the allocator for the output buffer
   * @param data the compressed data; ownership is transferred
   * @return the decompressed data; caller owns and must release
   */
  public static ByteBuf decompressFromTransform(
      int transformId, ByteBufAllocator allocator, ByteBuf data) {
    CompressionAlgorithm algorithm = fromTTransform(transformId);
    if (algorithm == CompressionAlgorithm.NONE) {
      return data;
    }
    return getCompressor(algorithm).decompress(allocator, data);
  }

  /**
   * Decompresses data using the given CompressionAlgorithm. Takes ownership of the input buffer.
   *
   * @param algorithm the compression algorithm
   * @param allocator the allocator for the output buffer
   * @param data the compressed data; ownership is transferred
   * @return the decompressed data; caller owns and must release
   */
  public static ByteBuf decompress(
      CompressionAlgorithm algorithm, ByteBufAllocator allocator, ByteBuf data) {
    if (algorithm == null || algorithm == CompressionAlgorithm.NONE) {
      return data;
    }
    return getCompressor(algorithm).decompress(allocator, data);
  }

  /**
   * Compresses data using the given CompressionAlgorithm. Takes ownership of the input buffer.
   *
   * @param algorithm the compression algorithm
   * @param allocator the allocator for the output buffer
   * @param data the data to compress; ownership is transferred
   * @return the compressed data; caller owns and must release
   */
  public static ByteBuf compress(
      CompressionAlgorithm algorithm, ByteBufAllocator allocator, ByteBuf data) {
    if (algorithm == null || algorithm == CompressionAlgorithm.NONE) {
      return data;
    }
    return getCompressor(algorithm).compress(allocator, data);
  }
}
