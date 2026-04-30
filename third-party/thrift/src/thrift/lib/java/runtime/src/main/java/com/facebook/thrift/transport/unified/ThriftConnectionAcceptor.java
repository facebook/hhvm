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

import static com.facebook.thrift.util.RpcServerUtils.getThriftSslSession;
import static java.util.Objects.requireNonNull;
import static org.apache.thrift.RpcKind.SINGLE_REQUEST_NO_RESPONSE;
import static org.apache.thrift.RpcKind.SINGLE_REQUEST_SINGLE_RESPONSE;

import com.facebook.nifty.core.NiftyConnectionContext;
import com.facebook.nifty.core.RequestContext;
import com.facebook.thrift.legacy.codec.ThriftFrame;
import com.facebook.thrift.payload.Reader;
import com.facebook.thrift.payload.ServerRequestPayload;
import com.facebook.thrift.payload.ServerResponsePayload;
import com.facebook.thrift.protocol.TProtocolType;
import com.facebook.thrift.server.RpcServerHandler;
import com.facebook.thrift.util.MonoTimeoutTransformer;
import com.facebook.thrift.util.NettyNiftyRequestContext;
import com.facebook.thrift.util.RpcServerUtils;
import com.facebook.thrift.util.resources.RpcResources;
import io.airlift.units.Duration;
import io.netty.buffer.ByteBuf;
import io.netty.handler.ssl.SslHandler;
import io.netty.util.ReferenceCountUtil;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Map;
import java.util.concurrent.RejectedExecutionException;
import java.util.concurrent.TimeUnit;
import java.util.function.Function;
import org.apache.thrift.RequestRpcMetadata;
import org.apache.thrift.RpcKind;
import org.apache.thrift.TApplicationException;
import org.apache.thrift.protocol.TMessage;
import org.apache.thrift.protocol.TMessageType;
import org.apache.thrift.protocol.TProtocol;
import org.reactivestreams.Publisher;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import reactor.core.publisher.Mono;
import reactor.netty.Connection;
import reactor.netty.NettyInbound;
import reactor.netty.NettyOutbound;
import reactor.netty.channel.AbortedException;

/**
 * Thrift Header connection handler for Reactor Netty.
 *
 * <p>This class processes inbound Thrift header requests and returns responses using Reactor
 * Netty's NettyInbound and NettyOutbound APIs. It handles:
 *
 * <ul>
 *   <li>ByteBuf → ThriftFrame decoding
 *   <li>Request processing via RpcServerHandler
 *   <li>ThriftFrame → ByteBuf encoding
 *   <li>Error handling (RejectedExecution, Timeout)
 *   <li>Memory management of ByteBufs and ThriftFrames
 * </ul>
 */
public class ThriftConnectionAcceptor implements Function<Connection, Publisher<Void>> {
  private static final Logger log = LoggerFactory.getLogger(ThriftConnectionAcceptor.class);

  private final RpcServerHandler rpcServerHandler;
  private final Duration requestTimeout;

  public ThriftConnectionAcceptor(RpcServerHandler rpcServerHandler, Duration requestTimeout) {
    this.rpcServerHandler = requireNonNull(rpcServerHandler, "rpcServerHandler is null");
    this.requestTimeout = requireNonNull(requestTimeout, "requestTimeout is null");
  }

  @Override
  public Mono<Void> apply(Connection conn) {
    NettyInbound in = conn.inbound();
    NettyOutbound out = conn.outbound();

    // Create connection context once — remote address and SSL session don't change
    NiftyConnectionContext connectionContext = getNiftyConnectionContext(conn);

    return in.receive()
        // Retain ByteBuf because the message body will be held and freed by the thrift frame in
        // processRequest
        .retain()
        // Fire-and-forget: decouple network read rate from RPC processing time.
        // processRequest handles all errors internally (try/catch + onErrorResume),
        // so the no-arg .subscribe() will never see an onError signal.
        // This matches the legacy ThriftServerHandler behavior where channelRead() dispatches
        // each frame independently, with overload protection via RejectedExecutionException.
        .doOnNext(byteBuf -> processRequest(conn, out, connectionContext, byteBuf).subscribe())
        .then()
        .doOnError(
            throwable -> {
              if (!AbortedException.isConnectionReset(throwable)
                  && !RpcServerUtils.isSslCloseNotify(throwable)) {
                log.error(
                    "Unrecoverable error handling request, closing connection. Remote: {}",
                    conn.channel().remoteAddress(),
                    throwable);
              }
            });
  }

  private static NiftyConnectionContext getNiftyConnectionContext(Connection conn) {
    NiftyConnectionContext connectionContext = new NiftyConnectionContext();
    connectionContext.setRemoteAddress(conn.channel().remoteAddress());
    connectionContext.setSslSession(
        getThriftSslSession(conn.channel().pipeline().get(SslHandler.class)));
    return connectionContext;
  }

  /**
   * Process a single request ByteBuf: decode → process → encode → send response.
   *
   * @param conn the connection
   * @param out the outbound channel
   * @param connectionContext the per-connection context (reused across requests)
   * @param byteBuf the incoming request ByteBuf (already retained by caller)
   * @return Mono that completes when response is sent
   */
  private Mono<Void> processRequest(
      Connection conn,
      NettyOutbound out,
      NiftyConnectionContext connectionContext,
      ByteBuf byteBuf) {
    ThriftFrame requestFrame = null;
    try {
      // Decode ByteBuf to ThriftFrame
      requestFrame = ReactiveHeaderCodec.decodeFrame(out.alloc(), byteBuf);

      // Parse the request
      NettyNiftyRequestContext requestContext =
          new NettyNiftyRequestContext(requestFrame.getHeaders(), connectionContext);
      ServerRequestPayload serverRequestPayload = decodeMessage(requestFrame, requestContext);
      RequestRpcMetadata metadata = serverRequestPayload.getRequestRpcMetadata();

      // Dispatch based on request type
      final ThriftFrame finalRequestFrame = requestFrame;
      Mono<Void> result;
      if (SINGLE_REQUEST_NO_RESPONSE == metadata.getKind()) {
        result = handleOnewayRequest(serverRequestPayload, metadata);
      } else if (SINGLE_REQUEST_SINGLE_RESPONSE == metadata.getKind()) {
        result =
            handleRequestResponse(
                conn, out, finalRequestFrame, serverRequestPayload, metadata, requestContext);
      } else {
        // Unsupported RpcKind - send protocol error
        String errorMessage =
            String.format(
                "Unsupported RpcKind %s for method %s. Only SINGLE_REQUEST_NO_RESPONSE and"
                    + " SINGLE_REQUEST_SINGLE_RESPONSE are supported for header transport.",
                metadata.getKind(), metadata.getName());
        log.error(errorMessage);

        result =
            sendProtocolError(
                conn,
                out,
                finalRequestFrame,
                metadata,
                errorMessage,
                requestContext.getResponseHeaders());
      }

      return result
          .doFinally(__ -> ReferenceCountUtil.safeRelease(finalRequestFrame))
          .onErrorResume(
              t -> {
                log.error("Exception processing request", t);
                return Mono.empty();
              });
    } catch (Throwable t) {
      if (requestFrame != null) {
        ReferenceCountUtil.safeRelease(requestFrame);
      } else {
        ReferenceCountUtil.safeRelease(byteBuf);
      }
      log.error("Exception decoding request", t);
      return Mono.empty();
    }
  }

  /**
   * Handle oneway (no response) request.
   *
   * <p>Note: RequestContext ThreadLocal propagation is handled by the generated RpcServerHandler
   * code, which reads the context from the payload and sets it on the execution thread. The
   * transport layer does not need to manage the ThreadLocal.
   *
   * @param serverRequestPayload the parsed request
   * @param metadata the request metadata
   * @return Mono that completes when processing is done
   */
  private Mono<Void> handleOnewayRequest(
      ServerRequestPayload serverRequestPayload, RequestRpcMetadata metadata) {
    return rpcServerHandler
        .singleRequestNoResponse(serverRequestPayload)
        .transform(createTimeoutTransformer(Mono.empty()))
        .onErrorResume(
            throwable -> {
              log.error(
                  "Error processing oneway request for method {}", metadata.getName(), throwable);
              return Mono.empty();
            });
  }

  /**
   * Handle request/response request.
   *
   * <p>Note: RequestContext ThreadLocal propagation is handled by the generated RpcServerHandler
   * code, which reads the context from the payload and sets it on the execution thread. The
   * transport layer does not need to manage the ThreadLocal.
   *
   * @param conn the connection
   * @param out the outbound channel
   * @param requestFrame the request frame
   * @param serverRequestPayload the parsed request
   * @param metadata the request metadata
   * @param requestContext the request context
   * @return Mono that completes when response is sent
   */
  private Mono<Void> handleRequestResponse(
      Connection conn,
      NettyOutbound out,
      ThriftFrame requestFrame,
      ServerRequestPayload serverRequestPayload,
      RequestRpcMetadata metadata,
      NettyNiftyRequestContext requestContext) {
    return rpcServerHandler
        .singleRequestSingleResponse(serverRequestPayload)
        .flatMap(
            serverResponsePayload ->
                encodeAndSendResponse(
                    conn,
                    out,
                    requestFrame,
                    metadata,
                    serverResponsePayload,
                    requestContext.getResponseHeaders(),
                    serverRequestPayload.getMessageSeqId()))
        .transform(
            createTimeoutTransformer(
                Mono.defer(
                    () ->
                        sendTimeoutException(
                            conn,
                            out,
                            requestFrame,
                            metadata,
                            requestContext.getResponseHeaders()))))
        .onErrorResume(
            throwable ->
                handleException(
                    conn,
                    out,
                    throwable,
                    metadata,
                    requestFrame,
                    requestContext.getResponseHeaders()));
  }

  /**
   * Encode response payload and send it.
   *
   * @param conn the connection
   * @param out the outbound channel
   * @param requestFrame the request frame (for metadata)
   * @param metadata the request metadata
   * @param serverResponsePayload the response payload
   * @param responseHeaders the response headers
   * @param sequenceId the sequence ID
   * @return Mono that completes when response is sent
   */
  private Mono<Void> encodeAndSendResponse(
      Connection conn,
      NettyOutbound out,
      ThriftFrame requestFrame,
      RequestRpcMetadata metadata,
      ServerResponsePayload serverResponsePayload,
      Map<String, String> responseHeaders,
      int sequenceId) {
    try {
      ThriftFrame responseFrame =
          encodeApplicationResponse(
              conn, requestFrame, metadata, serverResponsePayload, responseHeaders, sequenceId);

      ByteBuf responseBuf = ReactiveHeaderCodec.encodeFrame(out.alloc(), responseFrame, false);
      return out.sendObject(responseBuf).then();
    } catch (Throwable t) {
      log.error("Failed to encode response", t);
      return Mono.error(t);
    }
  }

  /**
   * Send timeout exception to client.
   *
   * @param conn the connection
   * @param out the outbound channel
   * @param requestFrame the request frame
   * @param metadata the request metadata
   * @param responseHeaders the response headers
   * @return Mono that completes when exception is sent
   */
  private Mono<Void> sendTimeoutException(
      Connection conn,
      NettyOutbound out,
      ThriftFrame requestFrame,
      RequestRpcMetadata metadata,
      Map<String, String> responseHeaders) {
    String message =
        String.format("method %s timed out after %s", metadata.getName(), requestTimeout);
    log.warn(message);

    try {
      ThriftFrame exceptionFrame =
          ReactiveHeaderCodec.encodeApplicationException(
              conn,
              metadata.getName(),
              requestFrame.getTransport(),
              requestFrame.getProtocol(),
              requestFrame.getSequenceId(),
              requestFrame.isSupportOutOfOrderResponse(),
              TApplicationException.TIMEOUT,
              message,
              responseHeaders);

      ByteBuf exceptionBuf = ReactiveHeaderCodec.encodeFrame(out.alloc(), exceptionFrame, false);
      return out.sendObject(exceptionBuf).then();
    } catch (Throwable t) {
      log.error("Failed to encode timeout exception", t);
      return Mono.error(t);
    }
  }

  /**
   * Send protocol error exception to client.
   *
   * @param conn the connection
   * @param out the outbound channel
   * @param requestFrame the request frame
   * @param metadata the request metadata
   * @param errorMessage the error message
   * @param responseHeaders the response headers
   * @return Mono that completes when exception is sent
   */
  private Mono<Void> sendProtocolError(
      Connection conn,
      NettyOutbound out,
      ThriftFrame requestFrame,
      RequestRpcMetadata metadata,
      String errorMessage,
      Map<String, String> responseHeaders) {
    try {
      ThriftFrame exceptionFrame =
          ReactiveHeaderCodec.encodeApplicationException(
              conn,
              metadata.getName(),
              requestFrame.getTransport(),
              requestFrame.getProtocol(),
              requestFrame.getSequenceId(),
              requestFrame.isSupportOutOfOrderResponse(),
              TApplicationException.PROTOCOL_ERROR,
              errorMessage,
              responseHeaders);

      ByteBuf exceptionBuf = ReactiveHeaderCodec.encodeFrame(out.alloc(), exceptionFrame, false);
      return out.sendObject(exceptionBuf).then();
    } catch (Throwable t) {
      log.error("Failed to encode protocol error exception", t);
      return Mono.error(t);
    }
  }

  /**
   * Handle exception during request processing.
   *
   * @param conn the connection
   * @param out the outbound channel
   * @param throwable the exception
   * @param metadata the request metadata
   * @param requestFrame the request frame
   * @param responseHeaders the response headers
   * @return Mono that handles the exception
   */
  private Mono<Void> handleException(
      Connection conn,
      NettyOutbound out,
      Throwable throwable,
      RequestRpcMetadata metadata,
      ThriftFrame requestFrame,
      Map<String, String> responseHeaders) {
    log.error("ThriftConnectionAcceptorV2 received uncaught exception", throwable);

    if (throwable instanceof RejectedExecutionException) {
      try {
        ThriftFrame exceptionFrame =
            ReactiveHeaderCodec.encodeApplicationException(
                conn,
                metadata.getName(),
                requestFrame.getTransport(),
                requestFrame.getProtocol(),
                requestFrame.getSequenceId(),
                requestFrame.isSupportOutOfOrderResponse(),
                TApplicationException.LOADSHEDDING,
                throwable.getMessage(),
                responseHeaders);

        ByteBuf exceptionBuf = ReactiveHeaderCodec.encodeFrame(out.alloc(), exceptionFrame, false);
        return out.sendObject(exceptionBuf).then();
      } catch (Throwable t) {
        log.error("Failed to encode loadshedding exception", t);
        return Mono.error(t);
      }
    } else {
      // Close connection for non-loadshedding errors
      conn.dispose();
      return Mono.empty();
    }
  }

  /**
   * Decode ThriftFrame into ServerRequestPayload.
   *
   * @param frame the frame to decode
   * @param requestContext the request context
   * @return the decoded ServerRequestPayload
   */
  private ServerRequestPayload decodeMessage(ThriftFrame frame, RequestContext requestContext) {
    try {
      TProtocolType protocol = frame.getProtocol();
      TProtocol byteBufTProtocol = protocol.apply(frame.getMessage());

      TMessage message = byteBufTProtocol.readMessageBegin();

      RpcKind kind =
          message.seqid == -1 ? SINGLE_REQUEST_NO_RESPONSE : SINGLE_REQUEST_SINGLE_RESPONSE;

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
      throw new RuntimeException("Failed to decode message", t);
    }
  }

  /**
   * Create reader function for parsing request arguments.
   *
   * @param protocol the protocol
   * @return function that reads request arguments
   */
  @SuppressWarnings("rawtypes")
  private static Function<List<Reader>, List<Object>> createReaderFunction(TProtocol protocol) {
    return readers -> {
      protocol.readStructBegin();
      List<Object> requestArguments = Collections.emptyList();
      if (readers != null && !readers.isEmpty()) {
        requestArguments = new ArrayList<>();
        for (Reader r : readers) {
          protocol.readFieldBegin();
          requestArguments.add(r.read(protocol));
          protocol.readFieldEnd();
        }
      }

      protocol.readStructEnd();
      protocol.readMessageEnd();
      return requestArguments;
    };
  }

  /**
   * Encode application response into ThriftFrame.
   *
   * @param conn the connection
   * @param requestFrame the request frame (for metadata)
   * @param metadata the request metadata
   * @param serverResponsePayload the response payload
   * @param responseHeaders the response headers
   * @param sequenceId the sequence ID
   * @return the encoded ThriftFrame
   */
  private static ThriftFrame encodeApplicationResponse(
      Connection conn,
      ThriftFrame requestFrame,
      RequestRpcMetadata metadata,
      ServerResponsePayload serverResponsePayload,
      Map<String, String> responseHeaders,
      int sequenceId) {
    ByteBuf buf = conn.channel().alloc().buffer();
    try {
      TProtocol out = requestFrame.getProtocol().apply(buf);

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

      ByteBuf finalBuf = buf;
      buf = null; // Prevent release in finally
      return new ThriftFrame(
          requestFrame.getSequenceId(),
          finalBuf,
          responseHeaders,
          Collections.emptyMap(),
          requestFrame.getTransport(),
          requestFrame.getProtocol(),
          requestFrame.isSupportOutOfOrderResponse());
    } catch (Throwable t) {
      throw new RuntimeException("Failed to encode application response", t);
    } finally {
      if (buf != null) {
        ReferenceCountUtil.safeRelease(buf);
      }
    }
  }

  /**
   * Create timeout transformer for request processing.
   *
   * @param fallback the fallback Mono to use on timeout
   * @return the timeout transformer
   */
  private <T> MonoTimeoutTransformer<T> createTimeoutTransformer(Mono<T> fallback) {
    return new MonoTimeoutTransformer<>(
        RpcResources.getOffLoopScheduler(),
        requestTimeout.toMillis(),
        TimeUnit.MILLISECONDS,
        fallback);
  }

}
