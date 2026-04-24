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

import com.facebook.thrift.client.RpcClientFactory;
import java.net.SocketAddress;
import java.util.Objects;

/**
 * Creates {@link SingleRpcClientManager} instances for concrete host addresses.
 *
 * <p>Each created manager owns one address and one live transport at a time.
 */
public final class SingleRpcClientManagerFactory implements RpcClientManagerFactory {
  private final RpcClientFactory transportFactory;

  public SingleRpcClientManagerFactory(RpcClientFactory transportFactory) {
    this.transportFactory = Objects.requireNonNull(transportFactory);
  }

  @Override
  public RpcClientManager createRpcClientManager(SocketAddress socketAddress) {
    return new SingleRpcClientManager(transportFactory, socketAddress);
  }
}
