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
import static org.assertj.core.api.Assertions.assertThatThrownBy;

import com.facebook.thrift.compression.lz4.Lz4Compressor;
import io.netty.buffer.ByteBuf;
import io.netty.buffer.ByteBufAllocator;
import io.netty.buffer.Unpooled;
import java.nio.charset.StandardCharsets;
import java.util.Random;
import org.junit.jupiter.api.Test;

class Lz4CompressorTest {

  private final Lz4Compressor compressor = Lz4Compressor.INSTANCE;
  private final ByteBufAllocator allocator = ByteBufAllocator.DEFAULT;

  @Test
  void roundtripSimpleString() {
    byte[] original = "Hello, LZ4 compression!".getBytes(StandardCharsets.UTF_8);
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
    byte[] original = new byte[64 * 1024];
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
  void roundtripRandomData() {
    byte[] original = new byte[4096];
    new Random(123).nextBytes(original);
    ByteBuf input = Unpooled.wrappedBuffer(original);

    ByteBuf compressed = compressor.compress(allocator, input);
    ByteBuf decompressed = compressor.decompress(allocator, compressed);

    byte[] result = new byte[decompressed.readableBytes()];
    decompressed.readBytes(result);
    decompressed.release();

    assertThat(result).isEqualTo(original);
  }

  @Test
  void roundtripWithHeapSlice() {
    byte[] padding = "prefix-".getBytes(StandardCharsets.UTF_8);
    byte[] original = "Heap slice roundtrip".getBytes(StandardCharsets.UTF_8);
    byte[] suffix = "-suffix".getBytes(StandardCharsets.UTF_8);
    byte[] backing = new byte[padding.length + original.length + suffix.length];
    System.arraycopy(padding, 0, backing, 0, padding.length);
    System.arraycopy(original, 0, backing, padding.length, original.length);
    System.arraycopy(suffix, 0, backing, padding.length + original.length, suffix.length);

    ByteBuf input = Unpooled.wrappedBuffer(backing, padding.length, original.length);
    ByteBuf compressed = compressor.compress(allocator, input);
    ByteBuf decompressed = compressor.decompress(allocator, compressed);

    byte[] result = new byte[decompressed.readableBytes()];
    decompressed.readBytes(result);
    decompressed.release();

    assertThat(result).isEqualTo(original);
  }

  @Test
  void roundtripWithDirectSlice() {
    byte[] original = "Direct slice roundtrip".getBytes(StandardCharsets.UTF_8);
    ByteBuf backing = allocator.directBuffer(original.length + 8);
    backing.writeZero(3).writeBytes(original).writeZero(5);
    ByteBuf input = backing.retainedSlice(3, original.length);
    backing.release();

    ByteBuf compressed = compressor.compress(allocator, input);
    ByteBuf decompressed = compressor.decompress(allocator, compressed);

    byte[] result = new byte[decompressed.readableBytes()];
    decompressed.readBytes(result);
    decompressed.release();

    assertThat(result).isEqualTo(original);
  }

  @Test
  void roundtripSingleByte() {
    ByteBuf input = Unpooled.wrappedBuffer(new byte[] {0x42});

    ByteBuf compressed = compressor.compress(allocator, input);
    ByteBuf decompressed = compressor.decompress(allocator, compressed);

    byte[] result = new byte[decompressed.readableBytes()];
    decompressed.readBytes(result);
    decompressed.release();

    assertThat(result).isEqualTo(new byte[] {0x42});
  }

  @Test
  void compressedOutputHasVarintPrefix() {
    byte[] original = "test data for varint prefix".getBytes(StandardCharsets.UTF_8);
    ByteBuf input = Unpooled.wrappedBuffer(original);

    ByteBuf compressed = compressor.compress(allocator, input);

    // The first byte(s) should be the varint-encoded uncompressed size.
    // For small sizes (< 128), varint is a single byte equal to the size.
    int firstByte = compressed.getByte(0) & 0xFF;
    assertThat(firstByte).isEqualTo(original.length);
    compressed.release();
  }

  @Test
  void varintPrefixForLargeSize() {
    // Create a payload > 127 bytes so varint uses 2+ bytes
    byte[] original = new byte[300];
    new Random(99).nextBytes(original);
    ByteBuf input = Unpooled.wrappedBuffer(original);

    ByteBuf compressed = compressor.compress(allocator, input);

    // 300 = 0b100101100 → varint: [0xAC, 0x02] (two bytes)
    int firstByte = compressed.getByte(0) & 0xFF;
    assertThat(firstByte & 0x80).isNotZero(); // continuation bit set
    int secondByte = compressed.getByte(1) & 0xFF;
    assertThat(secondByte & 0x80).isZero(); // last varint byte

    // Decode and verify it roundtrips
    ByteBuf decompressed = compressor.decompress(allocator, compressed);
    byte[] result = new byte[decompressed.readableBytes()];
    decompressed.readBytes(result);
    decompressed.release();
    assertThat(result).isEqualTo(original);
  }

  @Test
  void compressReleasesInputBuffer() {
    ByteBuf input = allocator.buffer().writeBytes("release test".getBytes(StandardCharsets.UTF_8));
    assertThat(input.refCnt()).isEqualTo(1);

    ByteBuf compressed = compressor.compress(allocator, input);
    assertThat(input.refCnt()).isZero();
    compressed.release();
  }

  @Test
  void decompressReleasesInputBuffer() {
    ByteBuf original = Unpooled.wrappedBuffer("release test".getBytes(StandardCharsets.UTF_8));
    ByteBuf compressed = compressor.compress(allocator, original);

    ByteBuf compressedCopy = allocator.buffer().writeBytes(compressed);
    compressed.release();
    assertThat(compressedCopy.refCnt()).isEqualTo(1);

    ByteBuf decompressed = compressor.decompress(allocator, compressedCopy);
    assertThat(compressedCopy.refCnt()).isZero();
    decompressed.release();
  }

  @Test
  void decompressInvalidDataThrows() {
    ByteBuf invalid = Unpooled.wrappedBuffer(new byte[] {0x05, 0x00, 0x01, 0x02, 0x03});
    assertThatThrownBy(() -> compressor.decompress(allocator, invalid))
        .isInstanceOf(Exception.class);
  }

  // --- Varint unit tests ---

  @Test
  void varintRoundtripSmallValue() {
    ByteBuf buffer = allocator.buffer();
    Lz4Compressor.writeVarint(buffer, 42);
    assertThat(buffer.readableBytes()).isEqualTo(1);
    assertThat(Lz4Compressor.readVarint(buffer)).isEqualTo(42);
    buffer.release();
  }

  @Test
  void varintRoundtripZero() {
    ByteBuf buffer = allocator.buffer();
    Lz4Compressor.writeVarint(buffer, 0);
    assertThat(buffer.readableBytes()).isEqualTo(1);
    assertThat(Lz4Compressor.readVarint(buffer)).isEqualTo(0);
    buffer.release();
  }

  @Test
  void varintRoundtripTwoBytes() {
    ByteBuf buffer = allocator.buffer();
    Lz4Compressor.writeVarint(buffer, 300);
    assertThat(buffer.readableBytes()).isEqualTo(2);
    assertThat(Lz4Compressor.readVarint(buffer)).isEqualTo(300);
    buffer.release();
  }

  @Test
  void varintRoundtripLargeValue() {
    ByteBuf buffer = allocator.buffer();
    long value = 1_000_000_000L;
    Lz4Compressor.writeVarint(buffer, value);
    assertThat(Lz4Compressor.readVarint(buffer)).isEqualTo(value);
    buffer.release();
  }

  @Test
  void varintSizeCalculation() {
    assertThat(Lz4Compressor.varintSize(0)).isEqualTo(1);
    assertThat(Lz4Compressor.varintSize(127)).isEqualTo(1);
    assertThat(Lz4Compressor.varintSize(128)).isEqualTo(2);
    assertThat(Lz4Compressor.varintSize(16383)).isEqualTo(2);
    assertThat(Lz4Compressor.varintSize(16384)).isEqualTo(3);
  }
}
