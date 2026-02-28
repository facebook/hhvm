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

package com.facebook.thrift.rsocket.transport.reactor.server;

import io.netty.buffer.ByteBuf;
import io.netty.channel.ChannelHandlerContext;
import io.netty.channel.ChannelInboundHandlerAdapter;
import io.rsocket.exceptions.RejectedSetupException;
import io.rsocket.frame.FrameHeaderCodec;
import io.rsocket.frame.FrameLengthCodec;
import io.rsocket.frame.FrameType;
import reactor.netty.Connection;

/**
 * Validate that the first frame coming to a server is a valid RSocket Frame. If it's not close the
 * server.
 */
public class RSocketProtocolDetector extends ChannelInboundHandlerAdapter {
  private final Connection connection;

  public RSocketProtocolDetector(Connection connection) {
    this.connection = connection;
  }

  public void channelRead(ChannelHandlerContext ctx, Object msg) {
    if (msg instanceof ByteBuf) {
      ByteBuf frame = (ByteBuf) msg;
      try {
        frame.markReaderIndex();
        frame.skipBytes(FrameLengthCodec.FRAME_LENGTH_SIZE);
        FrameHeaderCodec.ensureFrameType(FrameType.SETUP, frame);
        frame.resetReaderIndex();

        connection.removeHandler(RSocketProtocolDetector.class.getName());
      } catch (Throwable t) {
        ctx.fireExceptionCaught(new RejectedSetupException("Invalid Setup Frame", t));
      }
    }
  }
}
