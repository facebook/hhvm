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

import com.facebook.swift.service.ThriftClientStats;
import com.facebook.thrift.payload.ClientRequestPayload;
import com.facebook.thrift.payload.ClientResponsePayload;
import com.facebook.thrift.payload.Reader;
import com.facebook.thrift.payload.Writer;
import java.util.Map;
import java.util.concurrent.TimeUnit;
import org.apache.thrift.ClientPushMetadata;
import org.apache.thrift.RequestRpcMetadata;
import org.reactivestreams.Publisher;
import reactor.core.publisher.Flux;
import reactor.core.publisher.Mono;
import reactor.core.publisher.SignalType;

/**
 * This class wraps an RpcClient and instruments the calls. It publishes the stats to the {@link
 * ThriftClientStats} class.
 */
public class InstrumentedRpcClient extends DelegatingRpcClient {
  private final ThriftClientStats thriftClientStats;

  public InstrumentedRpcClient(RpcClient delegate, ThriftClientStats thriftClientStats) {
    super(delegate);
    this.thriftClientStats = thriftClientStats;
  }

  private void handleDoFinally(SignalType signalType, String name, long start) {
    if (signalType == SignalType.ON_ERROR) {
      thriftClientStats.error(name);
    }

    if (signalType == SignalType.CANCEL) {
      thriftClientStats.cancel(name);
    }

    thriftClientStats.complete(name, toMicros(System.nanoTime() - start));
  }

  @Override
  public <T> Mono<ClientResponsePayload<T>> singleRequestSingleResponse(
      ClientRequestPayload<T> payload, RpcOptions options) {
    try {
      final String name = payload.getRequestRpcMetadata().getName();
      final long start = System.nanoTime();
      final InstrumentedClientRequestPayload<T> instrumentedClientRequestPayload =
          new InstrumentedClientRequestPayload<>(payload, name, thriftClientStats);
      thriftClientStats.call(name);
      return getDelegate()
          .singleRequestSingleResponse(instrumentedClientRequestPayload, options)
          .doFinally(signalType -> handleDoFinally(signalType, name, start));
    } catch (Throwable t) {
      if (payload != null
          && payload.getRequestRpcMetadata() != null
          && payload.getRequestRpcMetadata().getName() != null) {
        thriftClientStats.error(payload.getRequestRpcMetadata().getName());
      }
      return Mono.error(t);
    }
  }

  @Override
  public <T> Mono<Void> singleRequestNoResponse(
      ClientRequestPayload<T> payload, RpcOptions options) {
    try {
      final String name = payload.getRequestRpcMetadata().getName();
      final long start = System.nanoTime();
      final InstrumentedClientRequestPayload<T> instrumentedClientRequestPayload =
          new InstrumentedClientRequestPayload<>(payload, name, thriftClientStats);
      thriftClientStats.call(name);
      return getDelegate()
          .singleRequestNoResponse(instrumentedClientRequestPayload, options)
          .doFinally(signalType -> handleDoFinally(signalType, name, start));
    } catch (Throwable t) {
      if (payload != null
          && payload.getRequestRpcMetadata() != null
          && payload.getRequestRpcMetadata().getName() != null) {
        thriftClientStats.error(payload.getRequestRpcMetadata().getName());
      }
      return Mono.error(t);
    }
  }

  @Override
  public Mono<Void> metadataPush(ClientPushMetadata clientMetadata, RpcOptions options) {
    return getDelegate().metadataPush(clientMetadata, options);
  }

  @Override
  public <T, K> Flux<ClientResponsePayload<K>> singleRequestStreamingResponse(
      ClientRequestPayload<T> payload, RpcOptions options) {
    try {
      final String name = payload.getRequestRpcMetadata().getName();
      final long start = System.nanoTime();
      final InstrumentedClientRequestPayload<T> instrumentedClientRequestPayload =
          new InstrumentedClientRequestPayload<>(payload, name, thriftClientStats);
      thriftClientStats.call(name);
      return getDelegate()
          .<T, K>singleRequestStreamingResponse(instrumentedClientRequestPayload, options)
          .doFinally(signalType -> handleDoFinally(signalType, name, start));
    } catch (Throwable t) {
      if (payload != null
          && payload.getRequestRpcMetadata() != null
          && payload.getRequestRpcMetadata().getName() != null) {
        thriftClientStats.error(payload.getRequestRpcMetadata().getName());
      }
      return Flux.error(t);
    }
  }

  @Override
  public <T, K> Flux<ClientResponsePayload<K>> streamingRequestStreamingResponse(
      Publisher<ClientRequestPayload<T>> payloads, RpcOptions options) {
    return Flux.from(payloads)
        .switchOnFirst(
            (signal, clientRequestPayloadFlux) -> {
              ClientRequestPayload<T> payload = signal.get();
              try {
                final String name = payload.getRequestRpcMetadata().getName();
                final long start = System.nanoTime();
                thriftClientStats.call(name);
                return getDelegate()
                    .<T, K>streamingRequestStreamingResponse(clientRequestPayloadFlux, options)
                    .doFinally(signalType -> handleDoFinally(signalType, name, start));
              } catch (Throwable t) {
                if (payload != null
                    && payload.getRequestRpcMetadata() != null
                    && payload.getRequestRpcMetadata().getName() != null) {
                  thriftClientStats.error(payload.getRequestRpcMetadata().getName());
                }
                return Flux.error(t);
              }
            });
  }

  private static class InstrumentedClientRequestPayload<T> implements ClientRequestPayload<T> {
    private final ClientRequestPayload<T> delegate;
    private final String name;
    private final ThriftClientStats thriftClientStats;

    public InstrumentedClientRequestPayload(
        ClientRequestPayload<T> delegate, String name, ThriftClientStats thriftClientStats) {
      this.delegate = delegate;
      this.name = name;
      this.thriftClientStats = thriftClientStats;
    }

    @Override
    public String getServiceName() {
      return delegate.getServiceName();
    }

    @Override
    public Writer getDataWriter() {
      long start = System.nanoTime();
      return protocol -> {
        delegate.getDataWriter().write(protocol);
        thriftClientStats.publishWrite(name, toMicros(System.nanoTime() - start));
      };
    }

    @Override
    public Reader<T> getResponseReader() {
      long start = System.nanoTime();
      return protocol -> {
        T read = delegate.getResponseReader().read(protocol);
        thriftClientStats.publishRead(name, toMicros(System.nanoTime() - start));
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
      return delegate.getRequestRpcMetadata();
    }

    @Override
    public Map<String, String> getPersistentHeaders() {
      return delegate.getPersistentHeaders();
    }
  }

  private static long toMicros(long nanoseconds) {
    return TimeUnit.NANOSECONDS.toMicros(nanoseconds);
  }
}
