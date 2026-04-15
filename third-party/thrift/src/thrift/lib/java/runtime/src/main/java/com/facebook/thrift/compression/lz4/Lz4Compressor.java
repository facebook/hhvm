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

package com.facebook.thrift.compression.lz4;

import com.facebook.thrift.compression.ThriftCompressor;
import io.netty.buffer.ByteBuf;
import io.netty.buffer.ByteBufAllocator;

/**
 * LZ4 compression placeholder. The real implementation using at.yawk.lz4:lz4-java will be added in
 * a follow-on diff with the varint size prefix wire format matching C++ LZ4_VARINT_SIZE.
 */
public final class Lz4Compressor implements ThriftCompressor {

  public static final Lz4Compressor INSTANCE = new Lz4Compressor();

  Lz4Compressor() {}

  @Override
  public ByteBuf compress(ByteBufAllocator allocator, ByteBuf data) {
    throw new UnsupportedOperationException("LZ4 compression is not yet implemented.");
  }

  @Override
  public ByteBuf decompress(ByteBufAllocator allocator, ByteBuf data) {
    throw new UnsupportedOperationException("LZ4 decompression is not yet implemented.");
  }
}
