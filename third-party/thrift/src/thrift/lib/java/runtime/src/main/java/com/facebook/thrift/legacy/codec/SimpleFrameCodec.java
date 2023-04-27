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

import static java.util.Objects.requireNonNull;

import com.facebook.thrift.protocol.TProtocolType;
import io.netty.buffer.ByteBuf;
import io.netty.channel.ChannelDuplexHandler;
import io.netty.channel.ChannelHandlerContext;
import io.netty.channel.ChannelPromise;
import java.util.Collections;
import org.apache.thrift.protocol.TMessage;

public class SimpleFrameCodec extends ChannelDuplexHandler {
  private final LegacyTransportType transport;
  private final TProtocolType protocol;
  private final boolean assumeClientsSupportOutOfOrderResponses;

  public SimpleFrameCodec(
      LegacyTransportType transport,
      TProtocolType protocol,
      boolean assumeClientsSupportOutOfOrderResponses) {
    this.transport = requireNonNull(transport, "transport is null");
    this.protocol = requireNonNull(protocol, "protocol is null");
    this.assumeClientsSupportOutOfOrderResponses = assumeClientsSupportOutOfOrderResponses;
  }

  @Override
  public void channelRead(ChannelHandlerContext context, Object message) throws Exception {
    if (message instanceof ByteBuf) {
      ByteBuf byteBuf = (ByteBuf) message;
      message = decodeMessage(byteBuf);
    }

    context.fireChannelRead(message);
  }

  private ThriftFrame decodeMessage(ByteBuf byteBuf) {
    byteBuf.markReaderIndex();
    TMessage message = protocol.apply(byteBuf).readMessageBegin();
    int seqid = message.seqid;
    byteBuf.resetReaderIndex();
    return new ThriftFrame(
        seqid,
        byteBuf.retain(),
        Collections.emptyMap(),
        Collections.emptyMap(),
        transport,
        protocol,
        assumeClientsSupportOutOfOrderResponses);
  }

  @Override
  public void write(ChannelHandlerContext context, Object message, ChannelPromise promise) {
    if (message instanceof ThriftFrame) {
      // strip the underlying message from the frame
      message = ((ThriftFrame) message).getMessage();
    }
    context.writeAndFlush(message, promise);
  }
}
