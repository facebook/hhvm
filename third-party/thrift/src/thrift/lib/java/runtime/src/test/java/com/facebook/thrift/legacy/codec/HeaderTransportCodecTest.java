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

package com.facebook.thrift.legacy.codec;

import static org.assertj.core.api.Assertions.assertThat;

import com.facebook.thrift.compression.CompressionManager;
import io.netty.buffer.ByteBuf;
import io.netty.buffer.ByteBufAllocator;
import io.netty.buffer.Unpooled;
import java.nio.charset.StandardCharsets;
import org.apache.thrift.CompressionAlgorithm;
import org.apache.thrift.TTransform;
import org.junit.jupiter.api.Test;

class HeaderTransportCodecTest {

  private static final int HEADER_MAGIC = 0x0FFF;
  private static final int BINARY_PROTOCOL_ID = 0;
  private final ByteBufAllocator allocator = ByteBufAllocator.DEFAULT;

  @Test
  void decodeFrameNoTransforms() {
    byte[] payload = "no compression".getBytes(StandardCharsets.UTF_8);
    ByteBuf frame = buildFrame(new int[0], Unpooled.wrappedBuffer(payload));

    ThriftFrame result = HeaderTransportCodec.decodeFrame(allocator, frame);

    byte[] decoded = new byte[result.getMessage().readableBytes()];
    result.getMessage().readBytes(decoded);
    assertThat(decoded).isEqualTo(payload);
    result.release();
  }

  @Test
  void decodeFrameWithZlibTransform() {
    byte[] payload = "ZLIB compressed payload for THeader".getBytes(StandardCharsets.UTF_8);
    ByteBuf compressed =
        CompressionManager.compress(
            CompressionAlgorithm.ZLIB, allocator, Unpooled.wrappedBuffer(payload));

    ByteBuf frame = buildFrame(new int[] {TTransform.ZLIB.getValue()}, compressed);

    ThriftFrame result = HeaderTransportCodec.decodeFrame(allocator, frame);

    byte[] decoded = new byte[result.getMessage().readableBytes()];
    result.getMessage().readBytes(decoded);
    assertThat(decoded).isEqualTo(payload);
    result.release();
  }

  @Test
  void decodeFrameWithZstdTransform() {
    byte[] payload = "ZSTD compressed payload for THeader".getBytes(StandardCharsets.UTF_8);
    ByteBuf compressed =
        CompressionManager.compress(
            CompressionAlgorithm.ZSTD, allocator, Unpooled.wrappedBuffer(payload));

    ByteBuf frame = buildFrame(new int[] {TTransform.ZSTD.getValue()}, compressed);

    ThriftFrame result = HeaderTransportCodec.decodeFrame(allocator, frame);

    byte[] decoded = new byte[result.getMessage().readableBytes()];
    result.getMessage().readBytes(decoded);
    assertThat(decoded).isEqualTo(payload);
    result.release();
  }

  @Test
  void decodeFrameWithLz4Transform() {
    byte[] payload = "LZ4 compressed payload for THeader".getBytes(StandardCharsets.UTF_8);
    ByteBuf compressed =
        CompressionManager.compress(
            CompressionAlgorithm.LZ4, allocator, Unpooled.wrappedBuffer(payload));

    ByteBuf frame = buildFrame(new int[] {TTransform.LZ4.getValue()}, compressed);

    ThriftFrame result = HeaderTransportCodec.decodeFrame(allocator, frame);

    byte[] decoded = new byte[result.getMessage().readableBytes()];
    result.getMessage().readBytes(decoded);
    assertThat(decoded).isEqualTo(payload);
    result.release();
  }

  @Test
  void decodeFrameWithNoneTransform() {
    byte[] payload = "NONE transform passthrough".getBytes(StandardCharsets.UTF_8);
    ByteBuf frame =
        buildFrame(new int[] {TTransform.NONE.getValue()}, Unpooled.wrappedBuffer(payload));

    ThriftFrame result = HeaderTransportCodec.decodeFrame(allocator, frame);

    byte[] decoded = new byte[result.getMessage().readableBytes()];
    result.getMessage().readBytes(decoded);
    assertThat(decoded).isEqualTo(payload);
    result.release();
  }

  /**
   * Builds a minimal THeader frame suitable for decodeFrame(). The wire format is:
   *
   * <pre>
   *   short  magic (0x0FFF)
   *   short  flags
   *   int    sequenceId
   *   short  headerSize (in 4-byte words)
   *   [header section]:
   *     varint protocolId
   *     varint numTransforms
   *     varint transformId * numTransforms
   *     [padding to 4-byte boundary]
   *   [payload]
   * </pre>
   */
  private ByteBuf buildFrame(int[] transformIds, ByteBuf payload) {
    ByteBuf headerSection = allocator.buffer();
    writeVarInt(headerSection, BINARY_PROTOCOL_ID);
    writeVarInt(headerSection, transformIds.length);
    for (int id : transformIds) {
      writeVarInt(id, headerSection);
    }

    int headerBytes = headerSection.readableBytes();
    int paddingSize = 4 - (headerBytes & 0b11);
    headerSection.writeZero(paddingSize);
    int paddedHeaderSize = headerBytes + paddingSize;

    ByteBuf frame = allocator.buffer();
    frame.writeShort(HEADER_MAGIC);
    frame.writeShort(0); // flags: none
    frame.writeInt(1); // sequenceId
    frame.writeShort(paddedHeaderSize >> 2); // header size in 4-byte words
    frame.writeBytes(headerSection);
    frame.writeBytes(payload);

    headerSection.release();
    payload.release();
    return frame;
  }

  private static void writeVarInt(ByteBuf buffer, int value) {
    while (true) {
      if ((value & ~0x7F) == 0) {
        buffer.writeByte(value);
        return;
      }
      buffer.writeByte(value | 0x80);
      value >>>= 7;
    }
  }

  private static void writeVarInt(int value, ByteBuf buffer) {
    writeVarInt(buffer, value);
  }
}
