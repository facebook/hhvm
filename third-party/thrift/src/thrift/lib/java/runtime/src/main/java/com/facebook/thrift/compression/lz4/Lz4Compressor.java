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
import io.netty.util.ReferenceCountUtil;
import java.nio.ByteBuffer;
import net.jpountz.lz4.LZ4Factory;
import net.jpountz.lz4.LZ4FastDecompressor;

/**
 * LZ4 compression matching C++ folly::compression::CodecType::LZ4_VARINT_SIZE wire format.
 *
 * <p>The wire format is: {@code [varint: uncompressed_size] [LZ4 block compressed data]}
 *
 * <p>The varint is a standard protobuf base-128 variable-length encoding of the original
 * uncompressed size, matching folly::encodeVarint / folly::decodeVarint.
 *
 * <p>Uses LZ4 block-mode compression (not frame), matching C++ LZ4_compress_default /
 * LZ4_decompress_safe.
 */
public final class Lz4Compressor implements ThriftCompressor {

  public static final Lz4Compressor INSTANCE = new Lz4Compressor();

  private static final LZ4Factory FACTORY = LZ4Factory.fastestInstance();
  private static final net.jpountz.lz4.LZ4Compressor COMPRESSOR = FACTORY.fastCompressor();
  private static final LZ4FastDecompressor DECOMPRESSOR = FACTORY.fastDecompressor();

  Lz4Compressor() {}

  @Override
  public ByteBuf compress(ByteBufAllocator allocator, ByteBuf data) {
    try {
      int uncompressedSize = data.readableBytes();
      int maxCompressedSize = COMPRESSOR.maxCompressedLength(uncompressedSize);
      int varintLength = varintSize(uncompressedSize);

      ByteBuf output = allocator.directBuffer(varintLength + maxCompressedSize);
      try {
        writeVarint(output, uncompressedSize);

        ByteBuffer source = data.nioBuffer();
        ByteBuffer destination = output.nioBuffer(output.writerIndex(), output.writableBytes());
        int compressedSize =
            COMPRESSOR.compress(
                source,
                source.position(),
                source.remaining(),
                destination,
                destination.position(),
                destination.remaining());
        output.writerIndex(output.writerIndex() + compressedSize);
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
      long uncompressedSize = readVarint(data);
      if (uncompressedSize < 0 || uncompressedSize > Integer.MAX_VALUE) {
        throw new IllegalStateException("LZ4 decompressed size out of range: " + uncompressedSize);
      }
      int destLen = (int) uncompressedSize;

      ByteBuf output = allocator.directBuffer(destLen);
      try {
        ByteBuffer source = data.nioBuffer();
        ByteBuffer destination = output.nioBuffer(0, destLen);
        DECOMPRESSOR.decompress(
            source, source.position(), destination, destination.position(), destLen);
        output.writerIndex(destLen);
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
   * Writes a varint-encoded value to the buffer. Matches folly::encodeVarint: little-endian
   * base-128 encoding with MSB as continuation bit (standard protobuf varint).
   */
  public static void writeVarint(ByteBuf buffer, long value) {
    while (value >= 0x80) {
      buffer.writeByte((int) (0x80 | (value & 0x7F)));
      value >>>= 7;
    }
    buffer.writeByte((int) value);
  }

  /**
   * Reads a varint-encoded value from the buffer, advancing the reader index. Matches
   * folly::decodeVarint.
   */
  public static long readVarint(ByteBuf buffer) {
    long result = 0;
    int shift = 0;
    while (shift < 64) {
      byte b = buffer.readByte();
      result |= (long) (b & 0x7F) << shift;
      if ((b & 0x80) == 0) {
        return result;
      }
      shift += 7;
    }
    throw new IllegalStateException("Varint too long");
  }

  public static int varintSize(long value) {
    int size = 1;
    while (value >= 0x80) {
      value >>>= 7;
      size++;
    }
    return size;
  }
}
