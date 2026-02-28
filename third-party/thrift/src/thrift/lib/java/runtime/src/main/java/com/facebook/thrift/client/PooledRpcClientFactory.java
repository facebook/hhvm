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

import java.net.SocketAddress;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.function.Function;
import reactor.core.publisher.Mono;

public class PooledRpcClientFactory implements RpcClientFactory {
  private final Map<SocketAddress, PooledRpcClientMono> pools;
  private final RpcClientFactory delegate;
  private final Function<String, Mono<List<SocketAddress>>> hostSelectFunction;
  private final int poolSize;

  public PooledRpcClientFactory(
      RpcClientFactory delegate,
      Function<String, Mono<List<SocketAddress>>> hostSelectFunction,
      int poolSize) {
    this.pools = new ConcurrentHashMap<>();
    this.delegate = delegate;
    this.hostSelectFunction = hostSelectFunction;
    this.poolSize = poolSize;
  }

  @Override
  public Mono<RpcClient> createRpcClient(SocketAddress socketAddress) {
    return createRpcClient((TierSocketAddress) socketAddress);
  }

  public Mono<RpcClient> createRpcClient(TierSocketAddress socketAddress) {
    return pools.computeIfAbsent(
        socketAddress,
        __ ->
            new PooledRpcClientMono(
                delegate, hostSelectFunction, socketAddress.getTierName(), poolSize));
  }
}
