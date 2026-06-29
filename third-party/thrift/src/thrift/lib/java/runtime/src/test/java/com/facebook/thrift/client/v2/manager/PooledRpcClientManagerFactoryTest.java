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

import static org.junit.jupiter.api.Assertions.assertNotSame;
import static org.junit.jupiter.api.Assertions.assertSame;
import static org.junit.jupiter.api.Assertions.assertTrue;

import com.facebook.thrift.client.TierSocketAddress;
import java.net.SocketAddress;
import java.util.Collections;
import org.junit.jupiter.api.Test;
import reactor.core.publisher.Mono;
import reactor.test.StepVerifier;

public class PooledRpcClientManagerFactoryTest {

  @Test
  public void testReusesOnePoolPerTierAndDisposesAllPools() {
    PooledRpcClientManagerFactory factory =
        new PooledRpcClientManagerFactory(
            socketAddress -> new NoopRpcClientManager(),
            tier -> Mono.just(Collections.<SocketAddress>emptyList()),
            2);

    RpcClientManager foo1 = factory.createRpcClientManager(new TierSocketAddress("foo"));
    RpcClientManager foo2 = factory.createRpcClientManager(new TierSocketAddress("foo"));
    RpcClientManager bar = factory.createRpcClientManager(new TierSocketAddress("bar"));

    assertSame(foo1, foo2);
    assertNotSame(foo1, bar);

    factory.dispose();

    assertTrue(foo1.isDisposed());
    assertTrue(bar.isDisposed());
  }

  @Test
  public void testEmptyDiscoveryFailsFast() {
    // An authoritative empty resolution makes acquire() fail fast rather than hang, and never
    // pins a stale host set.
    PooledRpcClientManager manager =
        new PooledRpcClientManager(
            socketAddress -> new NoopRpcClientManager(),
            tier -> Mono.just(Collections.<SocketAddress>emptyList()),
            "test.tier",
            2);
    try {
      StepVerifier.create(manager.acquire())
          .expectErrorMatches(
              e ->
                  e instanceof IllegalStateException
                      && e.getMessage().contains("No hosts available for tier test.tier"))
          .verify(java.time.Duration.ofSeconds(10));
    } finally {
      manager.dispose();
    }
  }

  @Test
  public void testDiscoveryErrorFailsFastWithNoPriorPool() {
    // A persistent discovery error with no last-known-good pool surfaces a fast failure after the
    // bounded retry burst instead of blocking until the request timeout.
    PooledRpcClientManager manager =
        new PooledRpcClientManager(
            socketAddress -> new NoopRpcClientManager(),
            tier -> Mono.error(new RuntimeException("discovery unavailable")),
            "test.tier",
            2);
    try {
      StepVerifier.create(manager.acquire())
          .expectErrorMatches(
              e ->
                  e instanceof IllegalStateException
                      && e.getMessage().contains("No hosts available for tier test.tier"))
          .verify(java.time.Duration.ofSeconds(15));
    } finally {
      manager.dispose();
    }
  }

  private static final class NoopRpcClientManager implements RpcClientManager {
    private boolean disposed;

    @Override
    public reactor.core.publisher.Mono<com.facebook.thrift.client.RpcClient> acquire() {
      return Mono.error(new UnsupportedOperationException());
    }

    @Override
    public Mono<Void> onClose() {
      return Mono.empty();
    }

    @Override
    public void dispose() {
      disposed = true;
    }

    @Override
    public boolean isDisposed() {
      return disposed;
    }
  }
}
