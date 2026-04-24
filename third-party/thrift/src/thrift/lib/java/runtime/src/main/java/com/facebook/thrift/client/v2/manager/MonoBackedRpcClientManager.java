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
import java.util.Objects;
import java.util.concurrent.atomic.AtomicReference;
import reactor.core.publisher.Mono;

/**
 * Adapts an existing {@code Mono<RpcClient>} source into the manager abstraction.
 *
 * <p>This is primarily a compatibility bridge while older Mono-based connection sources still
 * exist. The desired long-lived lifecycle abstraction for typed clients is {@link
 * RpcClientManager}, not a bare {@code Mono<RpcClient>}, because managers can be explicitly closed,
 * shared, and composed.
 *
 * <p>Use this when the source already knows how to hand out a live {@link RpcClient}. Disposing
 * this manager only affects the handle it adapts: it disposes the last acquired transport it has
 * observed, but it does not assume ownership of the source Mono's own lifecycle. Prefer dedicated
 * manager implementations for new lifecycle logic.
 */
public final class MonoBackedRpcClientManager extends AbstractRpcClientManager {
  private final Mono<RpcClient> source;
  private final AtomicReference<RpcClient> currentClient = new AtomicReference<>();

  public MonoBackedRpcClientManager(Mono<? extends RpcClient> source) {
    this.source = Mono.<RpcClient>from(Objects.requireNonNull(source)).doOnNext(currentClient::set);
  }

  @Override
  public Mono<RpcClient> acquire() {
    if (isDisposed()) {
      return closedMono();
    }
    return source;
  }

  /**
   * Disposes the last-seen transport. Only the most recently emitted RpcClient is tracked via
   * {@code doOnNext(currentClient::set)}, so this is correct for single-connection sources but will
   * only dispose one slot of a multi-connection source. Multi-connection lifecycle should use
   * dedicated manager types like {@link SimpleLoadBalancingRpcClientManager} instead.
   */
  @Override
  protected void doDispose() {
    RpcClient rpcClient = currentClient.getAndSet(null);
    if (rpcClient != null && !rpcClient.isDisposed()) {
      rpcClient.dispose();
    }
  }
}
