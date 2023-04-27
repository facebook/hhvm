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
import org.apache.thrift.ErrorBlame;
import org.apache.thrift.ErrorClassification;
import org.apache.thrift.PayloadAppUnknownExceptionMetdata;
import org.apache.thrift.PayloadDeclaredExceptionMetadata;
import org.apache.thrift.PayloadExceptionMetadata;
import org.apache.thrift.PayloadExceptionMetadataBase;
import org.apache.thrift.PayloadMetadata;
import org.apache.thrift.PayloadResponseMetadata;
import org.apache.thrift.ResponseRpcMetadata;
import org.apache.thrift.StreamPayloadMetadata;
import org.apache.thrift.TApplicationException;
import org.apache.thrift.TBaseException;
import org.apache.thrift.protocol.TField;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import reactor.core.Exceptions;
import reactor.core.publisher.Flux;
import reactor.core.publisher.Mono;

@SuppressWarnings({"rawtypes", "deprecation"})
abstract class StreamResponseHandlerTemplate<T> {
  private static final Logger logger = LoggerFactory.getLogger(StreamResponseHandlerTemplate.class);
  private static final Class[] NO_KNOWN_EXCEPTIONS = new Class[0];
  private final SingleRequestStreamResponseDelegate<T> delegate;
  private final List<Reader> readers;
  private final String name;
  private final Class<? extends TBaseException>[] functionExceptions;
  private final Class<? extends TBaseException>[] streamExceptions;
  private final Integer[] functionExceptionIds;
  private final Integer[] streamExceptionIds;

  @SuppressWarnings({"unchecked"})
  public StreamResponseHandlerTemplate(
      SingleRequestStreamResponseDelegate<T> delegate,
      List<Reader> readers,
      String name,
      Class<? extends TBaseException>[] functionExceptions,
      Integer[] functionExceptionIds,
      Class<? extends TBaseException>[] streamExceptions,
      Integer[] streamExceptionIds) {
    this.delegate = delegate;
    this.readers = readers;
    this.name = name;
    this.functionExceptions = functionExceptions == null ? NO_KNOWN_EXCEPTIONS : functionExceptions;
    this.functionExceptionIds = functionExceptionIds;
    this.streamExceptions = streamExceptions == null ? NO_KNOWN_EXCEPTIONS : streamExceptions;
    this.streamExceptionIds = streamExceptionIds;
  }

  protected abstract StreamHandler<T> getStreamResponseHandler(
      ServerRequestPayload requestPayload, ContextChain chain);

  protected abstract boolean isFirstResponseEmpty();

  @SuppressWarnings("unused")
  public final Flux<ServerResponsePayload> handleStream(
      ServerRequestPayload requestPayload, ContextChain chain) {
    final StreamHandler<T> handler = getStreamResponseHandler(requestPayload, chain);
    try {
      chain.preRead();
      List<Object> data = requestPayload.getData(readers);
      chain.postRead(data);

      Flux<T> stream = delegate.apply(data);

      if (RpcResources.isForceExecutionOffEventLoop()) {
        stream = stream.subscribeOn(RpcResources.getOffLoopScheduler());
      }

      Flux<ServerResponsePayload> responseFlux =
          stream
              .handle(handler)
              .onErrorResume(
                  throwable -> handleException(throwable, name, requestPayload, chain, handler));

      if (isFirstResponseEmpty()) {
        responseFlux =
            Mono.fromSupplier(() -> createEmptyFirstResponse(requestPayload))
                .concatWith(responseFlux);
        handler.firstResponseProcessed = true;
      }

      return responseFlux;
    } catch (Throwable t) {
      logger.warn("Can not handle stream", t);
      return handleException(t, name, requestPayload, chain, handler);
    }
  }

  private int isKnownException(Throwable t) {
    if (t instanceof ThriftSerializable) {
      Class<? extends Throwable> aClass = t.getClass();
      for (int i = 0; i < functionExceptions.length; i++) {
        if (aClass == functionExceptions[i]) {
          return functionExceptionIds[i];
        }
      }
      for (int i = 0; i < streamExceptions.length; i++) {
        if (aClass == streamExceptions[i]) {
          return streamExceptionIds[i];
        }
      }
    }
    return -1;
  }

  private Flux<ServerResponsePayload> handleException(
      Throwable throwable,
      String name,
      ServerRequestPayload requestPayload,
      ContextChain chain,
      StreamHandler handler) {
    int fieldId = isKnownException(throwable);
    if (fieldId == -1) {
      return handleUnknownException(
          throwable, name, requestPayload, chain, handler.isFirstResponseProcessed());
    }
    return handleKnownException(
        throwable, requestPayload, chain, fieldId, handler.isFirstResponseProcessed());
  }

  private Flux<ServerResponsePayload> handleKnownException(
      Throwable throwable,
      ServerRequestPayload requestPayload,
      ContextChain chain,
      int fieldId,
      boolean streamException) {
    Writer knownExceptionWriter = createKnownExceptionWriter(throwable, chain, fieldId);
    if (streamException) {
      return Flux.just(
          ServerResponsePayload.createWithTApplicationException(
              knownExceptionWriter,
              null,
              createStreamMetadataDeclaredException(requestPayload),
              true));
    }
    return Flux.just(
        ServerResponsePayload.createWithTApplicationException(
            knownExceptionWriter, createRpcMetadataDeclaredException(requestPayload), null, false));
  }

  protected static ResponseRpcMetadata createResponseRpcMetadata(
      ServerRequestPayload requestPayload) {
    return new ResponseRpcMetadata.Builder()
        .setPayloadMetadata(
            PayloadMetadata.fromResponseMetadata(new PayloadResponseMetadata.Builder().build()))
        .build();
  }

  private ServerResponsePayload createEmptyFirstResponse(ServerRequestPayload requestPayload) {
    ResponseRpcMetadata metadata = createResponseRpcMetadata(requestPayload);
    return ServerResponsePayload.create(Writers.emptyStruct(), metadata, null, true);
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

  private Writer createUnknownExceptionWriter(TApplicationException t, ContextChain chain) {
    return protocol -> {
      try {
        chain.preWriteException(t);
        chain.undeclaredUserException(t);
        t.write(protocol);
        chain.postWriteException(t);
      } catch (Throwable e) {
        throw Exceptions.propagate(e);
      }
    };
  }

  private Flux<ServerResponsePayload> handleUnknownException(
      Throwable throwable,
      String name,
      ServerRequestPayload requestPayload,
      ContextChain chain,
      boolean streamException) {
    String errorMessage =
        String.format(
            "Internal error processing %s: %s",
            name, throwable.getMessage() == null ? "<null>" : throwable.getMessage());
    TApplicationException exception =
        new TApplicationException(TApplicationException.INTERNAL_ERROR, errorMessage);
    exception.initCause(throwable);

    Writer unknownExceptionWriter = createUnknownExceptionWriter(exception, chain);

    if (streamException) {
      return Flux.just(
          ServerResponsePayload.createWithTApplicationException(
              unknownExceptionWriter,
              null,
              createStreamMetadataUndeclaredException(
                  requestPayload, throwable.getClass().getName(), throwable.getMessage()),
              true));
    }

    return Flux.just(
        ServerResponsePayload.createWithTApplicationException(
            unknownExceptionWriter,
            createRpcMetadataUndeclaredException(
                requestPayload, throwable.getClass().getName(), throwable.getMessage()),
            null,
            false));
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

  private static StreamPayloadMetadata createStreamMetadataDeclaredException(
      ServerRequestPayload requestPayload) {
    return new StreamPayloadMetadata.Builder()
        .setOtherMetadata(
            convertOtherData(requestPayload.getRequestRpcMetadata().getOtherMetadata()))
        .setPayloadMetadata(
            PayloadMetadata.fromExceptionMetadata(
                new PayloadExceptionMetadataBase.Builder()
                    .setMetadata(
                        PayloadExceptionMetadata.fromDeclaredException(
                            new PayloadDeclaredExceptionMetadata.Builder()
                                .setErrorClassification(
                                    new ErrorClassification.Builder()
                                        .setBlame(ErrorBlame.SERVER)
                                        .build())
                                .build()))
                    .build()))
        .build();
  }

  private static StreamPayloadMetadata createStreamMetadataUndeclaredException(
      ServerRequestPayload requestPayload, String name, String msg) {
    return new StreamPayloadMetadata.Builder()
        .setOtherMetadata(
            convertOtherData(requestPayload.getRequestRpcMetadata().getOtherMetadata()))
        .setPayloadMetadata(
            PayloadMetadata.fromExceptionMetadata(
                new PayloadExceptionMetadataBase.Builder()
                    .setNameUtf8(name)
                    .setWhatUtf8(msg)
                    .setMetadata(
                        PayloadExceptionMetadata.fromAppUnknownException(
                            new PayloadAppUnknownExceptionMetdata.Builder()
                                .setErrorClassification(
                                    new ErrorClassification.Builder()
                                        .setBlame(ErrorBlame.SERVER)
                                        .build())
                                .build()))
                    .build()))
        .build();
  }

  private static ResponseRpcMetadata createRpcMetadataDeclaredException(
      ServerRequestPayload requestPayload) {
    return new ResponseRpcMetadata.Builder()
        .setOtherMetadata(requestPayload.getRequestRpcMetadata().getOtherMetadata())
        .setPayloadMetadata(
            PayloadMetadata.fromExceptionMetadata(
                new PayloadExceptionMetadataBase.Builder()
                    .setMetadata(
                        PayloadExceptionMetadata.fromDeclaredException(
                            new PayloadDeclaredExceptionMetadata.Builder()
                                .setErrorClassification(
                                    new ErrorClassification.Builder()
                                        .setBlame(ErrorBlame.SERVER)
                                        .build())
                                .build()))
                    .build()))
        .build();
  }

  private static ResponseRpcMetadata createRpcMetadataUndeclaredException(
      ServerRequestPayload requestPayload, String name, String msg) {
    return new ResponseRpcMetadata.Builder()
        .setOtherMetadata(requestPayload.getRequestRpcMetadata().getOtherMetadata())
        .setPayloadMetadata(
            PayloadMetadata.fromExceptionMetadata(
                new PayloadExceptionMetadataBase.Builder()
                    .setNameUtf8(name)
                    .setWhatUtf8(msg)
                    .setMetadata(
                        PayloadExceptionMetadata.fromAppUnknownException(
                            new PayloadAppUnknownExceptionMetdata.Builder()
                                .setErrorClassification(
                                    new ErrorClassification.Builder()
                                        .setBlame(ErrorBlame.SERVER)
                                        .build())
                                .build()))
                    .build()))
        .build();
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
