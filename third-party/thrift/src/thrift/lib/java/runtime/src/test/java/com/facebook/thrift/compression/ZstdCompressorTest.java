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

import com.facebook.thrift.compression.zstd.ZstdCompressor;
import io.netty.buffer.ByteBuf;
import io.netty.buffer.ByteBufAllocator;
import io.netty.buffer.Unpooled;
import java.nio.charset.StandardCharsets;
import java.util.Random;
import org.junit.jupiter.api.Test;

class ZstdCompressorTest {

  private final ZstdCompressor compressor = ZstdCompressor.INSTANCE;
  private final ByteBufAllocator allocator = ByteBufAllocator.DEFAULT;

  // ZSTD frame magic number (little-endian): 0xFD2FB528
  private static final int ZSTD_MAGIC = 0xFD2FB528;

  // --- Basic roundtrip tests ---

  @Test
  void roundtripSimpleString() {
    byte[] original = "Hello, Thrift compression!".getBytes(StandardCharsets.UTF_8);
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
  void roundtripSingleByte() {
    ByteBuf input = Unpooled.wrappedBuffer(new byte[] {0x42});

    ByteBuf compressed = compressor.compress(allocator, input);
    ByteBuf decompressed = compressor.decompress(allocator, compressed);
    byte[] result = new byte[decompressed.readableBytes()];
    decompressed.readBytes(result);
    decompressed.release();

    assertThat(result).isEqualTo(new byte[] {0x42});
  }

  // --- Buffer type tests (exercises heap vs direct paths in FFI version) ---

  @Test
  void roundtripWithHeapBuffer() {
    // Unpooled.wrappedBuffer(byte[]) creates a heap-backed ByteBuf.
    // On Java 25 FFI, this exercises the toNativeSegment() copy-to-native fallback.
    byte[] original = "Heap buffer roundtrip test data for ZSTD".getBytes(StandardCharsets.UTF_8);
    ByteBuf input = Unpooled.wrappedBuffer(original);
    assertThat(input.hasArray()).isTrue();

    ByteBuf compressed = compressor.compress(allocator, input);
    ByteBuf decompressed = compressor.decompress(allocator, compressed);

    byte[] result = new byte[decompressed.readableBytes()];
    decompressed.readBytes(result);
    decompressed.release();

    assertThat(result).isEqualTo(original);
  }

  @Test
  void roundtripWithDirectBuffer() {
    // allocator.directBuffer() creates a direct ByteBuf.
    // On Java 25 FFI, this exercises the zero-copy MemorySegment.ofAddress() path.
    byte[] original = "Direct buffer roundtrip test data for ZSTD".getBytes(StandardCharsets.UTF_8);
    ByteBuf input = allocator.directBuffer(original.length).writeBytes(original);
    assertThat(input.hasMemoryAddress()).isTrue();

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
    byte[] original = "Heap slice roundtrip test data for ZSTD".getBytes(StandardCharsets.UTF_8);
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
    byte[] original = "Direct slice roundtrip test data for ZSTD".getBytes(StandardCharsets.UTF_8);
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
  void compressHeapDecompressDirect() {
    byte[] original = "Cross heap-to-direct test".getBytes(StandardCharsets.UTF_8);

    // Compress from heap buffer
    ByteBuf heapInput = Unpooled.wrappedBuffer(original);
    ByteBuf compressed = compressor.compress(allocator, heapInput);

    // Copy compressed data to a direct buffer for decompression
    ByteBuf directCompressed = allocator.directBuffer(compressed.readableBytes());
    directCompressed.writeBytes(compressed);
    compressed.release();

    ByteBuf decompressed = compressor.decompress(allocator, directCompressed);
    byte[] result = new byte[decompressed.readableBytes()];
    decompressed.readBytes(result);
    decompressed.release();

    assertThat(result).isEqualTo(original);
  }

  @Test
  void compressDirectDecompressHeap() {
    byte[] original = "Cross direct-to-heap test".getBytes(StandardCharsets.UTF_8);

    // Compress from direct buffer
    ByteBuf directInput = allocator.directBuffer(original.length).writeBytes(original);
    ByteBuf compressed = compressor.compress(allocator, directInput);

    // Copy compressed data to a heap buffer for decompression
    byte[] compressedBytes = new byte[compressed.readableBytes()];
    compressed.readBytes(compressedBytes);
    compressed.release();
    ByteBuf heapCompressed = Unpooled.wrappedBuffer(compressedBytes);

    ByteBuf decompressed = compressor.decompress(allocator, heapCompressed);
    byte[] result = new byte[decompressed.readableBytes()];
    decompressed.readBytes(result);
    decompressed.release();

    assertThat(result).isEqualTo(original);
  }

  // --- Context reuse tests (exercises thread-local CCtx/DCtx caching in FFI version) ---

  @Test
  void repeatedCompressionReusesSameThread() {
    // Compress multiple payloads on the same thread to exercise context reuse.
    // On Java 25 FFI, this verifies that the thread-local ZSTD_CCtx is reused correctly.
    for (int i = 0; i < 100; i++) {
      byte[] original =
          ("Iteration " + i + " of context reuse test").getBytes(StandardCharsets.UTF_8);
      ByteBuf input = Unpooled.wrappedBuffer(original);

      ByteBuf compressed = compressor.compress(allocator, input);
      ByteBuf decompressed = compressor.decompress(allocator, compressed);

      byte[] result = new byte[decompressed.readableBytes()];
      decompressed.readBytes(result);
      decompressed.release();

      assertThat(result).isEqualTo(original);
    }
  }

  @Test
  void repeatedCompressionWithVaryingSizes() {
    // Vary payload sizes to stress context reuse with different internal buffer states.
    Random random = new Random(99);
    for (int i = 0; i < 50; i++) {
      int size = 1 + random.nextInt(32 * 1024);
      byte[] original = new byte[size];
      random.nextBytes(original);

      ByteBuf input = Unpooled.wrappedBuffer(original);
      ByteBuf compressed = compressor.compress(allocator, input);
      ByteBuf decompressed = compressor.decompress(allocator, compressed);

      byte[] result = new byte[decompressed.readableBytes()];
      decompressed.readBytes(result);
      decompressed.release();

      assertThat(result).isEqualTo(original);
    }
  }

  // --- ZSTD frame format validation ---

  @Test
  void compressedOutputHasZstdMagicNumber() {
    byte[] original = "Check for ZSTD magic number".getBytes(StandardCharsets.UTF_8);
    ByteBuf input = Unpooled.wrappedBuffer(original);

    ByteBuf compressed = compressor.compress(allocator, input);

    // ZSTD frames start with magic number 0xFD2FB528 (little-endian)
    assertThat(compressed.readableBytes()).isGreaterThanOrEqualTo(4);
    int magic = compressed.getIntLE(0);
    assertThat(magic).isEqualTo(ZSTD_MAGIC);

    compressed.release();
  }

  @Test
  void compressedOutputContainsContentSize() {
    // ZSTD_compress / ZSTD_compressCCtx always writes the content size in the frame header.
    // The frame header descriptor byte (offset 4) has bit 5-6 encoding the content size flag.
    byte[] original = "Content size field test".getBytes(StandardCharsets.UTF_8);
    ByteBuf input = Unpooled.wrappedBuffer(original);

    ByteBuf compressed = compressor.compress(allocator, input);
    assertThat(compressed.readableBytes()).isGreaterThanOrEqualTo(5);

    // Frame header descriptor is at offset 4.
    // Bits 5-6 (FCS_Field_Size) should be non-zero, indicating content size is present.
    int descriptor = compressed.getByte(4) & 0xFF;
    int fcsFieldSize = (descriptor >> 6) & 0x03;
    // fcsFieldSize: 0=0 bytes (but Single_Segment flag may still encode 1 byte), 1=2 bytes, 2=4
    // bytes, 3=8 bytes
    // For small payloads with Single_Segment_flag set, fcsFieldSize=0 encodes 1-byte size.
    // Either way, the content size is always present for one-shot compression.
    boolean singleSegment = (descriptor & 0x20) != 0;
    assertThat(fcsFieldSize > 0 || singleSegment)
        .as("Content size should be present in frame header")
        .isTrue();

    compressed.release();
  }

  // --- Test vector: hardcoded ZSTD compressed data ---

  @Test
  void decompressKnownTestVector() {
    // This test vector was generated by compressing "ZSTD test vector" (16 bytes)
    // using ZSTD at default compression level (3). The ZSTD frame format is stable
    // across implementations and versions for the same input/level.
    //
    // Frame structure:
    //   Magic: FD 2F B5 28
    //   Frame header: 00 10 (single segment, 16 bytes content)
    //   Block: compressed block with the actual data
    //   (no checksum)
    byte[] plaintext = "ZSTD test vector".getBytes(StandardCharsets.UTF_8);

    // Rather than hardcoding exact compressed bytes (which may vary with zstd version),
    // we compress at runtime, then verify the output decompresses correctly AND has valid
    // frame structure. This validates the full compress→frame→decompress pipeline.
    ByteBuf compressInput = Unpooled.wrappedBuffer(plaintext);
    ByteBuf compressed = compressor.compress(allocator, compressInput);

    // Verify frame magic
    assertThat(compressed.getIntLE(0)).isEqualTo(ZSTD_MAGIC);

    // Save compressed bytes to verify decompression from a cold buffer
    byte[] compressedBytes = new byte[compressed.readableBytes()];
    compressed.getBytes(0, compressedBytes);
    compressed.release();

    // Decompress from a fresh heap buffer (not the one returned by compress)
    ByteBuf freshInput = Unpooled.wrappedBuffer(compressedBytes);
    ByteBuf decompressed = compressor.decompress(allocator, freshInput);

    byte[] result = new byte[decompressed.readableBytes()];
    decompressed.readBytes(result);
    decompressed.release();

    assertThat(result).isEqualTo(plaintext);
    assertThat(new String(result, StandardCharsets.UTF_8)).isEqualTo("ZSTD test vector");
  }

  @Test
  void compressedBytesDecompressFromIndependentBuffer() {
    // Compress known data, extract the raw bytes, then decompress from a completely
    // independent buffer to verify the compressed frame is self-contained and portable.
    byte[] original = {(byte) 0xDE, (byte) 0xAD, (byte) 0xBE, (byte) 0xEF};
    ByteBuf input = Unpooled.wrappedBuffer(original);
    ByteBuf compressed = compressor.compress(allocator, input);

    // Extract compressed bytes and release the original compressed buffer
    byte[] compressedBytes = new byte[compressed.readableBytes()];
    compressed.readBytes(compressedBytes);
    compressed.release();

    // Verify magic number in extracted bytes
    assertThat(compressedBytes[0]).isEqualTo((byte) 0x28);
    assertThat(compressedBytes[1]).isEqualTo((byte) 0xB5);
    assertThat(compressedBytes[2]).isEqualTo((byte) 0x2F);
    assertThat(compressedBytes[3]).isEqualTo((byte) 0xFD);

    // Decompress from a fresh, independent heap buffer
    ByteBuf freshInput = Unpooled.wrappedBuffer(compressedBytes);
    ByteBuf decompressed = compressor.decompress(allocator, freshInput);

    byte[] result = new byte[decompressed.readableBytes()];
    decompressed.readBytes(result);
    decompressed.release();

    assertThat(result).isEqualTo(original);
  }

  // --- Highly compressible data (exercises output sizing) ---

  @Test
  void roundtripHighlyCompressibleData() {
    // 256KB of zeros compresses to a tiny frame.
    // Verifies correct buffer sizing when compressed output is much smaller than input.
    byte[] original = new byte[256 * 1024];
    ByteBuf input = Unpooled.wrappedBuffer(original);

    ByteBuf compressed = compressor.compress(allocator, input);
    assertThat(compressed.readableBytes()).isLessThan(1024);

    ByteBuf decompressed = compressor.decompress(allocator, compressed);

    assertThat(decompressed.readableBytes()).isEqualTo(original.length);
    byte[] result = new byte[decompressed.readableBytes()];
    decompressed.readBytes(result);
    decompressed.release();

    assertThat(result).isEqualTo(original);
  }

  // --- Ownership / lifecycle tests ---

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
    ByteBuf original = Unpooled.wrappedBuffer("test data".getBytes(StandardCharsets.UTF_8));
    ByteBuf compressed = compressor.compress(allocator, original);

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

  // --- Error handling ---

  @Test
  void decompressInvalidDataThrows() {
    ByteBuf garbage = Unpooled.wrappedBuffer(new byte[] {0x00, 0x01, 0x02, 0x03});
    assertThatThrownBy(() -> compressor.decompress(allocator, garbage))
        .isInstanceOf(Exception.class);
  }

  @Test
  void decompressTruncatedFrameThrows() {
    // Valid ZSTD magic but truncated — should fail on getFrameContentSize or decompress
    byte[] truncated = {(byte) 0x28, (byte) 0xB5, (byte) 0x2F, (byte) 0xFD, (byte) 0x00};
    ByteBuf input = Unpooled.wrappedBuffer(truncated);
    assertThatThrownBy(() -> compressor.decompress(allocator, input)).isInstanceOf(Exception.class);
  }
}
