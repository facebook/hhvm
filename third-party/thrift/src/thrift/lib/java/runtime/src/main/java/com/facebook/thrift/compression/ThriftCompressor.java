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

import io.netty.buffer.ByteBuf;
import io.netty.buffer.ByteBufAllocator;

/**
 * Interface for Thrift payload compression and decompression.
 *
 * <p>Implementations must be thread-safe and stateless. Both methods take ownership of the input
 * buffer (releasing it) and return a new buffer owned by the caller.
 */
public interface ThriftCompressor {

  /**
   * Compresses the given data.
   *
   * @param allocator the allocator to use for the output buffer
   * @param data the data to compress; ownership is transferred (this method releases it)
   * @return a new buffer containing the compressed data; caller owns and must release
   */
  ByteBuf compress(ByteBufAllocator allocator, ByteBuf data);

  /**
   * Decompresses the given data.
   *
   * @param allocator the allocator to use for the output buffer
   * @param data the compressed data; ownership is transferred (this method releases it)
   * @return a new buffer containing the decompressed data; caller owns and must release
   */
  ByteBuf decompress(ByteBufAllocator allocator, ByteBuf data);
}
