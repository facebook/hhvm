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

import static com.facebook.thrift.util.RpcServerUtils.getThriftSslSession;
import static com.google.common.base.Strings.nullToEmpty;
import static java.util.Objects.requireNonNull;
import static java.util.regex.Pattern.CASE_INSENSITIVE;

import com.facebook.nifty.core.NiftyConnectionContext;
import com.facebook.nifty.core.RequestContext;
import com.facebook.nifty.core.RequestContexts;
import com.facebook.thrift.legacy.codec.FrameInfo;
import com.facebook.thrift.legacy.codec.LegacyTransportType;
import com.facebook.thrift.legacy.codec.ThriftFrame;
import com.facebook.thrift.legacy.exceptions.FrameTooLargeException;
import com.facebook.thrift.payload.Reader;
import com.facebook.thrift.payload.ServerRequestPayload;
import com.facebook.thrift.payload.ServerResponsePayload;
import com.facebook.thrift.protocol.TProtocolType;
import com.facebook.thrift.server.RpcServerHandler;
import com.facebook.thrift.util.MonoTimeoutTransformer;
import com.facebook.thrift.util.NettyNiftyRequestContext;
import com.facebook.thrift.util.NettyUtil;
import io.airlift.units.Duration;
import io.netty.buffer.ByteBuf;
import io.netty.channel.ChannelDuplexHandler;
import io.netty.channel.ChannelHandlerContext;
import io.netty.handler.ssl.SslHandler;
import io.netty.util.ReferenceCountUtil;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Map;
import java.util.concurrent.RejectedExecutionException;
import java.util.concurrent.TimeUnit;
import java.util.function.Function;
import java.util.regex.Pattern;
import org.apache.thrift.RequestRpcMetadata;
import org.apache.thrift.RpcKind;
import org.apache.thrift.TApplicationException;
import org.apache.thrift.protocol.TMessage;
import org.apache.thrift.protocol.TMessageType;
import org.apache.thrift.protocol.TProtocol;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import reactor.core.Exceptions;
import reactor.core.publisher.Mono;

public class ThriftServerHandler extends ChannelDuplexHandler {
  private static final Logger log = LoggerFactory.getLogger(ThriftServerHandler.class);

  private static final Pattern CONNECTION_CLOSED_MESSAGE =
      Pattern.compile(
          "^.*(?:connection.*(?:reset|closed|abort|broken)|broken.*pipe).*$", CASE_INSENSITIVE);

  private final RpcServerHandler rpcServerHandler;
  private final Duration requestTimeout;

  ThriftServerHandler(RpcServerHandler rpcServerHandler, Duration requestTimeout) {
    this.rpcServerHandler = requireNonNull(rpcServerHandler, "rpcServerHandler is null");
    this.requestTimeout = requireNonNull(requestTimeout, "requestTimeout is null");
  }

  @Override
  public void channelRead(ChannelHandlerContext context, Object message) {
    if (message instanceof ThriftFrame) {
      messageReceived(context, (ThriftFrame) message);
      return;
    }
    context.fireChannelRead(message);
  }

  @Override
  public void exceptionCaught(ChannelHandlerContext context, Throwable cause) {
    // if possible, try to reply with an exception in case of a too large request
    if (cause instanceof FrameTooLargeException) {
      FrameTooLargeException e = (FrameTooLargeException) cause;
      // frame info may be missing in case of a large, but invalid request
      if (e.getFrameInfo().isPresent()) {
        FrameInfo frameInfo = e.getFrameInfo().get();
        try {
          context.writeAndFlush(
              encodeApplicationException(
                  context,
                  frameInfo.getMethodName(),
                  frameInfo.getTransport(),
                  frameInfo.getProtocol(),
                  frameInfo.getSequenceId(),
                  frameInfo.isSupportOutOfOrderResponse(),
                  TApplicationException.PROTOCOL_ERROR,
                  e.getMessage(),
                  Collections.emptyMap()));
        } catch (Throwable t) {
          context.close();
          log.error("Failed to write frame info", t);
        }
        return;
      }
    }

    context.close();

    // Don't log connection closed exceptions
    if (!isConnectionClosed(cause)) {
      log.error("exception caught", cause);
    }
  }

  private void messageReceived(ChannelHandlerContext context, ThriftFrame frame) {
    NettyNiftyRequestContext nettyNiftyRequestContext =
        new NettyNiftyRequestContext(frame.getHeaders(), getConnectionContext(context));
    RequestContext currentContext = RequestContexts.getCurrentContext();
    RequestContexts.setCurrentContext(nettyNiftyRequestContext);

    ServerRequestPayload serverRequestPayload = decodeMessage(frame, nettyNiftyRequestContext);
    Mono<Void> response;
    RequestRpcMetadata metadata = serverRequestPayload.getRequestRpcMetadata();
    if (serverRequestPayload.getRequestRpcMetadata().getKind()
        == RpcKind.SINGLE_REQUEST_NO_RESPONSE) {
      response =
          handleRequestNoResponse(
              context,
              frame,
              serverRequestPayload,
              metadata,
              nettyNiftyRequestContext.getResponseHeaders());
    } else {
      response =
          handleRequestResponse(
              context,
              frame,
              serverRequestPayload,
              metadata,
              nettyNiftyRequestContext.getResponseHeaders());
    }

    response
        .onErrorResume(
            throwable ->
                handleException(
                    context,
                    throwable,
                    metadata,
                    frame,
                    nettyNiftyRequestContext.getResponseHeaders()))
        .onErrorResume(
            throwable -> {
              log.error("Exception processing request", throwable);
              context.disconnect();
              return Mono.empty();
            })
        .doFinally(
            __ -> {
              RequestContexts.setCurrentContext(currentContext);
              if (frame.refCnt() > 0) {
                frame.release();
              }
            })
        .subscribe();
  }

  private Mono<Void> handleRequestNoResponse(
      ChannelHandlerContext context,
      ThriftFrame frame,
      ServerRequestPayload serverRequestPayload,
      RequestRpcMetadata metadata,
      Map<String, String> responseHeaders) {
    return rpcServerHandler
        .singleRequestNoResponse(serverRequestPayload)
        .transform(
            new MonoTimeoutTransformer<>(
                context.channel().eventLoop(),
                requestTimeout.toMillis(),
                TimeUnit.MILLISECONDS,
                Mono.defer(
                    () -> {
                      String message = String.format("method %s timed out", metadata.getName());
                      log.warn(message);
                      ThriftFrame exceptionFrame =
                          encodeApplicationException(
                              context,
                              metadata.getName(),
                              frame.getTransport(),
                              frame.getProtocol(),
                              frame.getSequenceId(),
                              frame.isSupportOutOfOrderResponse(),
                              TApplicationException.TIMEOUT,
                              message,
                              responseHeaders);

                      return NettyUtil.toMono(context.writeAndFlush(exceptionFrame));
                    })));
  }

  private Mono<Void> handleApplicationResponse(
      ChannelHandlerContext context,
      ThriftFrame frame,
      ServerResponsePayload serverResponsePayload,
      RequestRpcMetadata metadata,
      Map<String, String> responseHeaders,
      int sequenceId) {
    if (!context.channel().isActive()) {
      return Mono.error(new IllegalStateException("unable to write to closed channel"));
    }

    return NettyUtil.toMono(
        context.writeAndFlush(
            encodeApplicationResponse(
                context, metadata, frame, serverResponsePayload, responseHeaders, sequenceId)));
  }

  private Mono<Void> handleRequestResponse(
      ChannelHandlerContext context,
      ThriftFrame frame,
      ServerRequestPayload serverRequestPayload,
      RequestRpcMetadata metadata,
      Map<String, String> responseHeaders) {
    return rpcServerHandler
        .singleRequestSingleResponse(serverRequestPayload)
        .flatMap(
            serverResponsePayload ->
                handleApplicationResponse(
                    context,
                    frame,
                    serverResponsePayload,
                    metadata,
                    responseHeaders,
                    serverRequestPayload.getMessageSeqId()))
        .transform(
            new MonoTimeoutTransformer<>(
                context.channel().eventLoop(),
                requestTimeout.toMillis(),
                TimeUnit.MILLISECONDS,
                Mono.defer(
                    () -> {
                      String message = String.format("method %s timed out", metadata.getName());
                      log.warn(message);

                      ThriftFrame exceptionFrame =
                          encodeApplicationException(
                              context,
                              metadata.getName(),
                              frame.getTransport(),
                              frame.getProtocol(),
                              frame.getSequenceId(),
                              frame.isSupportOutOfOrderResponse(),
                              TApplicationException.TIMEOUT,
                              message,
                              responseHeaders);

                      return NettyUtil.toMono(context.writeAndFlush(exceptionFrame));
                    })));
  }

  private Mono<Void> handleException(
      ChannelHandlerContext context,
      Throwable throwable,
      RequestRpcMetadata metadata,
      ThriftFrame frame,
      Map<String, String> responseHeaders) {
    log.error("ThriftServerHandler received uncaught exception", throwable);
    if (throwable instanceof RejectedExecutionException) {
      ThriftFrame exceptionFrame =
          encodeApplicationException(
              context,
              metadata.getName(),
              frame.getTransport(),
              frame.getProtocol(),
              frame.getSequenceId(),
              frame.isSupportOutOfOrderResponse(),
              TApplicationException.LOADSHEDDING,
              throwable.getMessage(),
              responseHeaders);

      if (!context.channel().isActive()) {
        return Mono.error(new IllegalStateException("unable to write to closed channel"));
      }

      return NettyUtil.toMono(context.writeAndFlush(exceptionFrame))
          .doOnError(
              throwable1 -> {
                log.error("Exception while processing RejectedExecutionException", throwable1);
                context.disconnect();
              });
    } else {
      context.disconnect();

      throw Exceptions.propagate(throwable);
    }
  }

  ServerRequestPayload decodeMessage(ThriftFrame frame, RequestContext requestContext) {
    try {
      TProtocolType protocol = frame.getProtocol();
      final TProtocol byteBufTProtocol = protocol.apply(frame.getMessage());

      TMessage message = byteBufTProtocol.readMessageBegin();

      RpcKind kind =
          message.seqid == -1
              ? RpcKind.SINGLE_REQUEST_NO_RESPONSE
              : RpcKind.SINGLE_REQUEST_SINGLE_RESPONSE;

      return ServerRequestPayload.create(
          createReaderFunction(byteBufTProtocol),
          new RequestRpcMetadata.Builder()
              .setKind(kind)
              .setProtocol(protocol.getProtocolId())
              .setName(message.name)
              .build(),
          requestContext,
          message.seqid);
    } catch (Throwable t) {
      ReferenceCountUtil.safeRelease(frame);
      throw Exceptions.propagate(t);
    }
  }

  @SuppressWarnings("rawtypes")
  private static Function<List<Reader>, List<Object>> createReaderFunction(TProtocol out) {
    return readers -> {
      out.readStructBegin();
      List<Object> requestArguments = Collections.emptyList();
      if (readers != null && !readers.isEmpty()) {
        requestArguments = new ArrayList<>();
        for (Reader r : readers) {
          out.readFieldBegin();
          requestArguments.add(r.read(out));
          out.readFieldEnd();
        }
      }

      out.readStructEnd();
      out.readMessageEnd();
      return requestArguments;
    };
  }

  private static ThriftFrame encodeApplicationResponse(
      ChannelHandlerContext context,
      RequestRpcMetadata metadata,
      ThriftFrame frame,
      ServerResponsePayload serverResponsePayload,
      Map<String, String> responseHeaders,
      int sequenceId) {
    ByteBuf buf = context.alloc().buffer();
    TProtocol out = frame.getProtocol().apply(buf);

    if (serverResponsePayload.isTApplicationException()) {
      out.writeMessageBegin(new TMessage(metadata.getName(), TMessageType.EXCEPTION, sequenceId));
    } else {
      out.writeMessageBegin(new TMessage(metadata.getName(), TMessageType.REPLY, sequenceId));
    }
    serverResponsePayload.getDataWriter().write(out);
    out.writeMessageEnd();

    Map<String, String> otherMetadata =
        serverResponsePayload.getResponseRpcMetadata().getOtherMetadata();

    if (otherMetadata != null && !otherMetadata.isEmpty()) {
      responseHeaders.putAll(otherMetadata);
    }

    return new ThriftFrame(
        frame.getSequenceId(),
        buf,
        responseHeaders,
        Collections.emptyMap(),
        frame.getTransport(),
        frame.getProtocol(),
        frame.isSupportOutOfOrderResponse());
  }

  private static ThriftFrame encodeApplicationException(
      ChannelHandlerContext context,
      String methodName,
      LegacyTransportType transport,
      TProtocolType protocol,
      int sequenceId,
      boolean supportOutOfOrderResponse,
      int errorCode,
      String errorMessage,
      Map<String, String> responseHeaders) {
    TApplicationException applicationException = new TApplicationException(errorCode, errorMessage);

    ByteBuf buf = context.alloc().buffer();
    TProtocol out = protocol.apply(buf);

    out.writeMessageBegin(new TMessage(methodName, TMessageType.EXCEPTION, sequenceId));
    applicationException.write(out);
    out.writeMessageEnd();

    return new ThriftFrame(
        sequenceId,
        buf,
        responseHeaders,
        Collections.emptyMap(),
        transport,
        protocol,
        supportOutOfOrderResponse);
  }

  /*
   * There is no good way of detecting connection closed exception
   *
   * This implementation is a simplified version of the implementation proposed
   * in Netty: io.netty.handler.ssl.SslHandler#exceptionCaught
   *
   * This implementation ony checks a message with the regex, and doesn't do any
   * more sophisticated matching, as the regex works in most of the cases.
   */
  private boolean isConnectionClosed(Throwable t) {
    if (t instanceof IOException) {
      return CONNECTION_CLOSED_MESSAGE.matcher(nullToEmpty(t.getMessage())).matches();
    }
    return false;
  }

  /**
   * Creates nifty connection context given handler channel handler context
   *
   * @param context Netty 4 ChannelHandlerContext
   */
  private NiftyConnectionContext getConnectionContext(ChannelHandlerContext context) {
    NiftyConnectionContext connectionContext = new NiftyConnectionContext();
    connectionContext.setRemoteAddress(context.channel().remoteAddress());
    connectionContext.setSslSession(getThriftSslSession(context.pipeline().get(SslHandler.class)));
    return connectionContext;
  }
}
