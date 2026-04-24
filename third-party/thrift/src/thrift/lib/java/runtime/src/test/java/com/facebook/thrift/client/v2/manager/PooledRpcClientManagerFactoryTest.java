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
