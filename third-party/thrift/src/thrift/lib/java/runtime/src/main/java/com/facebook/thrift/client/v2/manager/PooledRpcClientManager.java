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

import static com.facebook.swift.service.SwiftConstants.STICKY_HASH_KEY;

import com.facebook.thrift.client.RpcClient;
import com.facebook.thrift.util.resources.RpcResources;
import java.net.SocketAddress;
import java.time.Duration;
import java.util.ArrayList;
import java.util.Collections;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.concurrent.atomic.AtomicIntegerFieldUpdater;
import java.util.concurrent.atomic.AtomicReference;
import java.util.function.Function;
import reactor.core.Disposable;
import reactor.core.publisher.Flux;
import reactor.core.publisher.Mono;
import reactor.core.publisher.Sinks;
import reactor.util.context.ContextView;
import reactor.util.retry.Retry;

/**
 * Manages direct-to-host connections for a service tier whose host list changes over time.
 *
 * <p>The manager periodically refreshes the tier's resolved hosts via {@code hostSelectFunction}.
 * For each currently selected host it reuses or creates a child manager, up to {@code poolSize}
 * hosts. {@link #acquire()} then picks one current child manager by sticky hash or round-robin.
 *
 * <p>This is the manager-layer replacement for the older {@code PooledRpcClientMono}. It is used
 * for direct Service Router clients that bypass the local proxy and therefore need Java to track
 * the current backend host set itself.
 */
public final class PooledRpcClientManager extends AbstractRpcClientManager {
  private static final AtomicIntegerFieldUpdater<PooledRpcClientManager> CURRENT_INDEX =
      AtomicIntegerFieldUpdater.newUpdater(PooledRpcClientManager.class, "currentIndex");

  private static final Duration POLL_RATE = Duration.ofMinutes(1L);

  private final RpcClientManagerFactory delegateFactory;
  private final Function<String, Mono<List<SocketAddress>>> hostSelectFunction;
  private final String tierName;
  private final int poolSize;
  private final Sinks.Many<List<RpcClientManager>> managersSink;
  private final AtomicReference<Map<SocketAddress, RpcClientManager>> managersByAddress;
  private final Disposable refreshDisposable;

  private volatile int currentIndex;

  public PooledRpcClientManager(
      RpcClientManagerFactory delegateFactory,
      Function<String, Mono<List<SocketAddress>>> hostSelectFunction,
      String tierName,
      int poolSize) {
    if (poolSize < 1) {
      throw new IllegalArgumentException("0 or negative connection pool size is not allowed");
    }

    this.delegateFactory = Objects.requireNonNull(delegateFactory);
    this.hostSelectFunction = Objects.requireNonNull(hostSelectFunction);
    this.tierName = Objects.requireNonNull(tierName);
    this.poolSize = poolSize;
    this.managersSink = Sinks.many().replay().latest();
    this.managersByAddress = new AtomicReference<>(Collections.emptyMap());
    this.refreshDisposable =
        Flux.interval(Duration.ZERO, POLL_RATE, RpcResources.getOffLoopScheduler())
            .concatMap(__ -> refresh())
            .retryWhen(
                Retry.backoff(Long.MAX_VALUE, Duration.ofSeconds(1))
                    .maxBackoff(Duration.ofSeconds(10))
                    .jitter(0.5))
            .subscribe();
  }

  /**
   * Acquires a transport from one of the currently active host managers. Blocks reactively until
   * the first host refresh completes (via {@code managersSink}), then selects a child manager by
   * sticky-hash or round-robin and delegates to its {@code acquire()}.
   */
  @Override
  public Mono<RpcClient> acquire() {
    if (isDisposed()) {
      return closedMono();
    }

    return Mono.deferContextual(
        context ->
            managersSink
                .asFlux()
                .next()
                .flatMap(managers -> acquireFromManagers(managers, context)));
  }

  private Mono<RpcClient> acquireFromManagers(
      List<RpcClientManager> managers, ContextView contextView) {
    if (isDisposed()) {
      return closedMono();
    }
    if (managers.isEmpty()) {
      return Mono.error(new IllegalStateException("No rpc clients available for tier " + tierName));
    }
    return select(managers, contextView).acquire();
  }

  private RpcClientManager select(List<RpcClientManager> managers, ContextView context) {
    if (context.hasKey(STICKY_HASH_KEY)) {
      return managers.get(Math.floorMod(context.get(STICKY_HASH_KEY).hashCode(), managers.size()));
    }

    int selected = Math.floorMod(CURRENT_INDEX.incrementAndGet(this), managers.size());
    return managers.get(selected);
  }

  private Mono<Void> refresh() {
    return hostSelectFunction
        .apply(tierName)
        .filter(addresses -> !addresses.isEmpty())
        .doOnNext(this::replaceManagers)
        .then();
  }

  /**
   * Reconciles the current manager set with a fresh address list. Managers for addresses that are
   * still present are reused. New addresses get fresh managers. Addresses that disappeared have
   * their managers disposed, which cleanly shuts down the underlying transports.
   */
  private void replaceManagers(List<SocketAddress> addresses) {
    Map<SocketAddress, RpcClientManager> previous = managersByAddress.get();
    Map<SocketAddress, RpcClientManager> next = new LinkedHashMap<>();
    int limit = Math.min(poolSize, addresses.size());

    for (int i = 0; i < limit; i++) {
      SocketAddress address = addresses.get(i);
      RpcClientManager manager = previous.get(address);
      if (manager == null || manager.isDisposed()) {
        manager = delegateFactory.createRpcClientManager(address);
      }
      next.put(address, manager);
    }

    managersByAddress.set(Collections.unmodifiableMap(next));

    for (Map.Entry<SocketAddress, RpcClientManager> entry : previous.entrySet()) {
      if (!next.containsKey(entry.getKey())) {
        entry.getValue().dispose();
      }
    }

    managersSink.tryEmitNext(new ArrayList<>(next.values()));
  }

  @Override
  protected void doDispose() {
    refreshDisposable.dispose();

    Map<SocketAddress, RpcClientManager> current =
        managersByAddress.getAndSet(Collections.emptyMap());
    for (RpcClientManager manager : current.values()) {
      manager.dispose();
    }
  }
}
