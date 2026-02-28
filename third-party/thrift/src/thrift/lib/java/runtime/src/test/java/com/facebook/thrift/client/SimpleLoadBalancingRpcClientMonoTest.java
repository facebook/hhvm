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

import org.junit.Assert;
import org.junit.Test;
import org.mockito.Mockito;
import reactor.core.publisher.Mono;

public class SimpleLoadBalancingRpcClientMonoTest {
  @Test
  @SuppressWarnings("unchecked")
  public void testSimpleLoadBalancingRpcClientMonoShouldRoundRobinRequests() {
    Mono<RpcClient>[] clients = new Mono[3];
    clients[0] = Mono.just(Mockito.mock(RpcClient.class));
    clients[1] = Mono.just(Mockito.mock(RpcClient.class));
    clients[2] = Mono.just(Mockito.mock(RpcClient.class));

    SimpleLoadBalancingRpcClientMono mono = new SimpleLoadBalancingRpcClientMono(clients);
    Assert.assertEquals(clients[0].block(), mono.block());
    Assert.assertEquals(clients[1].block(), mono.block());
    Assert.assertEquals(clients[2].block(), mono.block());
    Assert.assertEquals(clients[0].block(), mono.block());
  }

  @Test
  @SuppressWarnings("unchecked")
  public void testSimpleLoadBalancingRpcClientMonoShouldPinConnection() {
    Mono<RpcClient>[] clients = new Mono[3];
    clients[0] = Mono.just(Mockito.mock(RpcClient.class));
    clients[1] = Mono.just(Mockito.mock(RpcClient.class));
    clients[2] = Mono.just(Mockito.mock(RpcClient.class));

    SimpleLoadBalancingRpcClientMono mono = new SimpleLoadBalancingRpcClientMono(clients);
    RpcClient client1 = mono.contextWrite(ctx -> ctx.put(STICKY_HASH_KEY, 12345)).block();
    RpcClient client2 = mono.contextWrite(ctx -> ctx.put(STICKY_HASH_KEY, 12345)).block();
    Assert.assertEquals(client1, client2);
  }
}
