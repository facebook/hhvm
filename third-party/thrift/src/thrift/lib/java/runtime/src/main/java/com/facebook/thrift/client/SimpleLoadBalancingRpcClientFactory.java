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
import reactor.core.publisher.Mono;

public class SimpleLoadBalancingRpcClientFactory implements RpcClientFactory {
  private final RpcClientFactory delegate;
  private final int poolSize;

  public SimpleLoadBalancingRpcClientFactory(RpcClientFactory delegate) {
    this(delegate, RpcResources.getNumEventLoopThreads());
  }

  public SimpleLoadBalancingRpcClientFactory(RpcClientFactory delegate, int poolSize) {
    this.delegate = delegate;
    this.poolSize = poolSize;
  }

  @Override
  @SuppressWarnings("unchecked")
  public Mono<RpcClient> createRpcClient(SocketAddress socketAddress) {
    Mono<RpcClient>[] clients = new Mono[poolSize];
    for (int i = 0; i < poolSize; i++) {
      clients[i] = delegate.createRpcClient(socketAddress);
    }

    return new SimpleLoadBalancingRpcClientMono(clients);
  }
}
