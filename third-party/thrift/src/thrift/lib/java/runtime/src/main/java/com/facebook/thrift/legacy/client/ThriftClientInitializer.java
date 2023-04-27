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

package com.facebook.thrift.legacy.client;

import com.facebook.thrift.legacy.codec.LegacyTransportType;
import com.facebook.thrift.protocol.TProtocolType;
import io.airlift.units.DataSize;
import io.netty.channel.Channel;
import io.netty.channel.ChannelInitializer;
import io.netty.channel.ChannelPipeline;
import io.netty.handler.proxy.Socks4ProxyHandler;
import io.netty.handler.ssl.SslContext;
import java.net.SocketAddress;
import java.util.Optional;

final class ThriftClientInitializer extends ChannelInitializer<Channel> {
  private final SslContext sslContext;
  private final LegacyTransportType transport;
  private final TProtocolType protocol;
  private final DataSize maxFrameSize;
  private final SocketAddress socksProxyAddress;

  ThriftClientInitializer(
      SslContext sslContext,
      LegacyTransportType transport,
      TProtocolType protocol,
      DataSize maxFrameSize,
      SocketAddress socksProxyAddress) {
    this.sslContext = sslContext;
    this.transport = transport;
    this.protocol = protocol;
    this.maxFrameSize = maxFrameSize;
    this.socksProxyAddress = socksProxyAddress;
  }

  @Override
  protected void initChannel(Channel channel) {
    ChannelPipeline pipeline = channel.pipeline();

    if (sslContext != null) {
      pipeline.addLast(sslContext.newHandler(channel.alloc()));
    }

    if (socksProxyAddress != null) {
      pipeline.addLast(new Socks4ProxyHandler(socksProxyAddress));
    }

    transport.addFrameHandlers(pipeline, Optional.of(protocol), maxFrameSize, true);
  }
}
