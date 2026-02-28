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
import reactor.core.publisher.Mono;

public class CachedRpcClientFactory implements RpcClientFactory {
  private final RpcClientFactory delegate;

  public CachedRpcClientFactory(RpcClientFactory delegate) {
    this.delegate = delegate;
  }

  @Override
  public Mono<RpcClient> createRpcClient(SocketAddress socketAddress) {
    // Return a cached Mono that automatically invalidates when the connection closes.
    // Each call creates a new cached Mono instance, allowing load balancing to create
    // separate connections. The cache invalidation prevents stale connection exceptions
    // from being cached indefinitely, which would cause suppressed exceptions to accumulate.
    return delegate.createRpcClient(socketAddress).cacheInvalidateWhen(RpcClient::onClose);
  }
}
