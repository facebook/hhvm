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

package com.facebook.thrift.rsocket.util;

import com.facebook.thrift.compression.CompressionManager;
import io.netty.buffer.ByteBuf;
import io.netty.buffer.ByteBufAllocator;
import io.rsocket.Payload;
import io.rsocket.util.ByteBufPayload;
import org.apache.thrift.CompressionAlgorithm;

public final class PayloadUtil {
  private PayloadUtil() {}

  /**
   * Creates a Payload, compressing data if a compression algorithm is specified. Takes ownership of
   * the data buffer (it will be released if compression occurs).
   */
  public static Payload createPayload(
      final ByteBufAllocator alloc,
      final CompressionAlgorithm algorithm,
      final ByteBuf data,
      final ByteBuf metadata) {
    ByteBuf payloadData = CompressionManager.compress(algorithm, alloc, data);
    return ByteBufPayload.create(payloadData, metadata);
  }

  public static Payload createPayload(final ByteBuf data, final ByteBuf metadata) {
    return ByteBufPayload.create(data, metadata);
  }

  /**
   * Extracts data from a Payload, decompressing if a compression algorithm is specified.
   *
   * <p>When decompression occurs, the returned ByteBuf is independently allocated and the caller is
   * responsible for releasing it. When no decompression occurs, the returned ByteBuf is a slice of
   * the payload's data (shared lifecycle).
   */
  public static ByteBuf getData(
      final ByteBufAllocator alloc, final CompressionAlgorithm algorithm, final Payload payload) {
    if (algorithm == null || algorithm == CompressionAlgorithm.NONE) {
      return payload.sliceData();
    }
    ByteBuf compressed = payload.sliceData();
    compressed.retain();
    return CompressionManager.decompress(algorithm, alloc, compressed);
  }
}
