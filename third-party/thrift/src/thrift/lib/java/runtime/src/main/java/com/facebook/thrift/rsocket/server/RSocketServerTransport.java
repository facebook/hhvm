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

package com.facebook.thrift.rsocket.server;

import static java.util.Objects.requireNonNull;

import com.facebook.swift.service.ThriftServerConfig;
import com.facebook.thrift.server.RpcServerHandler;
import com.facebook.thrift.server.ServerTransport;
import com.facebook.thrift.util.MetricsChannelDuplexHandler;
import com.facebook.thrift.util.NettyUtil;
import com.facebook.thrift.util.RpcServerUtils;
import com.facebook.thrift.util.SPINiftyMetrics;
import com.facebook.thrift.util.resources.RpcResources;
import io.netty.handler.ssl.SslContext;
import io.rsocket.core.RSocketServer;
import io.rsocket.frame.FrameLengthCodec;
import io.rsocket.frame.decoder.PayloadDecoder;
import io.rsocket.transport.ServerTransport.ConnectionAcceptor;
import io.rsocket.transport.netty.TcpDuplexConnection;
import java.net.SocketAddress;
import reactor.core.publisher.Mono;
import reactor.netty.DisposableServer;
import reactor.netty.tcp.SslProvider;
import reactor.netty.tcp.TcpServer;

public class RSocketServerTransport implements ServerTransport {

  private static final int MAX_FRAME_SIZE =
      Integer.parseInt(
          System.getProperty(
              "thrift.rsocket-max-frame-size", String.valueOf(FrameLengthCodec.FRAME_LENGTH_MASK)));

  private final DisposableServer disposableServer;
  private final SocketAddress address;
  private final SPINiftyMetrics metrics;

  RSocketServerTransport(
      DisposableServer disposableServer, SocketAddress address, SPINiftyMetrics metrics) {
    this.disposableServer = disposableServer;
    this.address = address;
    this.metrics = metrics;
  }

  static Mono<RSocketServerTransport> createInstance(
      SocketAddress socketAddress,
      RpcServerHandler rpcServerHandler,
      ThriftServerConfig config,
      SPINiftyMetrics serverMetrics) {
    try {
      requireNonNull(rpcServerHandler, "methodInvoker is null");
      requireNonNull(config, "config is null");

      TcpServer tcpServer =
          TcpServer.create()
              .doOnConnection(
                  connection -> {
                    connection
                        .addHandlerLast(NettyUtil.getDefaultThriftFlushConsolidationHandler())
                        .addHandlerLast(new MetricsChannelDuplexHandler(serverMetrics))
                        .addHandlerLast(NettyUtil.getRSocketLengthFieldBasedFrameDecoder());

                    ConnectionAcceptor acceptor =
                        RSocketServer.create(
                                new ThriftSocketAcceptor(
                                    rpcServerHandler,
                                    RpcServerUtils.getNiftyConnectionContext(connection)))
                            .fragment(MAX_FRAME_SIZE)
                            .payloadDecoder(PayloadDecoder.ZERO_COPY)
                            .asConnectionAcceptor();

                    acceptor
                        .apply(new TcpDuplexConnection("server", connection))
                        .then(Mono.<Void>never())
                        .subscribe(connection.disposeSubscriber());
                  })
              .runOn(RpcResources.getEventLoopGroup());

      if (config.isSslEnabled() && !config.isEnableUDS()) {
        SslContext sslContext = RpcServerUtils.getSslContext(config);
        tcpServer = tcpServer.secure(SslProvider.builder().sslContext(sslContext).build());
      } else {
        tcpServer = tcpServer.noSSL();
      }

      return tcpServer
          .bindAddress(() -> socketAddress)
          .bind()
          .map(server -> new RSocketServerTransport(server, socketAddress, serverMetrics));
    } catch (Exception e) {
      return Mono.error(e);
    }
  }

  @Override
  public SocketAddress getAddress() {
    return address;
  }

  @Override
  public Mono<Void> onClose() {
    return disposableServer.onDispose();
  }

  @Override
  public SPINiftyMetrics getNiftyMetrics() {
    return metrics;
  }

  @Override
  public void dispose() {
    disposableServer.dispose();
  }

  @Override
  public boolean isDisposed() {
    return disposableServer.isDisposed();
  }
}
