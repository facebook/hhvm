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
import static org.apache.thrift.Constants.K_ROCKET_PROTOCOL_KEY;

import com.facebook.thrift.client.ClientBuilder;
import com.facebook.thrift.client.ThriftClientConfig;
import com.facebook.thrift.legacy.codec.LegacyTransportType;
import com.facebook.thrift.metadata.ThriftTransportType;
import com.facebook.thrift.model.StreamResponse;
import com.facebook.thrift.payload.ClientRequestPayload;
import com.facebook.thrift.payload.ClientResponsePayload;
import com.facebook.thrift.payload.Reader;
import com.facebook.thrift.protocol.ByteBufTProtocol;
import com.facebook.thrift.protocol.TProtocolType;
import com.facebook.thrift.util.resources.RpcResources;
import io.netty.buffer.ByteBuf;
import io.netty.buffer.PooledByteBufAllocator;
import io.netty.buffer.Unpooled;
import io.netty.channel.Channel;
import io.netty.channel.EventLoopGroup;
import io.netty.channel.epoll.Epoll;
import io.netty.channel.epoll.EpollDomainSocketChannel;
import io.netty.channel.epoll.EpollEventLoopGroup;
import io.netty.channel.epoll.EpollSocketChannel;
import io.netty.channel.kqueue.KQueue;
import io.netty.channel.kqueue.KQueueDomainSocketChannel;
import io.netty.channel.kqueue.KQueueEventLoopGroup;
import io.netty.channel.kqueue.KQueueSocketChannel;
import io.netty.channel.socket.nio.NioSocketChannel;
import io.netty.channel.unix.DomainSocketAddress;
import io.netty.handler.ssl.ApplicationProtocolConfig;
import io.netty.handler.ssl.SslContext;
import io.netty.handler.ssl.SslContextBuilder;
import io.netty.handler.ssl.SslProvider;
import io.rsocket.Payload;
import io.rsocket.core.RSocketConnector;
import io.rsocket.frame.decoder.PayloadDecoder;
import io.rsocket.util.ByteBufPayload;
import java.io.FileInputStream;
import java.lang.invoke.MethodHandle;
import java.lang.invoke.MethodHandles;
import java.lang.invoke.MethodType;
import java.net.InetSocketAddress;
import java.net.SocketAddress;
import java.util.Map;
import java.util.Optional;
import javax.validation.constraints.NotNull;
import org.apache.thrift.PayloadExceptionMetadata;
import org.apache.thrift.PayloadMetadata;
import org.apache.thrift.RequestSetupMetadata;
import org.apache.thrift.ResponseRpcMetadata;
import org.apache.thrift.StreamPayloadMetadata;
import org.apache.thrift.TApplicationException;
import org.apache.thrift.TException;
import org.apache.thrift.protocol.TField;
import org.apache.thrift.protocol.TProtocol;
import org.apache.thrift.protocol.TType;
import org.apache.thrift.transport.TTransportException;
import reactor.core.Exceptions;

public final class RpcClientUtils {

  private RpcClientUtils() {}

  /**
   * Returns Channel Class from evenloopGroup and socket address. Throws
   * UnsupportedOperationException if the combination is invalid.
   *
   * @param group
   * @param socketAddress
   * @return Channel class
   */
  public static Class<? extends Channel> getChannelClass(
      EventLoopGroup group, SocketAddress socketAddress) {
    if (socketAddress instanceof InetSocketAddress) {
      if (group instanceof EpollEventLoopGroup) {
        return EpollSocketChannel.class;
      }
      if (group instanceof KQueueEventLoopGroup) {
        return KQueueSocketChannel.class;
      }
      return NioSocketChannel.class;
    }
    if (socketAddress instanceof DomainSocketAddress) {
      if (group instanceof EpollEventLoopGroup) {
        return EpollDomainSocketChannel.class;
      } else if (getOS() == PlatformUtils.OS.LINUX) {
        Throwable unavailabilityCause = Epoll.unavailabilityCause();
        String errorMsg =
            unavailabilityCause == null
                ? "unavailabilityCause is null"
                : unavailabilityCause.getMessage();
        throw new UnsupportedOperationException(
            String.format(
                "Unsupported combination of EventLoopGroup-{%s} & SocketAddress-{%s}. Likely due to system support for Epoll unavailable due to {%s}",
                group.getClass(), socketAddress.getClass(), errorMsg),
            unavailabilityCause);
      }
      if (group instanceof KQueueEventLoopGroup) {
        return KQueueDomainSocketChannel.class;
      } else if (getOS() == PlatformUtils.OS.MAC) {
        Throwable unavailabilityCause = KQueue.unavailabilityCause();
        String errorMsg =
            unavailabilityCause == null
                ? "unavailabilityCause is null"
                : unavailabilityCause.getMessage();
        throw new UnsupportedOperationException(
            String.format(
                "Unsupported combination of EventLoopGroup-{%s} & SocketAddress-{%s}. Likely due to system support for Kqueue unavailable due to {%s}.",
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
   * Creates SslContext based on NettyClientConfig & Socket Address. Returns SSLContext or null if
   * configuration disables ssl.
   *
   * @param config
   * @param socketAddress
   * @return: SSL Context
   */
  public static SslContext getSslContext(
      @NotNull ThriftClientConfig config, SocketAddress socketAddress) {
    return getSslContext(config, socketAddress, getTransportTypeFromNettyType(config));
  }

  public static SslContext getSslContext(
      @NotNull ThriftClientConfig config,
      SocketAddress socketAddress,
      ThriftTransportType thriftTransportType) {
    return getSslContext(
        config.isSslDisabled(),
        thriftTransportType,
        config.getEnableJdkSsl(),
        config.getCertFile(),
        config.getKeyFile(),
        config.getCAFile(),
        config.getSessionCacheSize(),
        socketAddress);
  }

  private static ThriftTransportType getTransportTypeFromNettyType(ThriftClientConfig config) {
    return config.getTransport() == LegacyTransportType.HEADER
        ? ThriftTransportType.HEADER
        : ThriftTransportType.FRAMED;
  }

  /**
   * Creates SslContext based on NettyClientConfig & Socket Address. Returns SSLContext or null if
   * configuration disables ssl.
   *
   * @return: SSL Context
   */
  private static SslContext getSslContext(
      boolean sslDisabled,
      ThriftTransportType type,
      boolean enableJdkSsl,
      String certFile,
      String keyFile,
      String caFile,
      long sessionCacheSize,
      SocketAddress socketAddress) {
    // SSL is not needed if it is disabled in configs or it is a UDS call
    if (sslDisabled || socketAddress instanceof DomainSocketAddress) {
      return null;
    }
    try {
      final SslProvider sslProvider;
      final ApplicationProtocolConfig applicationProtocolConfig;
      if (enableJdkSsl) {
        applicationProtocolConfig =
            new ApplicationProtocolConfig(
                ApplicationProtocolConfig.Protocol.ALPN,
                ApplicationProtocolConfig.SelectorFailureBehavior.FATAL_ALERT,
                ApplicationProtocolConfig.SelectedListenerFailureBehavior.FATAL_ALERT,
                type.protocol());

        sslProvider = SslProvider.JDK;
      } else {
        applicationProtocolConfig =
            new ApplicationProtocolConfig(
                ApplicationProtocolConfig.Protocol.ALPN,
                ApplicationProtocolConfig.SelectorFailureBehavior.CHOOSE_MY_LAST_PROTOCOL,
                ApplicationProtocolConfig.SelectedListenerFailureBehavior.CHOOSE_MY_LAST_PROTOCOL,
                type.protocol());

        sslProvider = SslProvider.OPENSSL;
      }

      return SslContextBuilder.forClient()
          .keyManager(new FileInputStream(certFile), new FileInputStream(keyFile))
          .sslProvider(sslProvider)
          .trustManager(new FileInputStream(caFile))
          .sessionCacheSize(sessionCacheSize)
          .applicationProtocolConfig(applicationProtocolConfig)
          .build();
    } catch (Exception e) {
      throw Exceptions.propagate(e);
    }
  }

  /**
   * Decodes RSocket Stream Payload to ClientResponsePayload using ClientRequestPayload
   *
   * @param requestPayload: ClientRequestPayload
   * @param payload: Response payload
   * @param streamPayloadMetadata: Stream metadata
   * @return: ClientResponsePayload with decoded data
   */
  public static <T, K> ClientResponsePayload<K> decodeStreamPayload(
      final ClientRequestPayload<T> requestPayload,
      final Payload payload,
      final StreamPayloadMetadata streamPayloadMetadata,
      final ResponseRpcMetadata responseRpcMetadata,
      final boolean isFirstResponse) {

    ClientResponsePayload<K> response;
    ByteBuf data = payload.sliceData();

    if (!data.isReadable()) {
      return ClientResponsePayload.createEmptyStreamingResult(
          responseRpcMetadata, streamPayloadMetadata);
    }

    TProtocolType tProtocolType =
        TProtocolType.fromProtocolId(requestPayload.getRequestRpcMetadata().getProtocol());
    final ByteBufTProtocol protocol = tProtocolType.apply(data);
    protocol.readStructBegin();
    TField field = protocol.readFieldBegin();

    if (field.id == 0) {
      K streamResponse;
      if (isFirstResponse) {
        streamResponse =
            (K)
                StreamResponse.fromFirstResponse(
                    field.type != 0
                        ? requestPayload.getFirstResponseReader().read(protocol)
                        : new Object());
      } else {
        streamResponse =
            (K) StreamResponse.fromData(requestPayload.getResponseReader().read(protocol));
      }
      response =
          ClientResponsePayload.createResult(
              streamResponse, responseRpcMetadata, streamPayloadMetadata, true);
    } else {
      @SuppressWarnings("rawtypes")
      Reader exceptionReader =
          isFirstResponse
              ? requestPayload.getExceptionReaders().get(field.id)
              : requestPayload.getStreamExceptionReaders().get(field.id);
      Exception exception;
      // Parse undefined exception as TApplicationException
      if (exceptionReader == null) {
        exception =
            new TTransportException(
                field.type == TType.STRING ? protocol.readString() : payload.getDataUtf8());
      } else {
        exception = (Exception) exceptionReader.read(protocol);
      }
      response =
          ClientResponsePayload.createException(
              exception, responseRpcMetadata, streamPayloadMetadata, true);
    }

    protocol.readFieldEnd();
    protocol.readStructEnd();
    return response;
  }

  /**
   * Decodes RSocket Single Payload to ClientResponsePayload
   *
   * @param reader: Reader for streamed event
   * @param exceptionReaders: Exception object reader map
   * @param protocol: TProtocol for the object to be decoded
   * @param responseRpcMetadata: Stream metadata
   * @return: ClientResponsePayload with decoded data
   */
  @SuppressWarnings("rawtypes")
  public static <K> ClientResponsePayload<K> decodeRSocketPayload(
      final Reader<K> reader,
      final Map<Short, Reader> exceptionReaders,
      final TProtocol protocol,
      final ResponseRpcMetadata responseRpcMetadata) {

    ClientResponsePayload<K> response;
    protocol.readStructBegin();
    TField field = protocol.readFieldBegin();
    if (field.id == 0) {
      K read = reader.read(protocol);
      response = ClientResponsePayload.createResult(read, responseRpcMetadata, null, false);
    } else {
      Reader exceptionReader = exceptionReaders.get(field.id);
      Exception exception;
      // Parse undefined exception as TApplicationException
      if (exceptionReader == null) {
        exception =
            new TApplicationException(
                field.type == TType.STRING ? protocol.readString() : "Undefined exception");
      } else {
        exception = (Exception) exceptionReader.read(protocol);
      }
      response = ClientResponsePayload.createException(exception, responseRpcMetadata, null, false);
    }

    protocol.readFieldEnd();
    protocol.readStructEnd();
    return response;
  }

  /**
   * Checks if there is undeclared exception from response metadata. If yes, return the exception,
   * or else, return Optional.empty().
   *
   * @param rpcMetadata: ResponseRpcMetadata
   * @return Undeclared Exception if there is one.
   */
  public static Optional<? extends TException> getUndeclaredException(
      ResponseRpcMetadata rpcMetadata) {
    return Optional.ofNullable(rpcMetadata.getPayloadMetadata())
        .filter(PayloadMetadata::isSetExceptionMetadata)
        .map(PayloadMetadata::getExceptionMetadata)
        .map(
            exceptionMetadata -> {
              PayloadExceptionMetadata payloadExceptionMetadata = exceptionMetadata.getMetadata();
              if (payloadExceptionMetadata != null) {
                if (payloadExceptionMetadata.isSetDEPRECATEDProxyException()) {
                  return new TTransportException(exceptionMetadata.getWhatUtf8());
                } else if (!payloadExceptionMetadata.isSetDeclaredException()) {
                  // A write and read to protocol is performed to match THeader exception trace
                  // TODO: (vishwagayasen) Revert this to directly using ex when RSocket is released
                  ByteBuf byteBuf = null;
                  try {
                    byteBuf = PooledByteBufAllocator.DEFAULT.buffer();
                    ByteBufTProtocol protocol = TProtocolType.TBinary.apply(byteBuf);
                    new TApplicationException(exceptionMetadata.getWhatUtf8()).write(protocol);
                    return TApplicationException.read(protocol);
                  } finally {
                    if (byteBuf != null) {
                      byteBuf.release();
                    }
                  }
                }
              }
              return null;
            });
  }

  /**
   * Returns thrift serialized exception string. If the exception is not generated by thrift, the
   * exceptions are thrown to user as internal error in TApplication Exception
   *
   * @param throwable throwable returned by rSocket
   * @param methodName Thrift method name which raised this exception
   * @return Thrift serialized exception
   */
  public static String getExceptionString(Throwable throwable, String methodName) {
    if (throwable == null || throwable.getMessage() == null) {
      TApplicationException ex =
          new TApplicationException(
              "Internal error processing " + methodName + ": " + throwable.getClass().getName());
      throw Exceptions.propagate(ex);
    } else {
      return throwable.getMessage();
    }
  }

  /**
   * Create default configured RSocket Connector for RSocket factories
   *
   * @return Created RSocketConnector
   */
  public static RSocketConnector createRSocketConnector() {
    ByteBuf setupMetadata =
        RpcResources.getByteBufAllocator().buffer().writeInt((int) K_ROCKET_PROTOCOL_KEY);

    ByteBufTProtocol protocol = TProtocolType.TCompact.apply(setupMetadata);

    RequestSetupMetadata requestSetupMetadata =
        new RequestSetupMetadata.Builder().setMaxVersion(8).setMinVersion(8).build();

    requestSetupMetadata.write0(protocol);

    final Payload setupPayload = ByteBufPayload.create(Unpooled.EMPTY_BUFFER, setupMetadata);

    return RSocketConnector.create()
        .setupPayload(setupPayload)
        .payloadDecoder(PayloadDecoder.ZERO_COPY);
  }

  @SuppressWarnings("unchecked")
  public static <T> ClientBuilder<T> getClientBuilder(Class<T> clientInterface) {
    MethodHandle clientBuilderMethodHandle = clientBuilderMethodHandle(clientInterface);
    try {
      return (ClientBuilder<T>) clientBuilderMethodHandle.invoke();
    } catch (Throwable t) {
      throw Exceptions.propagate(t);
    }
  }

  private static MethodHandle clientBuilderMethodHandle(Class<?> clientInterface) {
    MethodHandles.Lookup lookup = MethodHandles.lookup();
    try {
      return lookup.findStatic(
          clientInterface, "clientBuilder", MethodType.methodType(ClientBuilder.class));
    } catch (Throwable t) {
      throw Exceptions.propagate(t);
    }
  }
}
