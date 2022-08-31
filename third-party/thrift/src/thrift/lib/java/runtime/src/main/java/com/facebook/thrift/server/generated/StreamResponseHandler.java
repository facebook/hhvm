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
import com.facebook.thrift.payload.Writer;
import java.util.List;
import java.util.function.BiConsumer;
import org.apache.thrift.PayloadMetadata;
import org.apache.thrift.PayloadResponseMetadata;
import org.apache.thrift.StreamPayloadMetadata;
import org.apache.thrift.TBaseException;
import reactor.core.publisher.SynchronousSink;

@SuppressWarnings({"rawtypes", "unused", "deprecation"})
public class StreamResponseHandler<T> extends StreamResponseHandlerTemplate<T> {
  private final ResponseWriterFactory responseWriterFactory;

  @SafeVarargs
  public StreamResponseHandler(
      SingleRequestStreamResponseDelegate<T> delegate,
      ResponseWriterFactory responseWriterFactory,
      List<Reader> readers,
      String name,
      Class<? extends TBaseException>... knownExceptions) {
    super(delegate, readers, name, knownExceptions);
    this.responseWriterFactory = responseWriterFactory;
  }

  @Override
  protected boolean isFirstResponseEmpty() {
    return true;
  }

  @Override
  protected BiConsumer<T, SynchronousSink<ServerResponsePayload>> getStreamResponseHandler(
      ServerRequestPayload requestPayload, ContextChain chain) {
    return new InnerStreamResponseHandler<>(requestPayload, chain, responseWriterFactory);
  }

  private static class InnerStreamResponseHandler<T>
      implements BiConsumer<T, SynchronousSink<ServerResponsePayload>> {

    private final ServerRequestPayload requestPayload;

    private final ContextChain chain;

    private final ResponseWriterFactory responseWriterFactory;

    public InnerStreamResponseHandler(
        ServerRequestPayload requestPayload,
        ContextChain chain,
        ResponseWriterFactory responseWriterFactory) {
      this.requestPayload = requestPayload;
      this.chain = chain;
      this.responseWriterFactory = responseWriterFactory;
    }

    @Override
    public void accept(T data, SynchronousSink<ServerResponsePayload> sink) {
      Writer writer = responseWriterFactory.createResponseWriter(data, chain, requestPayload);
      doHandle(writer, sink);
    }

    private void doHandle(Writer writer, SynchronousSink<ServerResponsePayload> sink) {
      StreamPayloadMetadata metadata = createStreamPayloadMetadata();
      ServerResponsePayload payload = ServerResponsePayload.create(writer, null, metadata, true);
      sink.next(payload);
    }

    private StreamPayloadMetadata createStreamPayloadMetadata() {
      return new StreamPayloadMetadata.Builder()
          .setOtherMetadata(
              convertOtherData(requestPayload.getRequestRpcMetadata().getOtherMetadata()))
          .setPayloadMetadata(
              PayloadMetadata.fromResponseMetadata(PayloadResponseMetadata.defaultInstance()))
          .build();
    }
  }
}
