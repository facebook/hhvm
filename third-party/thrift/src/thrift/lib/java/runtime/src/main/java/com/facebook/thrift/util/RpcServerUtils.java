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

import static com.facebook.thrift.util.PlatformUtils.getOS;

import com.facebook.nifty.core.RequestContext;
import com.facebook.nifty.ssl.SslSession;
import com.facebook.swift.service.ThriftServerConfig;
import com.facebook.thrift.legacy.server.LegacyServerTransportFactory;
import com.facebook.thrift.metadata.ThriftTransportType;
import com.facebook.thrift.rsocket.server.RSocketServerTransportFactory;
import com.facebook.thrift.server.RpcServerHandler;
import com.facebook.thrift.server.ServerTransport;
import com.facebook.thrift.server.ServerTransportFactory;
import io.netty.channel.EventLoopGroup;
import io.netty.channel.ServerChannel;
import io.netty.channel.epoll.Epoll;
import io.netty.channel.epoll.EpollEventLoopGroup;
import io.netty.channel.epoll.EpollServerDomainSocketChannel;
import io.netty.channel.epoll.EpollServerSocketChannel;
import io.netty.channel.kqueue.KQueue;
import io.netty.channel.kqueue.KQueueEventLoopGroup;
import io.netty.channel.kqueue.KQueueServerDomainSocketChannel;
import io.netty.channel.kqueue.KQueueServerSocketChannel;
import io.netty.channel.socket.nio.NioServerSocketChannel;
import io.netty.channel.unix.DomainSocketAddress;
import io.netty.handler.ssl.ApplicationProtocolConfig;
import io.netty.handler.ssl.SslContext;
import io.netty.handler.ssl.SslContextBuilder;
import io.netty.handler.ssl.SslHandler;
import io.netty.handler.ssl.SslProvider;
import java.io.FileInputStream;
import java.net.InetSocketAddress;
import java.net.ServerSocket;
import java.net.SocketAddress;
import java.security.cert.X509Certificate;
import java.util.Objects;
import javax.net.ssl.SSLSession;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import reactor.core.Exceptions;
import reactor.core.publisher.Flux;
import reactor.core.publisher.Mono;

public final class RpcServerUtils {

  private static final Logger LOG = LoggerFactory.getLogger(RpcServerUtils.class);

  private RpcServerUtils() {}

  /**
   * Returns ServerChannel Class from eventLoopGroup and socketAddress. Throws
   * UnsupportedOperationException if the combination is invalid.
   *
   * @param group
   * @param socketAddress
   * @return Channel class
   */
  public static Class<? extends ServerChannel> getChannelClass(
      EventLoopGroup group, SocketAddress socketAddress) {
    if (socketAddress instanceof InetSocketAddress) {
      if (group instanceof EpollEventLoopGroup) {
        return EpollServerSocketChannel.class;
      }
      if (group instanceof KQueueEventLoopGroup) {
        return KQueueServerSocketChannel.class;
      }
      return NioServerSocketChannel.class;
    }
    if (socketAddress instanceof DomainSocketAddress) {
      if (group instanceof EpollEventLoopGroup) {
        return EpollServerDomainSocketChannel.class;
      } else if (getOS() == PlatformUtils.OS.LINUX) {
        Throwable unavailabilityCause = Epoll.unavailabilityCause();
        String errorMsg =
            unavailabilityCause == null
                ? "unavailabilityCause is null"
                : unavailabilityCause.getMessage();
        throw new UnsupportedOperationException(
            String.format(
                "Unsupported combination of EventLoopGroup-{%s} & SocketAddress-{%s}. Likely due to"
                    + " system support for Epoll unavailable due to {%s}.",
                group.getClass(), socketAddress.getClass(), errorMsg),
            unavailabilityCause);
      }
      if (group instanceof KQueueEventLoopGroup) {
        return KQueueServerDomainSocketChannel.class;
      } else if (getOS() == PlatformUtils.OS.MAC) {
        Throwable unavailabilityCause = KQueue.unavailabilityCause();
        String errorMsg =
            unavailabilityCause == null
                ? "unavailabilityCause is null"
                : unavailabilityCause.getMessage();
        throw new UnsupportedOperationException(
            String.format(
                "Unsupported combination of EventLoopGroup-{%s} & SocketAddress-{%s}. Likely due to"
                    + " system support for Kqueue unavailable due to {%s}.",
                group.getClass(), socketAddress.getClass(), errorMsg),
            unavailabilityCause);
      }
    }
    throw new UnsupportedOperationException(
        String.format(
            "Unsupported combination of EventLoopGroup-{%s} & SocketAddress-{%s}",
            group.getClass(), socketAddress.getClass()));
  }

  /**
   * Creates SslContext based on ThriftServerConfig. Returns SSLContext or throws exception due to
   * an error while building sslContext
   *
   * @param config
   * @return: SSL Context
   */
  public static SslContext getSslContext(ThriftServerConfig config) {

    try (FileInputStream keyFile = new FileInputStream(config.getKeyFile());
        FileInputStream certFile = new FileInputStream(config.getCertFile());
        FileInputStream caFile = new FileInputStream(config.getCAFile())) {
      SslProvider sslProvider = config.getEnableJdkSsl() ? SslProvider.JDK : SslProvider.OPENSSL;

      SslContextBuilder contextBuilder =
          SslContextBuilder.forServer(certFile, keyFile)
              .sslProvider(sslProvider)
              .trustManager(caFile)
              .sessionCacheSize(config.getSessionCacheSize());

      if (config.isEnableAlpn()) {
        contextBuilder.applicationProtocolConfig(
            new ApplicationProtocolConfig(
                ApplicationProtocolConfig.Protocol.ALPN,
                config.getEnableJdkSsl()
                    ? ApplicationProtocolConfig.SelectorFailureBehavior.FATAL_ALERT
                    : ApplicationProtocolConfig.SelectorFailureBehavior.CHOOSE_MY_LAST_PROTOCOL,
                config.getEnableJdkSsl()
                    ? ApplicationProtocolConfig.SelectedListenerFailureBehavior.FATAL_ALERT
                    : ApplicationProtocolConfig.SelectedListenerFailureBehavior
                        .CHOOSE_MY_LAST_PROTOCOL,
                ThriftTransportType.RSOCKET.protocol(),
                ThriftTransportType.HEADER.protocol(),
                ThriftTransportType.FRAMED.protocol()));
      }

      if (config.getClientAuth() != null) {
        contextBuilder.clientAuth(config.getClientAuth());
      }
      return contextBuilder.build();
    } catch (Exception e) {
      LOG.error("error getting SslContext", e);
      throw Exceptions.propagate(e);
    }
  }

  /**
   * Creates Nifty SslSession from Netty 4 SSL Handler object. Session fetch throws
   * SSLUnverifiedException interaction is not ssl based
   *
   * @param sslHandler: Netty 4 SSLHandler
   * @return: Nifty SslSession
   */
  public static SslSession getThriftSslSession(SslHandler sslHandler) {
    try {
      if (sslHandler != null && sslHandler.engine() != null) {
        X509Certificate certificate = null;
        SSLSession sslSession = sslHandler.engine().getSession();
        if (sslSession.getPeerCertificates() != null
            && sslSession.getPeerCertificates().length > 0
            && sslSession.getPeerCertificates()[0] instanceof X509Certificate) {
          certificate = (X509Certificate) sslSession.getPeerCertificates()[0];
        }
        return new SslSession(
            "",
            "",
            sslSession.getProtocol(),
            sslSession.getCipherSuite(),
            sslSession.getCreationTime(),
            certificate);
      }
    } catch (Exception e) {
      LOG.info("No SSL info received from client");
    }
    return null;
  }

  public static Mono<? extends ServerTransport> createServerTransport(
      ThriftServerConfig config, TransportType transportType, RpcServerHandler rpcServerHandler) {
    Objects.requireNonNull(config);
    Objects.requireNonNull(transportType);

    ServerTransportFactory<? extends ServerTransport> transportFactory;
    switch (transportType) {
      case RSOCKET:
        transportFactory = createRSocketTransport(config);
        break;
      case THEADER:
        transportFactory = createTHeaderTransport(config);
        break;
      default:
        throw new IllegalArgumentException("unknown transport type: " + transportType);
    }

    final int port = config.getPort() == 0 ? RpcServerUtils.findFreePort() : config.getPort();

    SPINiftyMetrics serverMetrics = new SPINiftyMetrics();
    if (config.isEnableUDS()) {
      return transportFactory.createServerTransport(
          new DomainSocketAddress(config.getUdsPath()), rpcServerHandler, serverMetrics);
    } else if (config.isBindAddressEnabled()) {
      return transportFactory.createServerTransport(
          new InetSocketAddress(config.getBindAddress(), port), rpcServerHandler, serverMetrics);
    } else {
      return transportFactory.createServerTransport(
          new InetSocketAddress("localhost", port), rpcServerHandler, serverMetrics);
    }
  }

  private static ServerTransportFactory<? extends ServerTransport> createTHeaderTransport(
      ThriftServerConfig config) {
    return new LegacyServerTransportFactory(config);
  }

  private static ServerTransportFactory<? extends ServerTransport> createRSocketTransport(
      ThriftServerConfig config) {
    return new RSocketServerTransportFactory(config);
  }

  public static int findFreePort() {
    try (ServerSocket s = new ServerSocket(0)) {
      return s.getLocalPort();
    } catch (Exception e) {
      throw Exceptions.propagate(e);
    }
  }

  public static <T> Mono<T> decorateWithRequestContext(
      com.facebook.nifty.core.RequestContext requestContext, Mono<T> mono) {
    return mono.contextWrite(context -> RequestContext.toContext(context, requestContext));
  }

  public static <T> Flux<T> decorateWithRequestContext(
      com.facebook.nifty.core.RequestContext requestContext, Flux<T> flux) {
    return flux.contextWrite(context -> RequestContext.toContext(context, requestContext));
  }
}
