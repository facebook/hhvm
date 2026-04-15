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

package com.facebook.thrift.compression.zstd;

import com.facebook.thrift.compression.ThriftCompressor;
import io.netty.buffer.ByteBuf;
import io.netty.buffer.ByteBufAllocator;

/**
 * ZSTD compression placeholder. The real implementation using zstd-jni will be added in a follow-on
 * diff with direct ByteBuffer APIs for zero-copy compression.
 */
public final class ZstdCompressor implements ThriftCompressor {

  public static final ZstdCompressor INSTANCE = new ZstdCompressor();

  ZstdCompressor() {}

  @Override
  public ByteBuf compress(ByteBufAllocator allocator, ByteBuf data) {
    throw new UnsupportedOperationException("ZSTD compression is not yet implemented.");
  }

  @Override
  public ByteBuf decompress(ByteBufAllocator allocator, ByteBuf data) {
    throw new UnsupportedOperationException("ZSTD decompression is not yet implemented.");
  }
}
