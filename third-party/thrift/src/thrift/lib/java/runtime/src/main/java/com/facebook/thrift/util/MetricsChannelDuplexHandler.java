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

package com.facebook.thrift.util;

import io.netty.buffer.ByteBuf;
import io.netty.channel.ChannelDuplexHandler;
import io.netty.channel.ChannelHandlerContext;
import io.netty.channel.ChannelPromise;

/**
 * ChannelDuplexHandler that captures metrics about bytes read, and written. It also captures
 * metrics about the number of open channels.
 */
public class MetricsChannelDuplexHandler extends ChannelDuplexHandler {
  private final SPINiftyMetrics spiNiftyMetrics;

  public MetricsChannelDuplexHandler(SPINiftyMetrics spiNiftyMetrics) {
    this.spiNiftyMetrics = spiNiftyMetrics;
  }

  @Override
  public void write(ChannelHandlerContext ctx, Object msg, ChannelPromise promise)
      throws Exception {
    if (msg instanceof ByteBuf) {
      spiNiftyMetrics.incrementBytesWrite(((ByteBuf) msg).readableBytes());
    }
    ctx.write(msg, promise);
  }

  @Override
  public void channelRead(ChannelHandlerContext ctx, Object msg) throws Exception {
    if (msg instanceof ByteBuf) {
      spiNiftyMetrics.incrementBytesRead(((ByteBuf) msg).readableBytes());
    }
    ctx.fireChannelRead(msg);
  }

  @Override
  public void channelActive(ChannelHandlerContext ctx) throws Exception {
    spiNiftyMetrics.incrementChannelCount();
    ctx.fireChannelActive();
  }

  @Override
  public void channelInactive(ChannelHandlerContext ctx) throws Exception {
    spiNiftyMetrics.decrementChannelCount();
    spiNiftyMetrics.incrementDroppedConnections();
    ctx.fireChannelInactive();
  }
}
