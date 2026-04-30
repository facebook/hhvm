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

package com.facebook.thrift.transport.unified;

import static com.facebook.thrift.metadata.ThriftTransportType.HEADER;
import static com.facebook.thrift.metadata.ThriftTransportType.RSOCKET;
import static io.rsocket.transport.ServerTransport.ConnectionAcceptor;

import com.facebook.swift.service.ThriftServerConfig;
import com.facebook.thrift.legacy.server.ThriftHeaderFrameLengthBasedDecoder;
import com.facebook.thrift.metadata.ThriftTransportType;
import com.facebook.thrift.rsocket.server.ThriftSocketAcceptor;
import com.facebook.thrift.server.RpcServerHandler;
import com.facebook.thrift.server.ServerTransport;
import com.facebook.thrift.util.MetricsChannelDuplexHandler;
import com.facebook.thrift.util.NettyUtil;
import com.facebook.thrift.util.RpcServerUtils;
import com.facebook.thrift.util.SPINiftyMetrics;
import com.facebook.thrift.util.resources.RpcResources;
import io.netty.channel.Channel;
import io.netty.handler.codec.LengthFieldPrepender;
import io.netty.handler.logging.LogLevel;
import io.netty.handler.logging.LoggingHandler;
import io.netty.handler.ssl.SslHandler;
import io.rsocket.core.RSocketServer;
import io.rsocket.frame.FrameLengthCodec;
import io.rsocket.frame.decoder.PayloadDecoder;
import io.rsocket.transport.netty.TcpDuplexConnection;
import java.net.InetSocketAddress;
import java.net.SocketAddress;
import javax.annotation.Nullable;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import reactor.core.publisher.Mono;
import reactor.netty.ChannelPipelineConfigurer;
import reactor.netty.Connection;
import reactor.netty.ConnectionObserver;
import reactor.netty.DisposableServer;
import reactor.netty.tcp.SslProvider;
import reactor.netty.tcp.TcpServer;

public class UnifiedServerTransport implements ServerTransport {
  private static final Logger logger = LoggerFactory.getLogger(UnifiedServerTransport.class);

  private static final int MAX_FRAME_SIZE =
      Integer.parseInt(
          System.getProperty(
              "thrift.rsocket-max-frame-size", String.valueOf(FrameLengthCodec.FRAME_LENGTH_MASK)));

  private final DisposableServer disposableServer;
  private final SocketAddress boundAddress;
  private final SPINiftyMetrics metrics;

  UnifiedServerTransport(
      DisposableServer disposableServer, SocketAddress boundAddress, SPINiftyMetrics metrics) {
    this.disposableServer = disposableServer;
    this.boundAddress = boundAddress;
    this.metrics = metrics;
  }

  static Mono<UnifiedServerTransport> createNewInstance(
      SocketAddress socketAddress,
      RpcServerHandler rpcServerHandler,
      ThriftServerConfig config,
      SPINiftyMetrics metrics) {

    // Configure RSocket Server
    ConnectionAcceptor rsocket =
        RSocketServer.create(new ThriftSocketAcceptor(rpcServerHandler))
            .fragment(MAX_FRAME_SIZE)
            .payloadDecoder(PayloadDecoder.ZERO_COPY)
            .asConnectionAcceptor();

    // Configure Thrift Header Server
    ThriftConnectionAcceptor thrift =
        new ThriftConnectionAcceptor(rpcServerHandler, config.getTaskExpirationTimeout());

    return TcpServer.create()
        .secure(SslProvider.builder().sslContext(RpcServerUtils.getSslContext(config)).build())
        .doOnChannelInit(new ThriftChannelInitializer(config, metrics))
        .doOnConnection(
            connection -> {
              ThriftTransportType protocol = getProtocol(connection);

              // If ALPN was not negotiated (client sent no ALPN extension), fall back
              // to Header protocol, matching the C++ ThriftServer behavior.
              if (protocol != HEADER && protocol != RSOCKET) {
                logger.debug(
                    "No supported protocol negotiated via ALPN (got: {}), falling back to HEADER",
                    protocol);
                protocol = HEADER;
              }

              // Add after SSL, before reactive bridge
              connection
                  .addHandlerLast(NettyUtil.getDefaultThriftFlushConsolidationHandler())
                  .addHandlerLast(new LoggingHandler(LogLevel.TRACE));

              // Add protocol-specific handlers
              if (protocol == RSOCKET) {
                connection.addHandlerLast(NettyUtil.getRSocketLengthFieldBasedFrameDecoder());
                configureRsocket(connection, rsocket);
              } else {
                int maxFrameLengthBytes =
                    (int) Math.min(config.getMaxFrameSize().toBytes(), Integer.MAX_VALUE);
                connection
                    .addHandlerLast(new ThriftHeaderFrameLengthBasedDecoder(maxFrameLengthBytes))
                    .addHandlerLast(new LengthFieldPrepender(Integer.BYTES));
                configureThrift(connection, thrift);
              }
            })
        .runOn(RpcResources.getEventLoopGroup())
        .bindAddress(() -> getBindAddress(socketAddress, config))
        .bind()
        .map(
            disposableServer ->
                new UnifiedServerTransport(disposableServer, disposableServer.address(), metrics));
  }

  /**
   * Match legacy transport behavior: when bindAddressEnabled is false, bind to the wildcard address
   * (all interfaces) using only the port, instead of binding to "localhost".
   */
  private static SocketAddress getBindAddress(
      SocketAddress socketAddress, ThriftServerConfig config) {
    if (!config.isBindAddressEnabled() && socketAddress instanceof InetSocketAddress) {
      return new InetSocketAddress(((InetSocketAddress) socketAddress).getPort());
    }
    return socketAddress;
  }

  private static ThriftTransportType getProtocol(Connection connection) {
    SslHandler sslHandler = connection.channel().pipeline().get(SslHandler.class);
    if (sslHandler == null) {
      throw new IllegalStateException(
          "cannot find an SslHandler in the pipeline (required for "
              + "application-level protocol negotiation)");
    }
    return ThriftTransportType.fromProtocol(sslHandler.applicationProtocol());
  }

  private static void configureThrift(Connection connection, ThriftConnectionAcceptor thrift) {
    thrift
        .apply(connection)
        .onErrorResume(
            throwable -> {
              if (!RpcServerUtils.isSslCloseNotify(throwable)) {
                logger.error(
                    "Error in ThriftConnectionAcceptor, closing connection. Remote: {}",
                    connection.channel().remoteAddress(),
                    throwable);
              }
              return Mono.empty();
            })
        .subscribe(connection.disposeSubscriber());
  }

  private static void configureRsocket(Connection connection, ConnectionAcceptor rsocket) {
    rsocket
        .apply(new TcpDuplexConnection("server", connection))
        .then(Mono.<Void>never())
        .subscribe(connection.disposeSubscriber());
  }

  @Override
  public SocketAddress getAddress() {
    return boundAddress;
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

  private static class ThriftChannelInitializer implements ChannelPipelineConfigurer {
    private final Logger logger = LoggerFactory.getLogger(ThriftChannelInitializer.class);

    private final int connectionLimit;
    private final SPINiftyMetrics metrics;

    public ThriftChannelInitializer(ThriftServerConfig config, SPINiftyMetrics metrics) {
      connectionLimit = config.getConnectionLimit();
      this.metrics = metrics;
    }

    @Override
    public void onChannelInit(
        ConnectionObserver connectionObserver,
        Channel channel,
        @Nullable SocketAddress remoteAddress) {
      if (connectionLimit > 0) {
        final int channelCount = metrics.getChannelCount();
        if (channelCount >= connectionLimit) {
          metrics.incrementRejectedConnections();
          logger.error(
              "Closing connection from {} because limit {} has been exceeded. Current count: {}",
              remoteAddress,
              connectionLimit,
              channelCount);
          channel.close();
          return;
        }
      }

      // MetricsChannelDuplexHandler handles incrementing metrics for channel count on active...
      // add after we reject to avoid increment or decrement on a connection that is going to be
      // disposed of
      Connection.from(channel).addHandlerLast(new MetricsChannelDuplexHandler(metrics));
    }
  }
}
