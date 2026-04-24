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
import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertSame;
import static org.junit.jupiter.api.Assertions.assertTrue;

import com.facebook.thrift.client.RpcClient;
import org.junit.jupiter.api.Test;
import reactor.core.publisher.Mono;

public class SimpleLoadBalancingRpcClientManagerTest {

  @Test
  public void testRoundRobinAcquire() {
    FixedRpcClientManager[] managers =
        new FixedRpcClientManager[] {
          new FixedRpcClientManager(new TestRpcClient("c1")),
          new FixedRpcClientManager(new TestRpcClient("c2")),
          new FixedRpcClientManager(new TestRpcClient("c3")),
        };

    SimpleLoadBalancingRpcClientManager manager = new SimpleLoadBalancingRpcClientManager(managers);

    assertSame(managers[0].client, manager.acquire().block());
    assertSame(managers[1].client, manager.acquire().block());
    assertSame(managers[2].client, manager.acquire().block());
    assertSame(managers[0].client, manager.acquire().block());
  }

  @Test
  public void testStickyAcquirePinsConnection() {
    FixedRpcClientManager[] managers =
        new FixedRpcClientManager[] {
          new FixedRpcClientManager(new TestRpcClient("c1")),
          new FixedRpcClientManager(new TestRpcClient("c2")),
          new FixedRpcClientManager(new TestRpcClient("c3")),
        };

    SimpleLoadBalancingRpcClientManager manager = new SimpleLoadBalancingRpcClientManager(managers);

    RpcClient client1 =
        manager.acquire().contextWrite(ctx -> ctx.put(STICKY_HASH_KEY, 12345)).block();
    RpcClient client2 =
        manager.acquire().contextWrite(ctx -> ctx.put(STICKY_HASH_KEY, 12345)).block();

    assertSame(client1, client2);
  }

  @Test
  public void testDisposeClosesChildren() {
    FixedRpcClientManager[] managers =
        new FixedRpcClientManager[] {
          new FixedRpcClientManager(new TestRpcClient("c1")),
          new FixedRpcClientManager(new TestRpcClient("c2")),
        };

    SimpleLoadBalancingRpcClientManager manager = new SimpleLoadBalancingRpcClientManager(managers);

    manager.dispose();

    assertTrue(manager.isDisposed());
    assertEquals(1, managers[0].disposeCalls);
    assertEquals(1, managers[1].disposeCalls);
  }

  private static final class FixedRpcClientManager implements RpcClientManager {
    private final RpcClient client;
    private int disposeCalls;

    private FixedRpcClientManager(RpcClient client) {
      this.client = client;
    }

    @Override
    public Mono<RpcClient> acquire() {
      return Mono.just(client);
    }

    @Override
    public Mono<Void> onClose() {
      return Mono.empty();
    }

    @Override
    public void dispose() {
      disposeCalls++;
    }

    @Override
    public boolean isDisposed() {
      return disposeCalls > 0;
    }
  }

  private static final class TestRpcClient implements RpcClient {
    private final String id;

    private TestRpcClient(String id) {
      this.id = id;
    }

    @Override
    public Mono<Void> onClose() {
      return Mono.empty();
    }

    @Override
    public void dispose() {}

    @Override
    public boolean isDisposed() {
      return false;
    }

    @Override
    public String toString() {
      return id;
    }
  }
}
