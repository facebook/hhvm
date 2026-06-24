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

import static com.facebook.thrift.rsocket.util.PayloadUtil.createPayload;

import com.facebook.nifty.core.ConnectionContext;
import com.facebook.nifty.core.NiftyConnectionContext;
import com.facebook.nifty.core.RequestContext;
import com.facebook.thrift.compression.CompressionManager;
import com.facebook.thrift.compression.ThriftCompressor;
import com.facebook.thrift.payload.Reader;
import com.facebook.thrift.payload.ServerRequestPayload;
import com.facebook.thrift.payload.ServerResponsePayload;
import com.facebook.thrift.payload.Writer;
import com.facebook.thrift.protocol.ByteBufTProtocol;
import com.facebook.thrift.protocol.ProtocolUtil;
import com.facebook.thrift.protocol.TProtocolType;
import com.facebook.thrift.server.RpcServerHandler;
import com.facebook.thrift.util.NettyNiftyRequestContext;
import io.netty.buffer.ByteBuf;
import io.netty.buffer.ByteBufAllocator;
import io.netty.util.ReferenceCountUtil;
import io.netty.util.ReferenceCounted;
import io.rsocket.Payload;
import io.rsocket.RSocket;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Map;
import java.util.function.Function;
import org.apache.thrift.CompressionAlgorithm;
import org.apache.thrift.RequestRpcMetadata;
import org.apache.thrift.RpcKind;
import org.apache.thrift.protocol.TProtocol;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import reactor.core.Exceptions;
import reactor.core.publisher.Flux;
import reactor.core.publisher.Mono;

public class ThriftServerRSocket implements RSocket {
  private static final Logger LOGGER = LoggerFactory.getLogger(ThriftServerRSocket.class);

  private final RpcServerHandler rpcServerHandler;
  private final ByteBufAllocator alloc;
  private final ConnectionContext connectionContext;

  public ThriftServerRSocket(RpcServerHandler rpcServerHandler, ByteBufAllocator alloc) {
    this(rpcServerHandler, alloc, null);
  }

  public ThriftServerRSocket(
      RpcServerHandler rpcServerHandler,
      ByteBufAllocator alloc,
      ConnectionContext connectionContext) {
    this.rpcServerHandler = rpcServerHandler;
    this.alloc = alloc;
    this.connectionContext =
        connectionContext != null ? connectionContext : new NiftyConnectionContext();
  }

  @Override
  public Mono<Payload> requestResponse(Payload payload) {
    ByteBuf data = null;
    ServerRequestPayload requestPayload = null;
    try {
      RequestRpcMetadata requestRpcMetadata = decodeRequestRpcMetadata(payload);

      RequestContext requestContext = createRequestContext(requestRpcMetadata.getOtherMetadata());

      data = maybeDecompressRequestData(requestRpcMetadata, payload);
      // The decoded request data is now owned independently of the RSocket Payload (retained slice
      // in the uncompressed case, freshly allocated in the compressed case). Release the Payload
      // immediately; the request buffer alone goes on the wire as the ServerRequestPayload's owned
      // buffer.
      ReferenceCountUtil.safeRelease(payload);
      payload = null;

      requestPayload = deserializeRequest(data, requestRpcMetadata, requestContext, data);
      data = null; // ownership transferred to ServerRequestPayload

      assert requestPayload.getRequestRpcMetadata().getKind()
          == RpcKind.SINGLE_REQUEST_SINGLE_RESPONSE;

      final ServerRequestPayload finalRequestPayload = requestPayload;
      return rpcServerHandler
          .singleRequestSingleResponse(requestPayload)
          .map(responsePayload -> handleResponse(alloc, finalRequestPayload, responsePayload))
          // The generated handler releases the request buffer right after it reads the args; this
          // is the idempotent backstop for paths where no method body runs (unsupported RpcKind,
          // unknown method, scheduler rejection, or cancellation before the read).
          .doFinally(__ -> finalRequestPayload.releaseRequestData());
    } catch (Throwable t) {
      releaseOnError(requestPayload, data, payload);
      return Mono.error(t);
    }
  }

  @Override
  public Flux<Payload> requestStream(Payload payload) {
    ByteBuf data = null;
    ServerRequestPayload requestPayload = null;
    try {
      RequestRpcMetadata requestRpcMetadata = decodeRequestRpcMetadata(payload);

      RequestContext requestContext = createRequestContext(requestRpcMetadata.getOtherMetadata());

      data = maybeDecompressRequestData(requestRpcMetadata, payload);
      ReferenceCountUtil.safeRelease(payload);
      payload = null;

      requestPayload = deserializeRequest(data, requestRpcMetadata, requestContext, data);
      data = null; // ownership transferred to ServerRequestPayload

      assert requestPayload.getRequestRpcMetadata().getKind()
          == RpcKind.SINGLE_REQUEST_STREAMING_RESPONSE;

      final ServerRequestPayload finalRequestPayload = requestPayload;
      return rpcServerHandler
          .singleRequestStreamingResponse(requestPayload)
          .map(responsePayload -> handleStreamResponse(alloc, finalRequestPayload, responsePayload))
          // The stream handler reads the args once when the stream is subscribed and releases the
          // request buffer then, so it is freed up front instead of being pinned for the whole
          // stream; this is the idempotent backstop for paths where no method body runs.
          .doFinally(__ -> finalRequestPayload.releaseRequestData());
    } catch (Throwable t) {
      releaseOnError(requestPayload, data, payload);
      return Flux.error(t);
    }
  }

  private static Payload handleStreamResponse(
      ByteBufAllocator alloc,
      ServerRequestPayload requestPayload,
      ServerResponsePayload responsePayload) {
    ByteBuf data = null;
    ByteBuf metadata = null;

    try {
      data = alloc.buffer();
      metadata = alloc.buffer();

      serializeStreamMetadata(responsePayload, metadata);
      serializeResponse(requestPayload, responsePayload, data);

      return createPayload(alloc, null, data, metadata);
    } catch (Throwable t) {
      if (data != null && data.refCnt() > 0) {
        data.release();
      }

      if (metadata != null && metadata.refCnt() > 0) {
        metadata.release();
      }
      throw Exceptions.propagate(t);
    }
  }

  private RequestContext createRequestContext(Map<String, String> requestHeaders) {
    return new NettyNiftyRequestContext(requestHeaders, connectionContext);
  }

  /**
   * Returns the request data, decompressing if the metadata specifies compression. The returned
   * buffer is always independently owned — the caller must release it. Always retains {@code
   * payload.sliceData()} first; in the uncompressed case that retained slice is returned, in the
   * compressed case the compressor takes ownership of it (releasing it in its own finally
   * regardless of success or failure) and returns a freshly allocated uncompressed buffer. Looking
   * up the compressor is the only pre-ownership-transfer step; if it throws (e.g. unknown
   * algorithm), the retained slice is released here.
   */
  private ByteBuf maybeDecompressRequestData(RequestRpcMetadata metadata, Payload payload) {
    ByteBuf data = payload.sliceData();
    data.retain();
    CompressionAlgorithm compression = metadata.getCompression();
    if (compression == null || compression == CompressionAlgorithm.NONE) {
      return data;
    }
    ThriftCompressor compressor;
    try {
      compressor = CompressionManager.getCompressor(compression);
    } catch (Throwable t) {
      ReferenceCountUtil.safeRelease(data);
      throw t;
    }
    return compressor.decompress(alloc, data); // ownership of `data` transferred to the compressor
  }

  @SuppressWarnings("rawtypes")
  private static ServerRequestPayload deserializeRequest(
      ByteBuf data,
      RequestRpcMetadata requestRpcMetadata,
      RequestContext requestContext,
      ReferenceCounted requestData) {
    ByteBufTProtocol out =
        TProtocolType.fromProtocolId(requestRpcMetadata.getProtocol()).apply(data);
    Function<List<Reader>, List<Object>> readerTransformer = createReaderFunction(out);
    // The payload owns the decoded request data buffer so the generated handler can release it (via
    // releaseRequestData()) right after reading the request args, instead of holding it until the
    // response completes.
    return ServerRequestPayload.create(
        readerTransformer, requestRpcMetadata, requestContext, requestData);
  }

  /**
   * Releases what is still owned by this method on a synchronous failure before the reactive
   * pipeline is returned. The success path nulls out {@code payload} (already released after
   * decode) and {@code data} (ownership transferred to {@code requestPayload}); this method
   * therefore safely no-ops on whichever has already been handed off.
   *
   * <p>If {@code requestPayload} exists, route the buffer release through its idempotent {@link
   * ServerRequestPayload#releaseRequestData()} so the catch and success paths free it exactly once;
   * otherwise the local {@code data} buffer is released directly. The original {@code payload} is
   * always {@link ReferenceCountUtil#safeRelease(Object) safeRelease}d in case the failure happened
   * before the post-decode release.
   */
  private static void releaseOnError(
      ServerRequestPayload requestPayload, ByteBuf data, Payload payload) {
    if (requestPayload != null) {
      requestPayload.releaseRequestData();
    } else {
      ReferenceCountUtil.safeRelease(data);
    }
    ReferenceCountUtil.safeRelease(payload);
  }

  private static RequestRpcMetadata decodeRequestRpcMetadata(Payload payload) {
    return ProtocolUtil.readCompact(RequestRpcMetadata::read0, payload.sliceMetadata());
  }

  private static Payload handleResponse(
      ByteBufAllocator alloc,
      ServerRequestPayload requestPayload,
      ServerResponsePayload responsePayload) {
    ByteBuf data = null;
    ByteBuf metadata = null;

    try {
      data = alloc.buffer();
      metadata = alloc.buffer();

      serializeResponse(requestPayload, responsePayload, data);
      serializeResponseMetadata(responsePayload, metadata);

      return createPayload(
          alloc, responsePayload.getResponseRpcMetadata().getCompression(), data, metadata);

    } catch (Throwable t) {
      if (data != null && data.refCnt() > 0) {
        data.release();
      }

      if (metadata != null && metadata.refCnt() > 0) {
        metadata.release();
      }
      throw Exceptions.propagate(t);
    }
  }

  private static void serializeResponseMetadata(
      ServerResponsePayload responsePayload, ByteBuf metadata) {
    ProtocolUtil.writeCompact(responsePayload.getResponseRpcMetadata()::write0, metadata);
  }

  private static void serializeStreamMetadata(
      ServerResponsePayload responsePayload, ByteBuf metadata) {
    if (responsePayload.getResponseRpcMetadata() != null) {
      ProtocolUtil.writeCompact(responsePayload.getResponseRpcMetadata()::write0, metadata);
    } else {
      ProtocolUtil.writeCompact(responsePayload.getStreamPayloadMetadata()::write0, metadata);
    }
  }

  private static void serializeResponse(
      ServerRequestPayload requestPayload, ServerResponsePayload responsePayload, ByteBuf data) {
    final ByteBufTProtocol in = getTProtocol(requestPayload, data);
    final Writer writer = responsePayload.getDataWriter();
    writer.write(in);
  }

  private static ByteBufTProtocol getTProtocol(ServerRequestPayload requestPayload, ByteBuf data) {
    return TProtocolType.fromProtocolId(requestPayload.getRequestRpcMetadata().getProtocol())
        .apply(data);
  }

  @Override
  public Mono<Void> fireAndForget(Payload payload) {
    ByteBuf data = null;
    ServerRequestPayload requestPayload = null;
    try {
      RequestRpcMetadata requestRpcMetadata = decodeRequestRpcMetadata(payload);

      RequestContext requestContext = createRequestContext(requestRpcMetadata.getOtherMetadata());

      data = maybeDecompressRequestData(requestRpcMetadata, payload);
      ReferenceCountUtil.safeRelease(payload);
      payload = null;

      requestPayload = deserializeRequest(data, requestRpcMetadata, requestContext, data);
      data = null; // ownership transferred to ServerRequestPayload

      assert requestPayload.getRequestRpcMetadata().getKind() == RpcKind.SINGLE_REQUEST_NO_RESPONSE;

      final ServerRequestPayload finalRequestPayload = requestPayload;
      return rpcServerHandler
          .singleRequestNoResponse(requestPayload)
          // The generated handler releases the request buffer right after it reads the args; this
          // is the idempotent backstop for paths where no method body runs.
          .doFinally(__ -> finalRequestPayload.releaseRequestData());
    } catch (Throwable t) {
      releaseOnError(requestPayload, data, payload);
      return Mono.error(t);
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
}
