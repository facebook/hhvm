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

import com.facebook.thrift.util.resources.RpcResources;
import java.net.SocketAddress;
import java.time.Duration;
import java.util.List;
import java.util.concurrent.atomic.AtomicIntegerFieldUpdater;
import java.util.function.Function;
import java.util.stream.Collectors;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import reactor.core.CoreSubscriber;
import reactor.core.Disposable;
import reactor.core.publisher.Flux;
import reactor.core.publisher.Mono;
import reactor.core.publisher.Sinks;
import reactor.util.retry.RetrySpec;

public class PooledRpcClientMono extends Mono<RpcClient> {
  private static final Logger LOGGER = LoggerFactory.getLogger(PooledRpcClientMono.class);

  private static final AtomicIntegerFieldUpdater<PooledRpcClientMono> CURRENT_INDEX =
      AtomicIntegerFieldUpdater.newUpdater(PooledRpcClientMono.class, "currentIndex");

  private static final Duration POLL_RATE = Duration.ofMinutes(1L);
  private final RpcClientFactory delegateFactory;
  private final Function<String, Mono<List<SocketAddress>>> hostSelectFunction;
  private final String tierName;

  private final Disposable disposable;

  private final int poolSize;

  private final Sinks.Many<List<Mono<RpcClient>>> rpcClientSink;

  private volatile int currentIndex;

  public PooledRpcClientMono(
      RpcClientFactory delegateFactory,
      Function<String, Mono<List<SocketAddress>>> hostSelectFunction,
      String tierName,
      int poolSize) {
    this.delegateFactory = delegateFactory;
    this.hostSelectFunction = hostSelectFunction;
    this.tierName = tierName;
    this.rpcClientSink = Sinks.many().replay().latest();
    this.disposable = selectHost();
    this.poolSize = poolSize;
  }

  @Override
  public void subscribe(CoreSubscriber<? super RpcClient> actual) {
    rpcClientSink.asFlux().next().flatMap(this::select).subscribe(actual);
  }

  private Mono<RpcClient> select(List<Mono<RpcClient>> select) {
    int i = CURRENT_INDEX.incrementAndGet(this) % select.size();
    return select.get(i);
  }

  private Disposable selectHost() {
    return Flux.interval(Duration.ZERO, POLL_RATE, RpcResources.getOffLoopScheduler())
        .flatMap(__ -> fetch())
        .retryWhen(
            RetrySpec.backoff(Long.MAX_VALUE, Duration.ofSeconds(1))
                .maxBackoff(Duration.ofSeconds(10))
                .jitter(0.5))
        .subscribe();
  }

  private Mono<Void> fetch() {
    return hostSelectFunction
        .apply(tierName)
        .filter(addresses -> !addresses.isEmpty())
        .map(
            addresses ->
                addresses.stream()
                    .map(delegateFactory::createRpcClient)
                    .limit(poolSize)
                    .collect(Collectors.toList()))
        .doOnNext(rpcClientSink::tryEmitNext)
        .then();
  }
}
