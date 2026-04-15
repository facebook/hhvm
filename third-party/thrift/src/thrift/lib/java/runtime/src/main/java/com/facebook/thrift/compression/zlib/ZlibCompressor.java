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

package com.facebook.thrift.compression.zlib;

import com.facebook.thrift.compression.ThriftCompressor;
import io.netty.buffer.ByteBuf;
import io.netty.buffer.ByteBufAllocator;

/**
 * ZLIB compression placeholder. The real implementation using JDK Deflater/Inflater will be added
 * in a follow-on diff with thread-local caching and Java 11 multi-release ByteBuffer I/O.
 */
public final class ZlibCompressor implements ThriftCompressor {

  public static final ZlibCompressor INSTANCE = new ZlibCompressor();

  ZlibCompressor() {}

  @Override
  public ByteBuf compress(ByteBufAllocator allocator, ByteBuf data) {
    throw new UnsupportedOperationException("ZLIB compression is not yet implemented.");
  }

  @Override
  public ByteBuf decompress(ByteBufAllocator allocator, ByteBuf data) {
    throw new UnsupportedOperationException("ZLIB decompression is not yet implemented.");
  }
}
