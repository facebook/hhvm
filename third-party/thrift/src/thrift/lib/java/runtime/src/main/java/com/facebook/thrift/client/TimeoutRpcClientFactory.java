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

import com.facebook.thrift.payload.ClientRequestPayload;
import com.facebook.thrift.payload.ClientResponsePayload;
import com.facebook.thrift.util.MonoTimeoutTransformer;
import com.facebook.thrift.util.resources.RpcResources;
import com.google.common.annotations.VisibleForTesting;
import java.net.SocketAddress;
import java.util.Objects;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;
import reactor.core.publisher.Mono;

/**
 * RpcClientFactory that wraps another RpcClient and applies a timeout to the request. It uses the
 * default timeout request timeout from {@link ThriftClientConfig} and applies that to requests. The
 * default timeout can be overridden using {@link RpcOptions} with a client timeout, or queue
 * timeout, or both. Since there isn't a concept of a queue timeout in the Thrift Java client, both
 * times outs add added together if supplied.
 */
public class TimeoutRpcClientFactory extends DelegatingRpcClientFactory {
  private final long defaultClientTimeoutMs;
  private final long connectionTimeoutMs;

  public TimeoutRpcClientFactory(RpcClientFactory delegate, ThriftClientConfig config) {
    super(delegate);

    Objects.requireNonNull(config, "config is null");
    Objects.requireNonNull(config.getRequestTimeout(), "request time out is null");

    this.defaultClientTimeoutMs = config.getRequestTimeout().toMillis();
    this.connectionTimeoutMs = config.getConnectTimeout().toMillis();

    validateTimeout(defaultClientTimeoutMs);
  }

  @Override
  public Mono<RpcClient> createRpcClient(SocketAddress socketAddress) {
    return getDelegate()
        .createRpcClient(socketAddress)
        .transform(
            new MonoTimeoutTransformer<>(
                RpcResources.getEventLoopGroup().next(),
                connectionTimeoutMs,
                TimeUnit.MILLISECONDS))
        .onErrorMap(
            TimeoutException.class,
            e -> new TimeoutException(e.getMessage() + " for 'client_connection'"))
        .map(this::wrapRpcClient);
  }

  private TimeoutRpcClient wrapRpcClient(RpcClient rpcClient) {
    return new TimeoutRpcClient(rpcClient, defaultClientTimeoutMs);
  }

  @VisibleForTesting
  static void validateTimeout(long timeoutMs) {
    if (timeoutMs < 1) {
      throw new IllegalArgumentException("timeout must be in milliseconds and greater then 0");
    }
  }

  static class TimeoutRpcClient extends DelegatingRpcClient {
    private final long defaultClientTimeoutMs;

    public TimeoutRpcClient(RpcClient delegate, long defaultClientTimeoutMs) {
      super(delegate);
      this.defaultClientTimeoutMs = defaultClientTimeoutMs;
    }

    private <T> Mono<T> applyTimeout(Mono<T> target, RpcOptions options) {
      long timeoutMs = calculateRequestTimeout(options);
      return target.transform(
          new MonoTimeoutTransformer<>(
              RpcResources.getEventLoopGroup().next(), timeoutMs, TimeUnit.MILLISECONDS));
    }

    @VisibleForTesting
    long calculateRequestTimeout(RpcOptions options) {
      Objects.requireNonNull(options);

      int clientTimeoutMs = getClientTimeoutMs(options);
      int queueTimeoutMs = getQueueTimeoutMs(options);

      if (clientTimeoutMs + queueTimeoutMs > 0) {
        int timeout = clientTimeoutMs + queueTimeoutMs;
        validateTimeout(timeout);
        return timeout;
      } else {
        return defaultClientTimeoutMs;
      }
    }

    @VisibleForTesting
    int getClientTimeoutMs(RpcOptions options) {
      Integer clientTimeoutMs = options.getClientTimeoutMs();
      return clientTimeoutMs != null ? clientTimeoutMs : 0;
    }

    @VisibleForTesting
    int getQueueTimeoutMs(RpcOptions options) {
      Integer queueTimeoutMs = options.getQueueTimeoutMs();
      return queueTimeoutMs != null ? queueTimeoutMs : 0;
    }

    @Override
    public <T> Mono<ClientResponsePayload<T>> singleRequestSingleResponse(
        ClientRequestPayload<T> payload, RpcOptions options) {
      return applyTimeout(getDelegate().singleRequestSingleResponse(payload, options), options)
          .onErrorMap(TimeoutException.class, t -> getTimeoutException(payload, t));
    }

    @Override
    public <T> Mono<Void> singleRequestNoResponse(
        ClientRequestPayload<T> payload, RpcOptions options) {
      return applyTimeout(getDelegate().singleRequestNoResponse(payload, options), options)
          .onErrorMap(TimeoutException.class, t -> getTimeoutException(payload, t));
    }

    private <T> TimeoutException getTimeoutException(ClientRequestPayload<T> payload, Throwable t) {
      String messageSuffix =
          payload != null && payload.getRequestRpcMetadata() != null
              ? " for Client: '" + payload.getRequestRpcMetadata().getName() + "'"
              : " for Client";
      return new TimeoutException(t.getMessage() + messageSuffix);
    }
  }
}
