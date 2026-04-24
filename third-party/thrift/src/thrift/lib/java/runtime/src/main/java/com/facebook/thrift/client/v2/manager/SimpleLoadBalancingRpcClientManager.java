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
import java.util.Arrays;
import java.util.Objects;
import java.util.concurrent.atomic.AtomicIntegerFieldUpdater;
import reactor.core.publisher.Mono;
import reactor.util.context.ContextView;

/**
 * Selects one child manager per acquire from a fixed set of child managers.
 *
 * <p>Sticky requests route by {@code STICKY_HASH_KEY}. Non-sticky requests route round-robin. This
 * manager does not create transports itself; it composes already-configured child managers and
 * chooses one when a call needs a client.
 */
public final class SimpleLoadBalancingRpcClientManager extends AbstractRpcClientManager {
  private static final AtomicIntegerFieldUpdater<SimpleLoadBalancingRpcClientManager> INDEX =
      AtomicIntegerFieldUpdater.newUpdater(SimpleLoadBalancingRpcClientManager.class, "index");

  private final RpcClientManager[] managers;

  private volatile int index;

  public SimpleLoadBalancingRpcClientManager(RpcClientManager[] managers) {
    Objects.requireNonNull(managers);
    if (managers.length == 0) {
      throw new IllegalArgumentException("At least one child manager is required");
    }
    this.managers = Arrays.copyOf(managers, managers.length);
  }

  @Override
  public Mono<RpcClient> acquire() {
    if (isDisposed()) {
      return closedMono();
    }

    return Mono.deferContextual(context -> select(context).acquire());
  }

  private RpcClientManager select(ContextView context) {
    if (context.hasKey(STICKY_HASH_KEY)) {
      return managers[Math.floorMod(context.get(STICKY_HASH_KEY).hashCode(), managers.length)];
    }

    int selected =
        INDEX.getAndUpdate(
            this,
            operand -> {
              int next = operand + 1;
              return next >= managers.length ? 0 : next;
            });

    return managers[selected];
  }

  @Override
  protected void doDispose() {
    for (RpcClientManager manager : managers) {
      manager.dispose();
    }
  }
}
