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

import com.facebook.swift.service.ThriftServerConfig;
import com.facebook.thrift.util.MetricsChannelDuplexHandler;
import com.facebook.thrift.util.NettyUtil;
import com.facebook.thrift.util.RpcServerUtils;
import com.facebook.thrift.util.SPINiftyMetrics;
import com.facebook.thrift.util.resources.RpcResources;
import io.netty.channel.EventLoopGroup;
import io.netty.handler.ssl.SslContext;
import io.rsocket.transport.ServerTransport;
import io.rsocket.transport.netty.TcpDuplexConnection;
import java.net.SocketAddress;
import java.util.Objects;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import reactor.core.publisher.Mono;
import reactor.netty.DisposableServer;
import reactor.netty.tcp.SslProvider;
import reactor.netty.tcp.TcpServer;

public class ReactorServerTransport implements ServerTransport<ReactorServerCloseable> {
  private Logger LOGGER = LoggerFactory.getLogger(ReactorServerTransport.class);
  final SocketAddress socketAddress;
  final ThriftServerConfig config;

  final SPINiftyMetrics metrics;

  public ReactorServerTransport(
      SocketAddress socketAddress, ThriftServerConfig config, SPINiftyMetrics metrics) {
    this.socketAddress = Objects.requireNonNull(socketAddress);
    this.config = Objects.requireNonNull(config);
    this.metrics = Objects.requireNonNull(metrics);
  }

  @Override
  public Mono<ReactorServerCloseable> start(final ConnectionAcceptor acceptor) {
    EventLoopGroup eventLoopGroup = RpcResources.getEventLoopGroup();

    TcpServer tcpServer =
        TcpServer.create()
            .doOnConnection(
                connection -> {
                  connection
                      .addHandlerLast(NettyUtil.getDefaultThriftFlushConsolidationHandler())
                      .addHandlerLast(new MetricsChannelDuplexHandler(metrics))
                      .addHandlerLast(NettyUtil.getRSocketLengthFieldBasedFrameDecoder());

                  acceptor
                      .apply(new TcpDuplexConnection("server", connection))
                      .then(Mono.<Void>never())
                      .subscribe(connection.disposeSubscriber());
                })
            .runOn(eventLoopGroup);

    if (config.isSslEnabled() && !config.isEnableUDS()) {
      SslContext sslContext = RpcServerUtils.getSslContext(config);
      tcpServer.secure(SslProvider.builder().sslContext(sslContext).build());
    } else {
      tcpServer.noSSL();
    }

    return tcpServer.bindAddress(() -> socketAddress).bind().map(this::handleBind);
  }

  private ReactorServerCloseable handleBind(DisposableServer disposableServer) {
    return new ReactorServerCloseable() {
      @Override
      public SocketAddress getAddress() {
        return socketAddress;
      }

      @Override
      public SPINiftyMetrics getMetrics() {
        return metrics;
      }

      @Override
      public Mono<Void> onClose() {
        return disposableServer.onDispose();
      }

      @Override
      public void dispose() {
        disposableServer.dispose();
      }
    };
  }
}
