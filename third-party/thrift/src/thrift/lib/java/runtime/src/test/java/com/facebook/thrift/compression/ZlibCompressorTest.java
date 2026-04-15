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

import static org.assertj.core.api.Assertions.assertThat;

import com.facebook.thrift.compression.zlib.ZlibCompressor;
import io.netty.buffer.ByteBuf;
import io.netty.buffer.ByteBufAllocator;
import io.netty.buffer.Unpooled;
import java.nio.charset.StandardCharsets;
import java.util.Random;
import java.util.zip.Deflater;
import org.junit.jupiter.api.Test;

class ZlibCompressorTest {

  private final ZlibCompressor compressor = ZlibCompressor.INSTANCE;
  private final ByteBufAllocator allocator = ByteBufAllocator.DEFAULT;

  @Test
  void roundtripSimpleString() {
    byte[] original = "Hello, ZLIB compression in Thrift!".getBytes(StandardCharsets.UTF_8);
    ByteBuf input = Unpooled.wrappedBuffer(original);

    ByteBuf compressed = compressor.compress(allocator, input);
    assertThat(compressed.readableBytes()).isGreaterThan(0);

    ByteBuf decompressed = compressor.decompress(allocator, compressed);
    byte[] result = new byte[decompressed.readableBytes()];
    decompressed.readBytes(result);
    decompressed.release();

    assertThat(result).isEqualTo(original);
  }

  @Test
  void roundtripLargePayload() {
    byte[] original = new byte[1024 * 1024];
    for (int i = 0; i < original.length; i++) {
      original[i] = (byte) (i % 251);
    }
    ByteBuf input = Unpooled.wrappedBuffer(original);

    ByteBuf compressed = compressor.compress(allocator, input);
    assertThat(compressed.readableBytes()).isLessThan(original.length);

    ByteBuf decompressed = compressor.decompress(allocator, compressed);
    byte[] result = new byte[decompressed.readableBytes()];
    decompressed.readBytes(result);
    decompressed.release();

    assertThat(result).isEqualTo(original);
  }

  @Test
  void roundtripRandomData() {
    byte[] original = new byte[8192];
    new Random(42).nextBytes(original);
    ByteBuf input = Unpooled.wrappedBuffer(original);

    ByteBuf compressed = compressor.compress(allocator, input);
    ByteBuf decompressed = compressor.decompress(allocator, compressed);
    byte[] result = new byte[decompressed.readableBytes()];
    decompressed.readBytes(result);
    decompressed.release();

    assertThat(result).isEqualTo(original);
  }

  @Test
  void decompressGrowsOutputBufferAcrossMultipleInflateIterations() {
    // Highly compressible data: 64KB of zeros compresses to ~100 bytes.
    // On decompress, the output ByteBuf starts small (~1024 bytes after ensureWritable)
    // and must grow many times as inflate() is called repeatedly. This exercises the
    // ensureWritable(MIN_WRITABLE_BYTES) growth path in the inflate loop.
    byte[] original = new byte[64 * 1024];

    ByteBuf input = Unpooled.wrappedBuffer(original);
    ByteBuf compressed = compressor.compress(allocator, input);

    // Verify high compression ratio — compressed output should be tiny
    assertThat(compressed.readableBytes()).isLessThan(200);

    ByteBuf decompressed = compressor.decompress(allocator, compressed);

    assertThat(decompressed.readableBytes()).isEqualTo(original.length);
    byte[] result = new byte[decompressed.readableBytes()];
    decompressed.readBytes(result);
    decompressed.release();

    assertThat(result).isEqualTo(original);
  }

  @Test
  void decompressDataCompressedByJdkDeflater() {
    // Verify our ZlibCompressor can decompress data compressed by the standard JDK Deflater,
    // which is what the existing CompressionUtil path would have produced.
    byte[] original = "Data compressed by standard JDK Deflater".getBytes(StandardCharsets.UTF_8);

    Deflater deflater = new Deflater();
    deflater.setInput(original);
    deflater.finish();
    byte[] compressedBytes = new byte[1024];
    int compressedLength = deflater.deflate(compressedBytes);
    deflater.end();

    ByteBuf compressed = Unpooled.wrappedBuffer(compressedBytes, 0, compressedLength);
    ByteBuf decompressed = compressor.decompress(allocator, compressed);
    byte[] result = new byte[decompressed.readableBytes()];
    decompressed.readBytes(result);
    decompressed.release();

    assertThat(result).isEqualTo(original);
  }

  @Test
  void compressReleasesInputBuffer() {
    ByteBuf input = allocator.buffer().writeBytes("test data".getBytes(StandardCharsets.UTF_8));
    assertThat(input.refCnt()).isEqualTo(1);

    ByteBuf compressed = compressor.compress(allocator, input);
    assertThat(input.refCnt()).isEqualTo(0);

    compressed.release();
  }

  @Test
  void decompressReleasesInputBuffer() {
    byte[] original = "test data".getBytes(StandardCharsets.UTF_8);
    ByteBuf input = Unpooled.wrappedBuffer(original);
    ByteBuf compressed = compressor.compress(allocator, input);

    ByteBuf compressedCopy =
        allocator
            .buffer()
            .writeBytes(compressed, compressed.readerIndex(), compressed.readableBytes());
    compressed.release();

    assertThat(compressedCopy.refCnt()).isEqualTo(1);
    ByteBuf decompressed = compressor.decompress(allocator, compressedCopy);
    assertThat(compressedCopy.refCnt()).isEqualTo(0);

    decompressed.release();
  }
}
