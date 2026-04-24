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

package com.facebook.thrift.client.v2.manager;

import com.facebook.thrift.client.RpcClient;
import com.facebook.thrift.client.RpcClientFactory;
import java.net.SocketAddress;
import java.time.Duration;
import reactor.core.publisher.Mono;
import reactor.util.retry.Retry;

/**
 * Manager-native reconnect strategy over a single-address manager.
 *
 * <p>Each {@link #acquire()} asks the underlying single manager for a live client and retries
 * connection establishment with backoff until one is available or the manager is closed.
 */
public final class ReconnectingRpcClientManager extends AbstractRpcClientManager {
  private static final Duration BASE_BACKOFF = Duration.ofMillis(100);
  private static final Duration MAX_BACKOFF = Duration.ofMillis(30_000);

  private final SingleRpcClientManager delegate;

  public ReconnectingRpcClientManager(
      RpcClientFactory transportFactory, SocketAddress socketAddress) {
    this.delegate = new SingleRpcClientManager(transportFactory, socketAddress);
  }

  @Override
  public Mono<RpcClient> acquire() {
    if (isDisposed()) {
      return closedMono();
    }

    Mono<RpcClient> retryingAcquire =
        Mono.defer(delegate::acquire)
            .retryWhen(
                Retry.backoff(Long.MAX_VALUE, BASE_BACKOFF)
                    .maxBackoff(MAX_BACKOFF)
                    .jitter(0.5)
                    .filter(__ -> !isDisposed()));

    return Mono.firstWithSignal(retryingAcquire, onClose().then(Mono.defer(this::closedMono)));
  }

  @Override
  protected void doDispose() {
    delegate.dispose();
  }
}
