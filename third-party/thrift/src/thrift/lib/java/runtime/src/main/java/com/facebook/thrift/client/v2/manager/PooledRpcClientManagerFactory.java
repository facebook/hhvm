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

import com.facebook.thrift.client.TierSocketAddress;
import java.net.SocketAddress;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.function.Function;
import reactor.core.publisher.Mono;

/**
 * Reuses one {@link PooledRpcClientManager} per tier address.
 *
 * <p>The delegate factory creates per-host child managers. {@code hostSelectFunction} resolves a
 * tier name to its current backend hosts. Reusing one pooled manager per tier preserves refresh
 * state and child-manager reuse across multiple borrowed typed clients.
 */
public final class PooledRpcClientManagerFactory implements RpcClientManagerFactory {
  private final Map<SocketAddress, PooledRpcClientManager> pools;
  private final RpcClientManagerFactory delegate;
  private final Function<String, Mono<List<SocketAddress>>> hostSelectFunction;
  private final int poolSize;

  public PooledRpcClientManagerFactory(
      RpcClientManagerFactory delegate,
      Function<String, Mono<List<SocketAddress>>> hostSelectFunction,
      int poolSize) {
    this.pools = new ConcurrentHashMap<>();
    this.delegate = delegate;
    this.hostSelectFunction = hostSelectFunction;
    this.poolSize = poolSize;
  }

  @Override
  public RpcClientManager createRpcClientManager(SocketAddress socketAddress) {
    return createRpcClientManager((TierSocketAddress) socketAddress);
  }

  public RpcClientManager createRpcClientManager(TierSocketAddress socketAddress) {
    return pools.computeIfAbsent(
        socketAddress,
        __ ->
            new PooledRpcClientManager(
                delegate, hostSelectFunction, socketAddress.getTierName(), poolSize));
  }

  /** Disposes all cached tier pools managed by this factory. */
  public void dispose() {
    for (PooledRpcClientManager pool : pools.values()) {
      pool.dispose();
    }
    pools.clear();
  }
}
