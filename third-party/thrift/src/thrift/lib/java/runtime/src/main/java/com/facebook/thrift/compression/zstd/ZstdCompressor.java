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
import com.github.luben.zstd.Zstd;
import io.netty.buffer.ByteBuf;
import io.netty.buffer.ByteBufAllocator;
import io.netty.util.ReferenceCountUtil;
import java.nio.ByteBuffer;

/**
 * ZSTD compression using zstd-jni. Uses standard ZSTD frame format which is self-describing
 * (includes content size in frame header), matching C++ folly::compression::CodecType::ZSTD.
 *
 * <p>Uses direct ByteBuffer APIs with {@link ByteBuf#nioBuffer()} for zero-copy when the underlying
 * buffer is direct. Heap-backed buffers are copied to a temporary direct ByteBuffer since
 * zstd-jni's ByteBuffer API requires direct buffers.
 */
public final class ZstdCompressor implements ThriftCompressor {

  public static final ZstdCompressor INSTANCE = new ZstdCompressor();

  ZstdCompressor() {}

  @Override
  public ByteBuf compress(ByteBufAllocator allocator, ByteBuf data) {
    try {
      int readableBytes = data.readableBytes();
      long maxCompressedSize = Zstd.compressBound(readableBytes);
      if (Zstd.isError(maxCompressedSize) || maxCompressedSize > Integer.MAX_VALUE) {
        throw new IllegalArgumentException(
            "ZSTD compressBound failed for input size: " + readableBytes);
      }
      ByteBuffer source = toDirectByteBuffer(data);
      ByteBuf output = allocator.directBuffer((int) maxCompressedSize);
      try {
        ByteBuffer destination = output.nioBuffer(0, (int) maxCompressedSize);
        long compressedSize = Zstd.compress(destination, source, Zstd.defaultCompressionLevel());
        if (Zstd.isError(compressedSize)) {
          throw new RuntimeException(
              "ZSTD compression failed: " + Zstd.getErrorName(compressedSize));
        }
        output.writerIndex((int) compressedSize);
        return output;
      } catch (Exception exception) {
        output.release();
        throw exception;
      }
    } finally {
      ReferenceCountUtil.safeRelease(data);
    }
  }

  @Override
  public ByteBuf decompress(ByteBufAllocator allocator, ByteBuf data) {
    try {
      ByteBuffer source = toDirectByteBuffer(data);
      long decompressedSize =
          Zstd.getDirectByteBufferFrameContentSize(source, source.position(), source.remaining());
      if (decompressedSize < 0) {
        throw new IllegalStateException(
            "Cannot determine ZSTD decompressed size from frame header");
      }
      if (decompressedSize > Integer.MAX_VALUE) {
        throw new IllegalStateException(
            "ZSTD decompressed size exceeds maximum: " + decompressedSize);
      }
      ByteBuf output = allocator.directBuffer((int) decompressedSize);
      try {
        ByteBuffer destination = output.nioBuffer(0, (int) decompressedSize);
        long result = Zstd.decompress(destination, source);
        if (Zstd.isError(result)) {
          throw new RuntimeException("ZSTD decompression failed: " + Zstd.getErrorName(result));
        }
        output.writerIndex((int) result);
        return output;
      } catch (Exception exception) {
        output.release();
        throw exception;
      }
    } finally {
      ReferenceCountUtil.safeRelease(data);
    }
  }

  /**
   * Returns a direct ByteBuffer view of the readable bytes. If the ByteBuf is already direct,
   * returns a zero-copy NIO buffer view. Otherwise copies to a temporary direct ByteBuffer since
   * zstd-jni's ByteBuffer API requires direct buffers.
   */
  private static ByteBuffer toDirectByteBuffer(ByteBuf data) {
    ByteBuffer nioBuffer = data.nioBuffer();
    if (nioBuffer.isDirect()) {
      return nioBuffer;
    }
    ByteBuffer direct = ByteBuffer.allocateDirect(nioBuffer.remaining());
    direct.put(nioBuffer);
    direct.flip();
    return direct;
  }
}
