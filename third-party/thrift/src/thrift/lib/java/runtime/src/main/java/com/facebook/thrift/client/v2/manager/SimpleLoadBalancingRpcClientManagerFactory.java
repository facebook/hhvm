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

import java.net.SocketAddress;
import java.util.Objects;

/**
 * Creates {@link SimpleLoadBalancingRpcClientManager} instances over a fixed number of child
 * managers.
 *
 * <p>The delegate factory is asked to create {@code poolSize} child managers for the same address.
 * The resulting manager chooses one child per acquire using sticky-hash or round-robin routing.
 */
public final class SimpleLoadBalancingRpcClientManagerFactory implements RpcClientManagerFactory {
  private final RpcClientManagerFactory delegate;
  private final int poolSize;

  public SimpleLoadBalancingRpcClientManagerFactory(
      RpcClientManagerFactory delegate, int poolSize) {
    if (poolSize < 1) {
      throw new IllegalArgumentException("0 or negative connection pool size is not allowed");
    }

    this.delegate = Objects.requireNonNull(delegate);
    this.poolSize = poolSize;
  }

  @Override
  public RpcClientManager createRpcClientManager(SocketAddress socketAddress) {
    RpcClientManager[] managers = new RpcClientManager[poolSize];
    for (int i = 0; i < poolSize; i++) {
      managers[i] = delegate.createRpcClientManager(socketAddress);
    }
    return new SimpleLoadBalancingRpcClientManager(managers);
  }
}
