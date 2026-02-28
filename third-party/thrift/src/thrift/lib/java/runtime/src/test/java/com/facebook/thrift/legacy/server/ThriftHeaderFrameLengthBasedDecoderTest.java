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

package com.facebook.thrift.legacy.server;

import static org.junit.Assert.*;

import io.netty.buffer.ByteBuf;
import io.netty.buffer.Unpooled;
import io.netty.channel.embedded.EmbeddedChannel;
import io.netty.handler.codec.TooLongFrameException;
import org.junit.Test;

public class ThriftHeaderFrameLengthBasedDecoderTest {

  @Test
  public void testNormalFramePayload() {
    int payloadSize = 12;
    ByteBuf buf = Unpooled.buffer(payloadSize + 4);
    buf.writeInt(payloadSize);
    for (int i = 0; i < payloadSize; i++) {
      buf.writeByte(i);
    }
    EmbeddedChannel channel = new EmbeddedChannel(new ThriftHeaderFrameLengthBasedDecoder());

    channel.writeInbound(buf);

    assertTrue(channel.finish());

    ByteBuf b = channel.readInbound();

    assertEquals(12, b.readableBytes());

    for (int i = 0; i < 12; i++) {
      assertEquals((byte) i, b.readByte());
    }
    b.release();
    assertNull(channel.readInbound());
    channel.finish();
  }

  @Test
  public void testNormalFrameMaxSizePayload() {
    // Max Size + Length Field
    int frameLength = 0x3FFFFFFF;
    ByteBuf buf = Unpooled.buffer(frameLength + 4);
    buf.writeInt(0x3FFFFFFF);
    for (int i = 0; i < frameLength; i++) {
      buf.writeByte(i);
    }
    EmbeddedChannel channel = new EmbeddedChannel(new ThriftHeaderFrameLengthBasedDecoder());

    channel.writeInbound(buf);

    assertTrue(channel.finish());

    ByteBuf b = channel.readInbound();

    assertEquals(0x3FFFFFFF, b.readableBytes());

    for (int i = 0; i < frameLength; i++) {
      assertEquals((byte) i, b.readByte());
    }
    b.release();
    assertNull(channel.readInbound());
    channel.finish();
  }

  @Test(expected = TooLongFrameException.class)
  public void testNormalFrameOverSizePayloadFails() {
    // Max Fame Size + 1
    int frameLength = 0x3FFFFFFF + 1;
    ByteBuf buf = Unpooled.buffer(frameLength + 4);
    buf.writeInt(frameLength);
    for (int i = 0; i < frameLength; i++) {
      buf.writeByte(i % Byte.MAX_VALUE);
    }
    EmbeddedChannel channel = new EmbeddedChannel(new ThriftHeaderFrameLengthBasedDecoder());

    channel.writeInbound(buf);
  }

  @Test
  public void testBigFrameOverSizedPayloadSucceeds() {
    int frameLength = Integer.MAX_VALUE - 1024;
    ByteBuf buf = Unpooled.buffer(frameLength + 12);
    // BIG_FRAME_MAGIC
    buf.writeInt(0x42494746);
    buf.writeLong(frameLength);
    for (int i = 0; i < frameLength; i++) {
      buf.writeByte(i % Byte.MAX_VALUE);
    }
    EmbeddedChannel channel = new EmbeddedChannel(new ThriftHeaderFrameLengthBasedDecoder());

    channel.writeInbound(buf);

    assertTrue(channel.finish());

    ByteBuf b = channel.readInbound();

    assertEquals(frameLength, b.readableBytes());

    for (int i = 0; i < frameLength; i++) {
      assertEquals((byte) (i % Byte.MAX_VALUE), b.readByte());
    }
    b.release();
    assertNull(channel.readInbound());
    channel.finish();
  }

  @Test
  public void testMultipleNormalFrames() {
    // Test sending 10 frames to ensure frame length calculation doesn't accumulate readerIndex
    EmbeddedChannel channel = new EmbeddedChannel(new ThriftHeaderFrameLengthBasedDecoder());
    int numFrames = 10;
    int payloadSize = 100;

    for (int frame = 0; frame < numFrames; frame++) {
      ByteBuf buf = Unpooled.buffer(payloadSize + 4);
      buf.writeInt(payloadSize);
      for (int i = 0; i < payloadSize; i++) {
        buf.writeByte((byte) ((frame * 256 + i) % 256));
      }
      channel.writeInbound(buf);
    }

    assertTrue(channel.finish());

    // Verify all frames were decoded correctly
    for (int frame = 0; frame < numFrames; frame++) {
      ByteBuf b = channel.readInbound();
      assertNotNull("Frame " + frame + " should not be null", b);
      assertEquals("Frame " + frame + " size mismatch", payloadSize, b.readableBytes());

      for (int i = 0; i < payloadSize; i++) {
        byte expected = (byte) ((frame * 256 + i) % 256);
        byte actual = b.readByte();
        assertEquals("Frame " + frame + " byte " + i + " mismatch", expected, actual);
      }
      b.release();
    }

    assertNull(channel.readInbound());
  }

  @Test
  public void testMultipleNormalFramesVaryingSizes() {
    // Test multiple frames with varying sizes to ensure proper frame boundary detection
    EmbeddedChannel channel = new EmbeddedChannel(new ThriftHeaderFrameLengthBasedDecoder());
    int[] frameSizes = {50, 200, 1000, 75, 500, 25, 300, 150, 800, 100};

    for (int frame = 0; frame < frameSizes.length; frame++) {
      int payloadSize = frameSizes[frame];
      ByteBuf buf = Unpooled.buffer(payloadSize + 4);
      buf.writeInt(payloadSize);
      for (int i = 0; i < payloadSize; i++) {
        buf.writeByte((byte) ((frame + i) % 256));
      }
      channel.writeInbound(buf);
    }

    assertTrue(channel.finish());

    // Verify all frames were decoded correctly with expected sizes
    for (int frame = 0; frame < frameSizes.length; frame++) {
      ByteBuf b = channel.readInbound();
      assertNotNull("Frame " + frame + " should not be null", b);
      assertEquals("Frame " + frame + " size mismatch", frameSizes[frame], b.readableBytes());

      for (int i = 0; i < frameSizes[frame]; i++) {
        byte expected = (byte) ((frame + i) % 256);
        byte actual = b.readByte();
        assertEquals("Frame " + frame + " byte " + i + " mismatch", expected, actual);
      }
      b.release();
    }

    assertNull(channel.readInbound());
  }

  @Test
  public void testMultipleBigFrames() {
    // Test multiple big frames to ensure proper handling of 12-byte length field
    EmbeddedChannel channel = new EmbeddedChannel(new ThriftHeaderFrameLengthBasedDecoder());
    int numFrames = 5;
    int payloadSize = 0x40000000; // 1GiB, requires big frame format

    for (int frame = 0; frame < numFrames; frame++) {
      ByteBuf buf = Unpooled.buffer(payloadSize + 12);
      buf.writeInt(0x42494746); // BIG_FRAME_MAGIC
      buf.writeLong(payloadSize);
      for (int i = 0; i < payloadSize; i++) {
        buf.writeByte((byte) ((frame * 256 + i) % 256));
      }
      channel.writeInbound(buf);
    }

    assertTrue(channel.finish());

    // Verify all big frames were decoded correctly
    for (int frame = 0; frame < numFrames; frame++) {
      ByteBuf b = channel.readInbound();
      assertNotNull("Frame " + frame + " should not be null", b);
      assertEquals("Frame " + frame + " size mismatch", payloadSize, b.readableBytes());

      for (int i = 0; i < payloadSize; i++) {
        byte expected = (byte) ((frame * 256 + i) % 256);
        byte actual = b.readByte();
        assertEquals("Frame " + frame + " byte " + i + " mismatch", expected, actual);
      }
      b.release();
    }

    assertNull(channel.readInbound());
  }

  @Test
  public void testMixedNormalAndBigFrames() {
    // Test alternating between normal and big frames
    EmbeddedChannel channel = new EmbeddedChannel(new ThriftHeaderFrameLengthBasedDecoder());
    int normalPayloadSize = 1000;
    int bigPayloadSize = 0x40000000; // 1GiB

    for (int frame = 0; frame < 6; frame++) {
      boolean isBigFrame = (frame % 2 == 0);
      int payloadSize = isBigFrame ? bigPayloadSize : normalPayloadSize;
      int lengthFieldSize = isBigFrame ? 12 : 4;
      ByteBuf buf = Unpooled.buffer(payloadSize + lengthFieldSize);

      if (isBigFrame) {
        buf.writeInt(0x42494746); // BIG_FRAME_MAGIC
        buf.writeLong(payloadSize);
      } else {
        buf.writeInt(payloadSize);
      }

      for (int i = 0; i < payloadSize; i++) {
        buf.writeByte((byte) ((frame * 256 + i) % 256));
      }
      channel.writeInbound(buf);
    }

    assertTrue(channel.finish());

    // Verify all frames were decoded correctly
    for (int frame = 0; frame < 6; frame++) {
      boolean isBigFrame = (frame % 2 == 0);
      int expectedSize = isBigFrame ? bigPayloadSize : normalPayloadSize;

      ByteBuf b = channel.readInbound();
      assertNotNull("Frame " + frame + " should not be null", b);
      assertEquals("Frame " + frame + " size mismatch", expectedSize, b.readableBytes());

      for (int i = 0; i < expectedSize; i++) {
        byte expected = (byte) ((frame * 256 + i) % 256);
        byte actual = b.readByte();
        assertEquals("Frame " + frame + " byte " + i + " mismatch", expected, actual);
      }
      b.release();
    }

    assertNull(channel.readInbound());
  }

  @Test
  public void testManySmallFramesDoesNotAccumulateError() {
    // Regression test: Ensure that after processing many frames, the frame length
    // calculation doesn't accumulate the readerIndex. This would have caught the bug
    // where lengthFieldSize was incorrectly set to (readerIndex + 4) instead of just 4.
    EmbeddedChannel channel = new EmbeddedChannel(new ThriftHeaderFrameLengthBasedDecoder());
    int numFrames = 100;
    int payloadSize = 20;

    for (int frame = 0; frame < numFrames; frame++) {
      ByteBuf buf = Unpooled.buffer(payloadSize + 4);
      buf.writeInt(payloadSize);
      for (int i = 0; i < payloadSize; i++) {
        buf.writeByte((byte) (frame + i));
      }
      channel.writeInbound(buf);
    }

    assertTrue(channel.finish());

    // Verify all 100 frames were decoded correctly
    for (int frame = 0; frame < numFrames; frame++) {
      ByteBuf b = channel.readInbound();
      assertNotNull("Frame " + frame + " should not be null", b);
      assertEquals("Frame " + frame + " size mismatch", payloadSize, b.readableBytes());

      for (int i = 0; i < payloadSize; i++) {
        byte expected = (byte) (frame + i);
        byte actual = b.readByte();
        assertEquals("Frame " + frame + " byte " + i + " mismatch", expected, actual);
      }
      b.release();
    }

    assertNull(channel.readInbound());
  }
}
