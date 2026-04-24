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
import java.util.function.Function;
import reactor.core.publisher.Mono;

/**
 * Applies a lightweight, stateless {@link RpcClient} wrapper to each acquired client.
 *
 * <p>Use this for consumer-facing adapters such as {@code ExceptionMappingRpcClient}. The
 * decoration is not its own lifecycle root; {@link #dispose()} still applies to the delegate
 * manager, not to the wrapper instances returned from {@link #acquire()}.
 */
public final class DecoratingRpcClientManager implements RpcClientManager {
  private final RpcClientManager delegate;
  private final Function<RpcClient, ? extends RpcClient> decorator;

  public DecoratingRpcClientManager(
      RpcClientManager delegate, Function<RpcClient, ? extends RpcClient> decorator) {
    this.delegate = Objects.requireNonNull(delegate);
    this.decorator = Objects.requireNonNull(decorator);
  }

  @Override
  public Mono<RpcClient> acquire() {
    return delegate.acquire().map(decorator);
  }

  @Override
  public Mono<Void> onClose() {
    return delegate.onClose();
  }

  @Override
  public void dispose() {
    delegate.dispose();
  }

  @Override
  public boolean isDisposed() {
    return delegate.isDisposed();
  }
}
