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

package com.facebook.thrift.server.generated;

import com.facebook.swift.service.ContextChain;
import com.facebook.thrift.payload.Reader;
import com.facebook.thrift.payload.ServerRequestPayload;
import com.facebook.thrift.payload.ServerResponsePayload;
import com.facebook.thrift.payload.ThriftSerializable;
import com.facebook.thrift.payload.Writer;
import com.facebook.thrift.util.RpcPayloadUtil;
import com.facebook.thrift.util.Writers;
import com.facebook.thrift.util.resources.RpcResources;
import java.nio.charset.StandardCharsets;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.function.BiConsumer;
import org.apache.thrift.PayloadMetadata;
import org.apache.thrift.PayloadResponseMetadata;
import org.apache.thrift.RequestRpcMetadata;
import org.apache.thrift.StreamPayloadMetadata;
import org.apache.thrift.TApplicationException;
import org.apache.thrift.TBaseException;
import org.apache.thrift.protocol.TField;
import org.reactivestreams.Publisher;
import reactor.core.Exceptions;
import reactor.core.publisher.Flux;
import reactor.core.publisher.Mono;
import reactor.core.publisher.SynchronousSink;

@SuppressWarnings({"rawtypes", "deprecation"})
abstract class StreamResponseHandlerTemplate<T> {
  private static final Class[] NO_KNOWN_EXCEPTIONS = new Class[0];

  private final SingleRequestStreamResponseDelegate<T> delegate;
  private final List<Reader> readers;
  private final String name;
  private final Class<? extends TBaseException>[] knownExceptions;

  @SafeVarargs
  @SuppressWarnings({"unchecked"})
  public StreamResponseHandlerTemplate(
      SingleRequestStreamResponseDelegate<T> delegate,
      List<Reader> readers,
      String name,
      Class<? extends TBaseException>... knownExceptions) {
    this.delegate = delegate;
    this.readers = readers;
    this.name = name;
    this.knownExceptions = knownExceptions == null ? NO_KNOWN_EXCEPTIONS : knownExceptions;
  }

  protected abstract BiConsumer<T, SynchronousSink<ServerResponsePayload>> getStreamResponseHandler(
      ServerRequestPayload requestPayload, ContextChain chain);

  protected abstract boolean isFirstResponseEmpty();

  @SuppressWarnings("unused")
  public final Flux<ServerResponsePayload> handleStream(
      ServerRequestPayload requestPayload, ContextChain chain) {
    chain.preRead();
    List<Object> data = requestPayload.getData(readers);

    chain.postRead(data);

    Flux<T> stream = delegate.apply(data);

    if (RpcResources.isForceExecutionOffEventLoop()) {
      stream = stream.subscribeOn(RpcResources.getOffLoopScheduler());
    }

    Flux<ServerResponsePayload> responseFlux =
        stream
            .handle(getStreamResponseHandler(requestPayload, chain))
            .switchIfEmpty(handleEmpty(requestPayload.getRequestRpcMetadata(), chain))
            .onErrorResume(throwable -> handleException(throwable, name, requestPayload, chain));

    if (isFirstResponseEmpty()) {
      responseFlux =
          Mono.fromSupplier(() -> createEmptyFirstResponse(requestPayload))
              .concatWith(responseFlux);
    }

    return responseFlux;
  }

  private ServerResponsePayload createEmptyFirstResponse(ServerRequestPayload requestPayload) {
    StreamPayloadMetadata streamPayloadMetadata = createStreamPayloadMetadata(requestPayload);
    return ServerResponsePayload.create(Writers.emptyStruct(), null, streamPayloadMetadata, false);
  }

  private int isKnownException(Throwable t) {
    if (t instanceof ThriftSerializable) {
      Class<? extends Throwable> aClass = t.getClass();
      for (int i = 0; i < knownExceptions.length; i++) {
        if (aClass == knownExceptions[i]) {
          return i + 1;
        }
      }
    }
    return -1;
  }

  private Writer createKnownExceptionWriter(Throwable t, ContextChain chain, final int fieldId) {
    return protocol -> {
      try {
        chain.declaredUserException(t);
        protocol.writeStructBegin(RpcPayloadUtil.TSTRUCT);
        protocol.writeFieldBegin(new TField("responseField", (byte) 12, (short) fieldId));
        ((ThriftSerializable) t).write0(protocol);
        protocol.writeFieldEnd();
        protocol.writeFieldStop();
        protocol.writeStructEnd();
        chain.postWriteException(t);
      } catch (Throwable e) {
        throw Exceptions.propagate(e);
      }
    };
  }

  private Mono<ServerResponsePayload> handleEmpty(
      RequestRpcMetadata requestRpcMetadata, ContextChain chain) {
    return Mono.fromSupplier(
        () -> {
          TApplicationException tApplicationException =
              new TApplicationException(
                  TApplicationException.MISSING_RESULT, "method " + name + " returned null");

          return RpcPayloadUtil.fromTApplicationException(
              tApplicationException, requestRpcMetadata, chain);
        });
  }

  private Publisher<? extends ServerResponsePayload> handleException(
      Throwable throwable, String name, ServerRequestPayload requestPayload, ContextChain chain) {

    int fieldId = isKnownException(throwable);
    if (fieldId == -1) {
      return handleUnknownException(throwable, name, requestPayload, chain);
    } else {
      return handleKnownException(throwable, requestPayload, chain, fieldId);
    }
  }

  private Publisher<? extends ServerResponsePayload> handleUnknownException(
      Throwable throwable, String name, ServerRequestPayload requestPayload, ContextChain chain) {
    String errorMessage =
        String.format(
            "Internal error processing %s: %s",
            name, throwable.getMessage() == null ? "<null>" : throwable.getMessage());
    TApplicationException exception =
        new TApplicationException(TApplicationException.INTERNAL_ERROR, errorMessage);
    exception.initCause(throwable);
    ServerResponsePayload serverResponsePayload =
        RpcPayloadUtil.fromTApplicationException(
            exception, requestPayload.getRequestRpcMetadata(), chain);
    return Mono.just(serverResponsePayload);
  }

  private Publisher<? extends ServerResponsePayload> handleKnownException(
      Throwable throwable, ServerRequestPayload requestPayload, ContextChain chain, int fieldId) {
    Writer knownExceptionWriter = createKnownExceptionWriter(throwable, chain, fieldId);
    return Mono.just(
        RpcPayloadUtil.createServerResponsePayload(
            requestPayload, knownExceptionWriter, throwable.getMessage()));
  }

  protected static Map<String, byte[]> convertOtherData(Map<String, String> otherData) {
    if (otherData == null || otherData.isEmpty()) {
      return Collections.emptyMap();
    } else {
      Map<String, byte[]> converted = new HashMap<>(otherData.size());
      for (Map.Entry<String, String> entry : otherData.entrySet()) {
        converted.put(entry.getKey(), entry.getValue().getBytes(StandardCharsets.UTF_8));
      }
      return converted;
    }
  }

  protected static StreamPayloadMetadata createStreamPayloadMetadata(
      ServerRequestPayload requestPayload) {
    return new StreamPayloadMetadata.Builder()
        .setOtherMetadata(
            convertOtherData(requestPayload.getRequestRpcMetadata().getOtherMetadata()))
        .setPayloadMetadata(
            PayloadMetadata.fromResponseMetadata(PayloadResponseMetadata.defaultInstance()))
        .build();
  }
}
