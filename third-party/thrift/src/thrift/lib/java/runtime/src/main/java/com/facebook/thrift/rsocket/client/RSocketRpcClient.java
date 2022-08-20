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

package com.facebook.thrift.rsocket.client;

import static com.facebook.thrift.rsocket.util.MetadataUtil.decodePayloadMetadata;
import static com.facebook.thrift.rsocket.util.MetadataUtil.decodeStreamingPayloadMetadata;
import static com.facebook.thrift.rsocket.util.PayloadUtil.createPayload;
import static com.facebook.thrift.util.RpcClientUtils.getExceptionString;
import static com.facebook.thrift.util.RpcClientUtils.getUndeclaredException;

import com.facebook.thrift.client.RpcClient;
import com.facebook.thrift.client.RpcOptions;
import com.facebook.thrift.payload.ClientRequestPayload;
import com.facebook.thrift.payload.ClientResponsePayload;
import com.facebook.thrift.payload.Writer;
import com.facebook.thrift.protocol.ByteBufTProtocol;
import com.facebook.thrift.protocol.TProtocolType;
import com.facebook.thrift.util.RpcClientUtils;
import io.netty.buffer.ByteBuf;
import io.netty.buffer.ByteBufAllocator;
import io.rsocket.Payload;
import io.rsocket.RSocket;
import io.rsocket.util.ByteBufPayload;
import java.util.Optional;
import java.util.function.Function;
import org.apache.thrift.ClientPushMetadata;
import org.apache.thrift.ProtocolId;
import org.apache.thrift.ResponseRpcMetadata;
import org.apache.thrift.StreamPayloadMetadata;
import org.apache.thrift.TException;
import org.apache.thrift.protocol.TStruct;
import org.reactivestreams.Publisher;
import reactor.core.Exceptions;
import reactor.core.publisher.Flux;
import reactor.core.publisher.Mono;

public final class RSocketRpcClient implements RpcClient {
  private static final TStruct PARAMETERS_STRUCT = new TStruct();

  private final RSocket rsocket;
  private final ByteBufAllocator alloc;

  RSocketRpcClient(final RSocket rsocket) {
    this.rsocket = rsocket;
    this.alloc = ByteBufAllocator.DEFAULT;
  }

  @Override
  public Mono<Void> onClose() {
    return rsocket.onClose();
  }

  @Override
  public void close() {
    rsocket.dispose();
  }

  @Override
  public <T> Mono<ClientResponsePayload<T>> singleRequestSingleResponse(
      ClientRequestPayload<T> payload, RpcOptions options) {
    try {
      final ProtocolId protocol = payload.getRequestRpcMetadata().getProtocol();
      final TProtocolType protocolType = TProtocolType.fromProtocolId(protocol);

      return rsocket
          .requestResponse(clientRequestPayloadToRSocketPayload(payload, protocolType))
          .onErrorResume(
              t ->
                  Mono.just(
                      ByteBufPayload.create(
                          getExceptionString(t, payload.getRequestRpcMetadata().getName()))))
          .map(response -> rsocketPayloadToClientResponsePayload(payload, response, protocolType));
    } catch (Throwable t) {
      return Mono.error(t);
    }
  }

  @Override
  public <T> Mono<Void> singleRequestNoResponse(
      ClientRequestPayload<T> payload, RpcOptions options) {
    try {
      final ProtocolId protocol = payload.getRequestRpcMetadata().getProtocol();
      final TProtocolType protocolType = TProtocolType.fromProtocolId(protocol);

      return rsocket.fireAndForget(clientRequestPayloadToRSocketPayload(payload, protocolType));
    } catch (Throwable t) {
      return Mono.error(t);
    }
  }

  @Override
  public <T, K> Flux<ClientResponsePayload<K>> singleRequestStreamingResponse(
      ClientRequestPayload<T> payload, RpcOptions options) {
    try {
      final ProtocolId protocol = payload.getRequestRpcMetadata().getProtocol();
      final TProtocolType protocolType = TProtocolType.fromProtocolId(protocol);

      return rsocket
          .requestStream(clientRequestPayloadToRSocketPayload(payload, protocolType))
          .onErrorResume(
              t ->
                  Flux.just(
                      ByteBufPayload.create(
                          getExceptionString(t, payload.getRequestRpcMetadata().getName()))))
          .map(new StreamingResponseHandler<>(payload));
    } catch (Throwable t) {
      return Flux.error(t);
    }
  }

  @Override
  public <T, K> Flux<ClientResponsePayload<K>> streamingRequestStreamingResponse(
      Publisher<ClientRequestPayload<T>> payloads, RpcOptions options) {
    Flux<ClientResponsePayload<K>> clientResponsePayloadFlux =
        Flux.from(payloads)
            .switchOnFirst(
                (signal, flux) -> {
                  final ClientRequestPayload<T> clientRequestPayload = signal.get();

                  final ProtocolId protocol =
                      clientRequestPayload.getRequestRpcMetadata().getProtocol();
                  final TProtocolType protocolType = TProtocolType.fromProtocolId(protocol);

                  final Flux<Payload> payloadFlux =
                      flux.map(t -> clientRequestPayloadToRSocketPayload(t, protocolType));

                  Flux<Payload> requestChannel = rsocket.requestChannel(payloadFlux);

                  return requestChannel
                      .onErrorResume(
                          t ->
                              Flux.just(
                                  ByteBufPayload.create(
                                      getExceptionString(
                                          t,
                                          clientRequestPayload.getRequestRpcMetadata().getName()))))
                      .map(new StreamingResponseHandler<>(clientRequestPayload));
                });

    return clientResponsePayloadFlux;
  }

  @Override
  public Mono<Void> metadataPush(ClientPushMetadata clientMetadata, RpcOptions options) {
    ByteBuf metadata = null;
    try {
      metadata = alloc.buffer();
      ByteBufTProtocol metadataProtocol = TProtocolType.TCompact.apply(metadata);
      clientMetadata.write0(metadataProtocol);

      Payload payload = ByteBufPayload.create(alloc.buffer(), metadata);
      return rsocket.metadataPush(payload);
    } catch (Throwable t) {
      return Mono.error(t);
    }
  }

  private <T> Payload clientRequestPayloadToRSocketPayload(
      ClientRequestPayload<T> payload, TProtocolType protocolType) {
    ByteBuf data = null;
    ByteBuf metadata = null;
    try {
      data = alloc.buffer();
      metadata = alloc.buffer();

      final ByteBufTProtocol in = protocolType.apply(data);
      in.writeStructBegin(PARAMETERS_STRUCT);

      final Writer writer = payload.getDataWriter();
      writer.write(in);

      in.writeFieldStop();
      in.writeStructEnd();

      final ByteBufTProtocol metadataProtocol = TProtocolType.TCompact.apply(metadata);
      payload.getRequestRpcMetadata().write0(metadataProtocol);

      return createPayload(alloc, payload.getRequestRpcMetadata().getCompression(), data, metadata);

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

  private <T> ClientResponsePayload<T> rsocketPayloadToClientResponsePayload(
      ClientRequestPayload<T> requestPayload, Payload response, TProtocolType protocolType) {
    try {
      final ResponseRpcMetadata rpcMetadata = decodePayloadMetadata(response);

      Optional<? extends TException> undeclaredException = getUndeclaredException(rpcMetadata);
      if (undeclaredException.isPresent()) {
        return ClientResponsePayload.createException(
            undeclaredException.get(), rpcMetadata, null, false);
      }

      final ByteBufTProtocol out = protocolType.apply(response.sliceData());

      return RpcClientUtils.decodeRSocketPayload(
          requestPayload.getResponseReader(),
          requestPayload.getExceptionReaders(),
          out,
          rpcMetadata);
    } catch (Exception e) {
      throw Exceptions.propagate(e);
    } finally {
      if (response.refCnt() > 0) {
        response.release();
      }
    }
  }

  private static class StreamingResponseHandler<T, K>
      implements Function<Payload, ClientResponsePayload<K>> {

    private final ClientRequestPayload<T> requestPayload;
    private boolean isFirstPayload = true;

    private StreamingResponseHandler(ClientRequestPayload<T> requestPayload) {
      this.requestPayload = requestPayload;
    }

    @Override
    public ClientResponsePayload<K> apply(Payload response) {
      try {
        StreamPayloadMetadata streamPayloadMetadata;
        ResponseRpcMetadata rpcMetadata;
        if (isFirstPayload) {
          streamPayloadMetadata = StreamPayloadMetadata.defaultInstance();
          rpcMetadata = decodePayloadMetadata(response);
        } else {
          streamPayloadMetadata = decodeStreamingPayloadMetadata(response);
          rpcMetadata = ResponseRpcMetadata.defaultInstance();
        }

        Optional<? extends TException> undeclaredException = getUndeclaredException(rpcMetadata);
        if (undeclaredException.isPresent()) {
          return ClientResponsePayload.createException(
              undeclaredException.get(), rpcMetadata, streamPayloadMetadata, true);
        }
        ClientResponsePayload<K> responsePayload =
            RpcClientUtils.decodeStreamPayload(
                requestPayload, response, streamPayloadMetadata, rpcMetadata, isFirstPayload);
        isFirstPayload = false;
        return responsePayload;
      } catch (Exception e) {
        throw Exceptions.propagate(e);
      } finally {
        if (response.refCnt() > 0) {
          response.release();
        }
      }
    }
  }
}
