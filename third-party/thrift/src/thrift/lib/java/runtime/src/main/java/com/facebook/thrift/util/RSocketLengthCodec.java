/*
 * Copyright 2015-2018 the original author or authors.
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
package com.facebook.thrift.util;

import static io.rsocket.frame.FrameLengthCodec.FRAME_LENGTH_MASK;
import static io.rsocket.frame.FrameLengthCodec.FRAME_LENGTH_SIZE;

import io.netty.buffer.ByteBuf;
import io.netty.channel.ChannelHandlerContext;
import io.netty.handler.codec.LengthFieldBasedFrameDecoder;

/**
 * An extension to the Netty {@link LengthFieldBasedFrameDecoder} that encapsulates the
 * RSocket-specific frame length header details.
 */
public final class RSocketLengthCodec extends LengthFieldBasedFrameDecoder {

  /** Creates a new instance of the decoder, specifying the RSocket frame length header size. */
  public RSocketLengthCodec() {
    this(FRAME_LENGTH_MASK);
  }

  /**
   * Creates a new instance of the decoder, specifying the RSocket frame length header size.
   *
   * @param maxFrameLength maximum allowed frame length for incoming rsocket frames
   */
  public RSocketLengthCodec(int maxFrameLength) {
    super(maxFrameLength, 0, FRAME_LENGTH_SIZE, 0, 0);
  }

  /**
   * Simplified non-netty focused decode usage.
   *
   * @param in the input buffer to read data from.
   * @return decoded buffer or null is none available.
   * @see #decode(ChannelHandlerContext, ByteBuf)
   * @throws Exception if any error happens.
   */
  public Object decode(ByteBuf in) throws Exception {
    return decode(null, in);
  }
}
