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

import static com.facebook.swift.service.SwiftConstants.STICKY_HASH_KEY;

import java.util.concurrent.atomic.AtomicIntegerFieldUpdater;
import reactor.core.CoreSubscriber;
import reactor.core.publisher.Mono;
import reactor.util.context.Context;

public class SimpleLoadBalancingRpcClientMono extends Mono<RpcClient> {
  private static final AtomicIntegerFieldUpdater<SimpleLoadBalancingRpcClientMono> INDEX =
      AtomicIntegerFieldUpdater.newUpdater(SimpleLoadBalancingRpcClientMono.class, "index");

  private final int numOfClient;
  private final Mono<RpcClient>[] clients;

  private volatile int index = 0;

  public SimpleLoadBalancingRpcClientMono(Mono<RpcClient>[] clients) {
    this.clients = clients;
    this.numOfClient = clients.length;
  }

  @Override
  public void subscribe(CoreSubscriber<? super RpcClient> actual) {
    Context context = actual.currentContext();
    select(context).subscribe(actual);
  }

  private Mono<RpcClient> select(Context context) {
    int index = 0;
    if (!context.isEmpty() && context.hasKey(STICKY_HASH_KEY)) {
      Object obj = context.get(STICKY_HASH_KEY);
      index = obj.hashCode() % numOfClient;
    } else {
      index =
          INDEX.getAndUpdate(
              this,
              operand -> {
                final int r = operand + 1;
                return r >= numOfClient ? 0 : r;
              });
    }
    return clients[index];
  }
}
