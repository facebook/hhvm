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

package com.facebook.thrift.rsocket.transport.reactor.client;

import com.facebook.thrift.client.ThriftClientConfig;
import com.facebook.thrift.metadata.ThriftTransportType;
import com.facebook.thrift.rsocket.client.ConnectionClosureMetricsHandler;
import com.facebook.thrift.util.NettyUtil;
import com.facebook.thrift.util.RpcClientUtils;
import com.facebook.thrift.util.resources.RpcResources;
import io.netty.handler.ssl.SslContext;
import io.rsocket.DuplexConnection;
import io.rsocket.transport.ClientTransport;
import io.rsocket.transport.netty.TcpDuplexConnection;
import java.net.SocketAddress;
import reactor.core.publisher.Mono;
import reactor.netty.NettyPipeline;
import reactor.netty.resources.ConnectionProvider;
import reactor.netty.resources.LoopResources;
import reactor.netty.tcp.SslProvider;
import reactor.netty.tcp.TcpClient;

public class ReactorClientTransport implements ClientTransport {
  private final SocketAddress socketAddress;
  private final LoopResources loopResources;
  private final SslContext sslContext;
  private final boolean debugNettyPipeline;

  public ReactorClientTransport(SocketAddress socketAddress, ThriftClientConfig config) {
    this.socketAddress = socketAddress;
    this.loopResources = RpcResources.getLoopResources();
    this.sslContext = RpcClientUtils.getSslContext(config, socketAddress);
    this.debugNettyPipeline = config.getDebugNettyPipeline();
  }

  public ReactorClientTransport(
      SocketAddress socketAddress, ThriftClientConfig config, ThriftTransportType transportType) {
    this.socketAddress = socketAddress;
    this.loopResources = RpcResources.getLoopResources();
    this.sslContext = RpcClientUtils.getSslContext(config, socketAddress, transportType);
    this.debugNettyPipeline = config.getDebugNettyPipeline();
  }

  @Override
  public Mono<DuplexConnection> connect() {
    // Max number of connections to a specific socket address.
    TcpClient tcpClient =
        TcpClient.create(ConnectionProvider.create("thrift-connection-provider", 30000));

    if (debugNettyPipeline) {
      ConnectionClosureMetricsHandler handler = new ConnectionClosureMetricsHandler();
      tcpClient =
          tcpClient
              .observe(handler)
              .doOnChannelInit(
                  (connectionObserver, channel, remoteAddress) -> {
                    if (sslContext != null) {
                      channel
                          .pipeline()
                          .addAfter(
                              NettyPipeline.SslHandler, "connectionClosureMetricsHandler", handler);
                    } else {
                      channel.pipeline().addFirst("connectionClosureMetricsHandler", handler);
                    }
                  });
    }

    if (sslContext != null) {
      tcpClient = tcpClient.secure(SslProvider.builder().sslContext(sslContext).build());
    } else {
      tcpClient = tcpClient.noSSL();
    }

    return tcpClient
        .remoteAddress(() -> socketAddress)
        .runOn(loopResources)
        .doOnConnected(
            c ->
                c.addHandlerLast(NettyUtil.getDefaultThriftFlushConsolidationHandler())
                    .addHandlerLast(NettyUtil.getRSocketLengthFieldBasedFrameDecoder()))
        .connect()
        .map(conn -> new TcpDuplexConnection("client", conn));
  }
}
