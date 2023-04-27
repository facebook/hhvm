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
import java.util.HashSet;
import java.util.Set;
import org.junit.Assert;
import org.junit.Test;
import org.mockito.Mockito;
import reactor.core.publisher.Mono;

public class SimpleLoadBalancingRpcClientFactoryTest {
  @Test
  public void testRpcClientFactoryOnlyMakeNumberOfConnectionsEqualToPoolSize() {
    SocketAddress address = Mockito.mock(SocketAddress.class);
    RpcClientFactory mockFactory = Mockito.mock(RpcClientFactory.class);

    Mockito.when(mockFactory.createRpcClient(address))
        .then(
            invocation -> {
              RpcClient mockClient = Mockito.mock(RpcClient.class);
              return Mono.just(mockClient);
            });

    SimpleLoadBalancingRpcClientFactory factory =
        new SimpleLoadBalancingRpcClientFactory(mockFactory, 5);
    Mono<RpcClient> rpcClient = factory.createRpcClient(address);

    Set<RpcClient> clientSet = new HashSet<>();

    for (int i = 0; i < 20; i++) {
      RpcClient block = rpcClient.block();
      Assert.assertNotNull(block);
      clientSet.add(block);
    }

    Assert.assertEquals(5, clientSet.size());
  }
}
