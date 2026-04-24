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

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertSame;
import static org.junit.jupiter.api.Assertions.assertTrue;

import com.facebook.thrift.client.RpcClient;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.atomic.AtomicInteger;
import org.junit.jupiter.api.Test;
import reactor.core.publisher.Mono;
import reactor.core.publisher.Sinks;
import reactor.test.StepVerifier;

public class RpcClientBindingTest {

  @Test
  public void testOwnedBindingDisposesManagerAndRejectsFutureAcquires() {
    TestRpcClientManager manager = new TestRpcClientManager();
    RpcClientBinding binding = new RpcClientBinding(manager, ClientOwnership.OWNED);

    assertSame(manager.rpcClient, binding.acquire().block());

    binding.dispose();

    assertTrue(binding.isDisposed());
    assertEquals(1, manager.disposeCalls.get());
    assertTrue(manager.isDisposed());

    StepVerifier.create(binding.acquire()).expectError(IllegalStateException.class).verify();
  }

  @Test
  public void testBorrowedBindingClosesHandleButLeavesManagerAlive() {
    TestRpcClientManager manager = new TestRpcClientManager();
    RpcClientBinding binding = new RpcClientBinding(manager, ClientOwnership.BORROWED);

    binding.dispose();

    assertTrue(binding.isDisposed());
    assertEquals(0, manager.disposeCalls.get());
    assertFalse(manager.isDisposed());

    StepVerifier.create(binding.acquire()).expectError(IllegalStateException.class).verify();

    RpcClientBinding anotherBorrowedBinding =
        new RpcClientBinding(manager, ClientOwnership.BORROWED);
    assertSame(manager.rpcClient, anotherBorrowedBinding.acquire().block());
  }

  private static final class TestRpcClientManager implements RpcClientManager {
    private final TestRpcClient rpcClient = new TestRpcClient();
    private final AtomicBoolean disposed = new AtomicBoolean(false);
    private final AtomicInteger disposeCalls = new AtomicInteger(0);

    @Override
    public Mono<RpcClient> acquire() {
      if (disposed.get()) {
        return Mono.error(new IllegalStateException("manager is closed"));
      }
      return Mono.just(rpcClient);
    }

    @Override
    public Mono<Void> onClose() {
      return Mono.empty();
    }

    @Override
    public void dispose() {
      disposeCalls.incrementAndGet();
      disposed.set(true);
      rpcClient.dispose();
    }

    @Override
    public boolean isDisposed() {
      return disposed.get();
    }
  }

  private static final class TestRpcClient implements RpcClient {
    private final AtomicBoolean disposed = new AtomicBoolean(false);
    private final Sinks.One<Void> onClose = Sinks.one();

    @Override
    public Mono<Void> onClose() {
      return onClose.asMono();
    }

    @Override
    public void dispose() {
      if (disposed.compareAndSet(false, true)) {
        onClose.tryEmitEmpty();
      }
    }

    @Override
    public boolean isDisposed() {
      return disposed.get();
    }
  }
}
