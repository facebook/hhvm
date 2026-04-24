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
import static org.junit.jupiter.api.Assertions.assertInstanceOf;
import static org.junit.jupiter.api.Assertions.assertNull;
import static org.junit.jupiter.api.Assertions.assertSame;
import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.junit.jupiter.api.Assertions.assertTrue;

import com.facebook.thrift.client.RpcClient;
import com.facebook.thrift.client.RpcClientFactory;
import java.lang.reflect.Field;
import java.net.InetSocketAddress;
import java.net.SocketAddress;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicReference;
import org.junit.jupiter.api.Test;
import reactor.core.publisher.Mono;
import reactor.core.publisher.Sinks;
import reactor.test.StepVerifier;

public class SingleRpcClientManagerTest {
  private static final SocketAddress ADDRESS =
      InetSocketAddress.createUnresolved("localhost", 8080);

  @Test
  public void testAcquireReusesLiveClientAndCreatesReplacementAfterClose() {
    TestRpcClient client1 = new TestRpcClient("c1");
    TestRpcClient client2 = new TestRpcClient("c2");
    AtomicInteger createCalls = new AtomicInteger(0);
    RpcClientFactory factory =
        __ -> Mono.just(createCalls.getAndIncrement() == 0 ? client1 : client2);

    SingleRpcClientManager manager = new SingleRpcClientManager(factory, ADDRESS);

    assertSame(client1, manager.acquire().block());
    assertSame(client1, manager.acquire().block());

    client1.dispose();

    assertSame(client2, manager.acquire().block());
    assertEquals(2, createCalls.get());
  }

  @Test
  public void testDisposeClosesCachedClientAndRejectsFutureAcquire() {
    TestRpcClient client = new TestRpcClient("cached");
    SingleRpcClientManager manager = new SingleRpcClientManager(__ -> Mono.just(client), ADDRESS);

    assertSame(client, manager.acquire().block());

    manager.dispose();

    assertTrue(client.isDisposed());
    StepVerifier.create(manager.acquire()).expectError(IllegalStateException.class).verify();
  }

  @Test
  public void testConcurrentWaitersShareHandleConnectionSideEffects() throws Exception {
    Sinks.One<RpcClient> connectSink = Sinks.one();
    SingleRpcClientManager manager =
        new SingleRpcClientManager(__ -> connectSink.asMono(), ADDRESS);

    Mono<RpcClient> first = manager.acquire();
    Mono<RpcClient> second = manager.acquire();

    assertSame(first, second);

    AtomicReference<RpcClient> firstValue = new AtomicReference<>();
    AtomicReference<RpcClient> secondValue = new AtomicReference<>();
    AtomicReference<Throwable> firstError = new AtomicReference<>();
    AtomicReference<Throwable> secondError = new AtomicReference<>();
    CountDownLatch done = new CountDownLatch(2);

    first.subscribe(
        client -> firstValue.set(client),
        error -> {
          firstError.set(error);
          done.countDown();
        },
        done::countDown);
    second.subscribe(
        client -> secondValue.set(client),
        error -> {
          secondError.set(error);
          done.countDown();
        },
        done::countDown);

    TestRpcClient client = new TestRpcClient("shared");
    connectSink.tryEmitValue(client);

    assertTrue(done.await(5, TimeUnit.SECONDS));
    assertNull(firstError.get());
    assertNull(secondError.get());
    assertSame(client, firstValue.get());
    assertSame(client, secondValue.get());
    assertEquals(1, client.getOnCloseSubscriptionCount());
  }

  @Test
  public void testLateConnectCompletionIsRejectedAndDisposedAfterShutdown() throws Exception {
    Sinks.One<RpcClient> connectSink = Sinks.one();
    Sinks.One<Void> subscribedSink = Sinks.one();
    RpcClientFactory factory =
        __ ->
            Mono.<RpcClient>create(
                sink -> {
                  subscribedSink.tryEmitEmpty();
                  connectSink.asMono().subscribe(sink::success, sink::error);
                });

    SingleRpcClientManager manager = new SingleRpcClientManager(factory, ADDRESS);

    CompletableFuture<RpcClient> acquireFuture =
        CompletableFuture.supplyAsync(() -> manager.acquire().block());

    subscribedSink.asMono().block();

    manager.dispose();

    TestRpcClient lateClient = new TestRpcClient("late");
    connectSink.tryEmitValue(lateClient);

    ExecutionException ex =
        assertThrows(ExecutionException.class, () -> acquireFuture.get(5, TimeUnit.SECONDS));
    assertInstanceOf(IllegalStateException.class, ex.getCause());

    assertTrue(lateClient.isDisposed());
    assertNull(getCachedClient(manager));
    StepVerifier.create(manager.acquire()).expectError(IllegalStateException.class).verify();
  }

  private static RpcClient getCachedClient(SingleRpcClientManager manager) throws Exception {
    Field field = SingleRpcClientManager.class.getDeclaredField("rpcClient");
    field.setAccessible(true);
    return (RpcClient) field.get(manager);
  }

  private static final class TestRpcClient implements RpcClient {
    private final String id;
    private final AtomicInteger disposeCalls = new AtomicInteger(0);
    private final AtomicInteger onCloseSubscriptions = new AtomicInteger(0);
    private final Sinks.One<Void> onClose = Sinks.one();

    private TestRpcClient(String id) {
      this.id = id;
    }

    @Override
    public Mono<Void> onClose() {
      return Mono.defer(
          () -> {
            onCloseSubscriptions.incrementAndGet();
            return onClose.asMono();
          });
    }

    @Override
    public void dispose() {
      if (disposeCalls.getAndIncrement() == 0) {
        onClose.tryEmitEmpty();
      }
    }

    @Override
    public boolean isDisposed() {
      return disposeCalls.get() > 0;
    }

    private int getOnCloseSubscriptionCount() {
      return onCloseSubscriptions.get();
    }

    @Override
    public String toString() {
      return id;
    }
  }
}
