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
import com.facebook.thrift.model.StreamResponse;
import com.facebook.thrift.payload.Reader;
import com.facebook.thrift.payload.ServerRequestPayload;
import com.facebook.thrift.payload.ServerResponsePayload;
import com.facebook.thrift.payload.Writer;
import java.util.List;
import java.util.function.BiConsumer;
import org.apache.thrift.StreamPayloadMetadata;
import org.apache.thrift.TBaseException;
import reactor.core.publisher.SynchronousSink;

@SuppressWarnings({"rawtypes", "unused", "deprecation"})
public final class StreamWithFirstResponseHandler<T, K>
    extends StreamResponseHandlerTemplate<StreamResponse<T, K>> {
  private final ResponseWriterFactory firstResponseWriterFactory;
  private final ResponseWriterFactory responseWriterFactory;

  @SafeVarargs
  public StreamWithFirstResponseHandler(
      SingleRequestStreamResponseDelegate<StreamResponse<T, K>> delegate,
      ResponseWriterFactory firstResponseWriterFactory,
      ResponseWriterFactory responseWriterFactory,
      List<Reader> readers,
      String name,
      Class<? extends TBaseException>... knownExceptions) {
    super(delegate, readers, name, knownExceptions);
    this.firstResponseWriterFactory = firstResponseWriterFactory;
    this.responseWriterFactory = responseWriterFactory;
  }

  @Override
  protected boolean isFirstResponseEmpty() {
    return false;
  }

  @Override
  protected BiConsumer<StreamResponse<T, K>, SynchronousSink<ServerResponsePayload>>
      getStreamResponseHandler(ServerRequestPayload requestPayload, ContextChain chain) {
    return new InnerStreamResponseHandler<>(
        requestPayload, chain, firstResponseWriterFactory, responseWriterFactory);
  }

  private static class InnerStreamResponseHandler<T, K>
      implements BiConsumer<StreamResponse<T, K>, SynchronousSink<ServerResponsePayload>> {
    private boolean firstResponseProcessed = false;

    private final ServerRequestPayload requestPayload;

    private final ContextChain chain;

    private final ResponseWriterFactory firstResponseWriterFactory;

    private final ResponseWriterFactory responseWriterFactory;

    public InnerStreamResponseHandler(
        ServerRequestPayload requestPayload,
        ContextChain chain,
        ResponseWriterFactory firstResponseWriterFactory,
        ResponseWriterFactory responseWriterFactory) {
      this.requestPayload = requestPayload;
      this.chain = chain;
      this.firstResponseWriterFactory = firstResponseWriterFactory;
      this.responseWriterFactory = responseWriterFactory;
    }

    @Override
    public void accept(
        StreamResponse<T, K> streamResponse, SynchronousSink<ServerResponsePayload> sink) {
      if (firstResponseProcessed) {
        handle(streamResponse, sink);
      } else {
        handleFirst(streamResponse, sink);
      }
    }

    private void handleFirst(
        StreamResponse<T, K> streamResponse, SynchronousSink<ServerResponsePayload> sink) {
      firstResponseProcessed = true;
      T firstResponse = streamResponse.getFirstResponse();
      if (firstResponse == null) {
        sink.error(new NullPointerException("the first response must not be null"));
      } else {
        Writer writer =
            firstResponseWriterFactory.createResponseWriter(firstResponse, chain, requestPayload);
        doHandle(writer, sink);
      }
    }

    private void handle(
        StreamResponse<T, K> streamResponse, SynchronousSink<ServerResponsePayload> sink) {
      K data = streamResponse.getData();
      if (data == null) {
        sink.error(new NullPointerException("the response is must not be null"));
      } else {
        Writer writer = responseWriterFactory.createResponseWriter(data, chain, requestPayload);
        doHandle(writer, sink);
      }
    }

    private void doHandle(Writer writer, SynchronousSink<ServerResponsePayload> sink) {
      StreamPayloadMetadata metadata = createStreamPayloadMetadata(requestPayload);
      ServerResponsePayload payload = ServerResponsePayload.create(writer, null, metadata, true);
      sink.next(payload);
    }
  }
}
