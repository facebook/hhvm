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

package com.facebook.thrift.client;

import com.facebook.swift.service.ThriftClientEventHandler;
import com.facebook.thrift.payload.ClientRequestPayload;
import com.facebook.thrift.payload.ClientResponsePayload;
import com.facebook.thrift.payload.Reader;
import com.facebook.thrift.payload.Writer;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import org.apache.thrift.ClientPushMetadata;
import org.apache.thrift.RequestRpcMetadata;
import org.apache.thrift.client.ClientRequestContext;
import org.reactivestreams.Publisher;
import reactor.core.publisher.Flux;
import reactor.core.publisher.Mono;

/** This class wraps an RpcClient and executes ThriftClientEventHandler with the calls. */
public class EventHandlerRpcClient extends DelegatingRpcClient {
  private final List<? extends ThriftClientEventHandler> eventHandlers;

  public EventHandlerRpcClient(
      RpcClient delegate, List<? extends ThriftClientEventHandler> eventHandlers) {
    super(delegate);
    this.eventHandlers = eventHandlers;
  }

  @Override
  public <T> Mono<ClientResponsePayload<T>> singleRequestSingleResponse(
      ClientRequestPayload<T> payload, RpcOptions options) {
    final EventHandlerClientRequestPayload<T> requestPayload =
        new EventHandlerClientRequestPayload<>(payload, eventHandlers);
    return getDelegate()
        .singleRequestSingleResponse(requestPayload, options)
        .doOnNext(
            clientResponsePayload -> {
              requestPayload.setResponseHeaders(clientResponsePayload.getHeaders());
              if (clientResponsePayload.getException() != null) {
                requestPayload.onError(clientResponsePayload.getException());
              }
            })
        .doOnError(requestPayload::onError)
        .doFinally(__ -> requestPayload.done());
  }

  @Override
  public <T> Mono<Void> singleRequestNoResponse(
      ClientRequestPayload<T> payload, RpcOptions options) {
    final EventHandlerClientRequestPayload<T> requestPayload =
        new EventHandlerClientRequestPayload<>(payload, eventHandlers);
    return getDelegate()
        .singleRequestNoResponse(requestPayload, options)
        .doOnError(requestPayload::onError)
        .doFinally(__ -> requestPayload.done());
  }

  @Override
  public Mono<Void> metadataPush(ClientPushMetadata clientMetadata, RpcOptions options) {
    return getDelegate().metadataPush(clientMetadata, options);
  }

  @Override
  public <T, K> Flux<ClientResponsePayload<K>> singleRequestStreamingResponse(
      ClientRequestPayload<T> payload, RpcOptions options) {
    return getDelegate().singleRequestStreamingResponse(payload, options);
  }

  @Override
  public <T, K> Flux<ClientResponsePayload<K>> streamingRequestStreamingResponse(
      Publisher<ClientRequestPayload<T>> payloads, RpcOptions options) {
    return getDelegate().streamingRequestStreamingResponse(payloads, options);
  }

  private static class EventHandlerClientRequestPayload<T>
      implements ClientRequestContext, ClientRequestPayload<T> {
    private final ClientRequestPayload<T> delegate;
    private final String methodName;
    private final List<Object> contexts;
    private final List<? extends ThriftClientEventHandler> handlers;
    private final Object[] args = new Object[0];
    private final Map<String, String> requestHeaders = new HashMap<>();

    private Map<String, String> responseHeaders;

    public EventHandlerClientRequestPayload(
        ClientRequestPayload<T> delegate, List<? extends ThriftClientEventHandler> handlers) {
      this.delegate = delegate;
      this.methodName = delegate.getRequestRpcMetadata().getName();
      this.handlers = handlers;
      List<Object> contexts = new ArrayList<>();
      for (ThriftClientEventHandler h : this.handlers) {
        contexts.add(h.getContext(methodName, this));
      }
      this.contexts = contexts;
    }

    public void done() {
      for (int i = 0; i < handlers.size(); i++) {
        handlers.get(i).done(contexts.get(i), methodName);
      }
    }

    public void onError(Throwable t) {
      for (int i = 0; i < handlers.size(); i++) {
        handlers.get(i).onError(contexts.get(i), methodName, t);
      }
    }

    public void setResponseHeaders(Map<String, String> responseHeaders) {
      this.responseHeaders = responseHeaders;
    }

    @Override
    public String getServiceName() {
      return delegate.getServiceName();
    }

    @Override
    public Writer getDataWriter() {
      return protocol -> {
        for (int i = 0; i < handlers.size(); i++) {
          handlers.get(i).preWrite(contexts.get(i), methodName, args);
        }
        delegate.getDataWriter().write(protocol);
        for (int i = 0; i < handlers.size(); i++) {
          handlers.get(i).postWrite(contexts.get(i), methodName, args);
        }
      };
    }

    @Override
    public Reader<T> getResponseReader() {
      return protocol -> {
        for (int i = 0; i < handlers.size(); i++) {
          handlers.get(i).preRead(contexts.get(i), methodName);
        }
        T read = delegate.getResponseReader().read(protocol);
        for (int i = 0; i < handlers.size(); i++) {
          handlers.get(i).postRead(contexts.get(i), methodName, read);
        }
        return read;
      };
    }

    @Override
    @SuppressWarnings("rawtypes")
    public Reader getFirstResponseReader() {
      return delegate.getFirstResponseReader();
    }

    @Override
    @SuppressWarnings("rawtypes")
    public Map<Short, Reader> getExceptionReaders() {
      return delegate.getExceptionReaders();
    }

    @Override
    @SuppressWarnings("rawtypes")
    public Map<Short, Reader> getStreamExceptionReaders() {
      return delegate.getStreamExceptionReaders();
    }

    @Override
    public RequestRpcMetadata getRequestRpcMetadata() {
      RequestRpcMetadata requestRpcMetadata = delegate.getRequestRpcMetadata();
      Map<String, String> allHeaders = requestRpcMetadata.getOtherMetadata();
      allHeaders.putAll(requestHeaders);
      return new RequestRpcMetadata.Builder(requestRpcMetadata)
          .setOtherMetadata(allHeaders)
          .build();
    }

    @Override
    public Map<String, String> getPersistentHeaders() {
      return delegate.getPersistentHeaders();
    }

    @Override
    public void setRequestHeader(String key, String value) {
      requestHeaders.put(key, value);
    }

    @Override
    public Map<String, String> getResponseHeaders() {
      return responseHeaders;
    }
  }
}
