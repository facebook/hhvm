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

import static com.facebook.thrift.util.RpcServerUtils.getChannelClass;
import static com.facebook.thrift.util.RpcServerUtils.getSslContext;
import static java.util.Objects.requireNonNull;

import com.facebook.swift.service.ThriftServerConfig;
import com.facebook.thrift.server.RpcServerHandler;
import com.facebook.thrift.server.ServerTransport;
import com.facebook.thrift.util.NettyUtil;
import com.facebook.thrift.util.SPINiftyMetrics;
import com.facebook.thrift.util.resources.RpcResources;
import io.netty.bootstrap.ServerBootstrap;
import io.netty.channel.Channel;
import io.netty.channel.ChannelFuture;
import io.netty.channel.ChannelOption;
import io.netty.channel.EventLoopGroup;
import io.netty.channel.ServerChannel;
import io.netty.handler.ssl.SslContext;
import java.net.InetSocketAddress;
import java.net.SocketAddress;
import java.util.Optional;
import java.util.function.Supplier;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import reactor.core.publisher.Mono;
import reactor.core.publisher.MonoProcessor;

public class LegacyServerTransport implements ServerTransport {
  private static final Logger logger = LoggerFactory.getLogger(LegacyServerTransport.class);
  private final Channel channel;
  private final ChannelFuture closeFuture;
  private final Mono<Void> onClose;
  private final SPINiftyMetrics metrics;

  LegacyServerTransport(Channel channel, SPINiftyMetrics metrics, Mono<Void> onClose) {
    this.channel = channel;
    this.onClose = onClose;
    this.closeFuture = channel.closeFuture();
    this.metrics = metrics;
  }

  static Mono<LegacyServerTransport> createNewInstance(
      SocketAddress bindAddress,
      RpcServerHandler rpcServerHandler,
      ThriftServerConfig config,
      SPINiftyMetrics serverMetrics) {

    return Mono.defer(
        () -> {
          requireNonNull(rpcServerHandler, "methodInvoker is null");
          requireNonNull(config, "config is null");

          MonoProcessor<Void> onClose = MonoProcessor.create();

          Optional<Supplier<SslContext>> sslContext = Optional.empty();
          if (config.isSslEnabled() && bindAddress instanceof InetSocketAddress) {
            sslContext = Optional.of(() -> getSslContext(config));
          }

          ThriftServerInitializer serverInitializer =
              new ThriftServerInitializer(
                  rpcServerHandler,
                  config.getMaxFrameSize(),
                  config.getTaskExpirationTimeout(),
                  sslContext,
                  config.isAllowPlaintext(),
                  config.isAssumeClientsSupportOutOfOrderResponses(),
                  serverMetrics,
                  config.getConnectionLimit());

          EventLoopGroup group = RpcResources.getEventLoopGroup();

          Class<? extends ServerChannel> channelClass = getChannelClass(group, bindAddress);

          ServerBootstrap bootstrap =
              new ServerBootstrap()
                  .group(group)
                  .channel(channelClass)
                  .childHandler(serverInitializer)
                  .option(ChannelOption.SO_BACKLOG, config.getAcceptBacklog())
                  .validate();
          // SO_KEEPALIVE is not required in UDS Servers
          if (!config.isEnableUDS()) {
            bootstrap.childOption(ChannelOption.SO_KEEPALIVE, true);
          }

          ChannelFuture bind;
          if (config.isEnableUDS()) {
            bind = bootstrap.bind(bindAddress);
          } else {
            bind =
                config.isBindAddressEnabled()
                    ? bootstrap.bind(
                        ((InetSocketAddress) bindAddress).getHostString(),
                        ((InetSocketAddress) bindAddress).getPort())
                    : bootstrap.bind(((InetSocketAddress) bindAddress).getPort());
          }

          Channel channel = bind.channel();

          NettyUtil.toMono(channel.closeFuture()).subscribe(onClose);

          return NettyUtil.toMono(bind)
              .thenReturn(new LegacyServerTransport(channel, serverMetrics, onClose));
        });
  }

  @Override
  public SocketAddress getAddress() {
    return channel.localAddress();
  }

  @Override
  public Mono<Void> onClose() {
    return onClose;
  }

  @Override
  public void dispose() {
    channel.close();
  }

  @Override
  public boolean isDisposed() {
    return closeFuture.isVoid();
  }

  @Override
  public SPINiftyMetrics getNiftyMetrics() {
    return metrics;
  }
}
