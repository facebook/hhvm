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
    ByteBuf buf = Unpooled.buffer();
    buf.writeInt(12);
    for (int i = 0; i < 12; i++) {
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
}
