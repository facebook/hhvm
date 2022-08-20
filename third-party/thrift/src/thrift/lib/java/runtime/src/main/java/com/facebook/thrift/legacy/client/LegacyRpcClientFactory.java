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

import static com.facebook.thrift.util.RpcClientUtils.getChannelClass;
import static com.facebook.thrift.util.RpcClientUtils.getSslContext;
import static io.netty.channel.ChannelOption.CONNECT_TIMEOUT_MILLIS;
import static java.util.Objects.requireNonNull;

import com.facebook.thrift.client.RpcClient;
import com.facebook.thrift.client.RpcClientFactory;
import com.facebook.thrift.client.ThriftClientConfig;
import com.facebook.thrift.protocol.TProtocolType;
import com.facebook.thrift.util.NettyUtil;
import com.facebook.thrift.util.resources.RpcResources;
import com.google.common.primitives.Ints;
import io.netty.bootstrap.Bootstrap;
import io.netty.channel.Channel;
import io.netty.channel.ChannelFuture;
import io.netty.channel.EventLoopGroup;
import io.netty.handler.ssl.SslContext;
import java.net.SocketAddress;
import reactor.core.publisher.Mono;

public final class LegacyRpcClientFactory implements RpcClientFactory {
  private final ThriftClientConfig nettyConfig;
  private final boolean forceExecutionOffEventLoop;

  public LegacyRpcClientFactory(final ThriftClientConfig config) {
    this.nettyConfig = requireNonNull(config, "swift netty config is null");
    this.forceExecutionOffEventLoop = RpcResources.isForceExecutionOffEventLoop();
  }

  @Override
  public Mono<RpcClient> createRpcClient(SocketAddress socketAddress) {
    requireNonNull(socketAddress, "socket address is null");

    return Mono.defer(
        () -> {
          try {
            final SslContext sslContext = getSslContext(nettyConfig, socketAddress);
            final EventLoopGroup group = RpcResources.getEventLoopGroup();
            final Bootstrap bootstrap = new Bootstrap();
            bootstrap.group(group);
            bootstrap.channel(getChannelClass(group, socketAddress));
            bootstrap.option(
                CONNECT_TIMEOUT_MILLIS,
                Ints.saturatedCast(nettyConfig.getConnectTimeout().toMillis()));
            bootstrap.handler(
                new ThriftClientInitializer(
                    sslContext,
                    nettyConfig.getTransport(),
                    TProtocolType.fromProtocolId(nettyConfig.getProtocol()),
                    nettyConfig.getMaxFrameSize(),
                    nettyConfig.getSocksProxy()));

            final ChannelFuture channelFuture = bootstrap.connect(socketAddress);
            final Channel channel = channelFuture.channel();
            return NettyUtil.toMono(channelFuture)
                .thenReturn(
                    new LegacyRpcClient(
                        channel,
                        RpcResources.getClientOffLoopScheduler(),
                        forceExecutionOffEventLoop,
                        nettyConfig));
          } catch (Throwable t) {
            return Mono.error(t);
          }
        });
  }
}
