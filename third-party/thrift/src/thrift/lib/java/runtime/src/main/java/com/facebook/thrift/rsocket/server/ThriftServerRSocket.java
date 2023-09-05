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
import io.rsocket.Payload;
import io.rsocket.RSocket;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Map;
import java.util.function.Function;
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

  public ThriftServerRSocket(RpcServerHandler rpcServerHandler, ByteBufAllocator alloc) {
    this.rpcServerHandler = rpcServerHandler;
    this.alloc = alloc;
  }

  @Override
  public Mono<Payload> requestResponse(Payload payload) {
    try {
      RequestRpcMetadata requestRpcMetadata = decodeRequestRpcMetadata(payload);

      RequestContext requestContext = createRequestContext(requestRpcMetadata.getOtherMetadata());

      ServerRequestPayload requestPayload =
          deserializeRequest(payload, requestRpcMetadata, requestContext);

      assert requestPayload.getRequestRpcMetadata().getKind()
          == RpcKind.SINGLE_REQUEST_SINGLE_RESPONSE;

      return rpcServerHandler
          .singleRequestSingleResponse(requestPayload)
          .map(responsePayload -> handleResponse(alloc, requestPayload, responsePayload))
          .doFinally(
              __ -> {
                if (payload.refCnt() > 0) {
                  payload.release();
                }
              });
    } catch (Throwable t) {
      payload.release();
      return Mono.error(t);
    }
  }

  @Override
  public Flux<Payload> requestStream(Payload payload) {
    try {
      RequestRpcMetadata requestRpcMetadata = decodeRequestRpcMetadata(payload);

      RequestContext requestContext = createRequestContext(requestRpcMetadata.getOtherMetadata());

      ServerRequestPayload requestPayload =
          deserializeRequest(payload, requestRpcMetadata, requestContext);

      assert requestPayload.getRequestRpcMetadata().getKind()
          == RpcKind.SINGLE_REQUEST_STREAMING_RESPONSE;

      return rpcServerHandler
          .singleRequestStreamingResponse(requestPayload)
          .map(responsePayload -> handleStreamResponse(alloc, requestPayload, responsePayload))
          .doFinally(
              __ -> {
                if (payload.refCnt() > 0) {
                  payload.release();
                }
              });
    } catch (Throwable t) {
      payload.release();
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

  private static RequestContext createRequestContext(Map<String, String> requestHeaders) {
    ConnectionContext connectionContext = new NiftyConnectionContext();
    return new NettyNiftyRequestContext(requestHeaders, connectionContext);
  }

  @SuppressWarnings("rawtypes")
  private static ServerRequestPayload deserializeRequest(
      Payload payload, RequestRpcMetadata requestRpcMetadata, RequestContext requestContext) {
    ByteBufTProtocol out =
        TProtocolType.fromProtocolId(requestRpcMetadata.getProtocol()).apply(payload.sliceData());
    Function<List<Reader>, List<Object>> readerTransformer = createReaderFunction(out);
    return ServerRequestPayload.create(readerTransformer, requestRpcMetadata, requestContext);
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
    try {
      RequestRpcMetadata requestRpcMetadata = decodeRequestRpcMetadata(payload);

      RequestContext requestContext = createRequestContext(requestRpcMetadata.getOtherMetadata());

      ServerRequestPayload requestPayload =
          deserializeRequest(payload, requestRpcMetadata, requestContext);

      assert requestPayload.getRequestRpcMetadata().getKind() == RpcKind.SINGLE_REQUEST_NO_RESPONSE;

      return rpcServerHandler
          .singleRequestNoResponse(requestPayload)
          .doFinally(
              __ -> {
                if (payload.refCnt() > 0) {
                  payload.release();
                }
              });
    } catch (Throwable t) {
      payload.release();
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
