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

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotEquals;
import static org.junit.Assert.assertNotNull;
import static org.mockito.Mockito.*;

import java.net.InetSocketAddress;
import java.net.SocketAddress;
import org.junit.Test;
import reactor.core.publisher.Mono;
import reactor.core.publisher.Sinks;
import reactor.test.StepVerifier;

public class CachedRpcClientFactoryTest {

  @Test
  public void testCachesWithinSameMono() {
    SocketAddress address = new InetSocketAddress("localhost", 8080);
    RpcClientFactory mockDelegate = mock(RpcClientFactory.class);
    RpcClient mockClient = createMockRpcClient();

    when(mockDelegate.createRpcClient(address)).thenReturn(Mono.just(mockClient));

    CachedRpcClientFactory factory = new CachedRpcClientFactory(mockDelegate);

    // Get the cached Mono once
    Mono<RpcClient> cachedMono = factory.createRpcClient(address);

    // Multiple subscriptions to the SAME Mono should share the cached client
    RpcClient client1 = cachedMono.block();
    assertNotNull(client1);

    RpcClient client2 = cachedMono.block();
    assertNotNull(client2);

    // Both clients should be the same instance
    assertEquals(client1, client2);

    // Verify delegate was only called once for this Mono
    verify(mockDelegate, times(1)).createRpcClient(address);
  }

  @Test
  public void testInvalidatesCacheOnConnectionClose() throws InterruptedException {
    SocketAddress address = new InetSocketAddress("localhost", 8080);
    RpcClientFactory mockDelegate = mock(RpcClientFactory.class);

    // Create two different mock clients to distinguish them
    RpcClient mockClient1 = createMockRpcClient();
    RpcClient mockClient2 = createMockRpcClient();

    // Use AtomicInteger to track which subscription we're on
    // This simulates cache invalidation - first subscription gets client1, second gets client2
    java.util.concurrent.atomic.AtomicInteger subscriptionCount =
        new java.util.concurrent.atomic.AtomicInteger(0);

    // Create a Mono that emits different values on each subscription
    // This is what happens with cache invalidation - after invalidation, next subscription
    // triggers a new fetch
    Mono<RpcClient> sourceMono =
        Mono.defer(
            () -> {
              int count = subscriptionCount.getAndIncrement();
              return count == 0 ? Mono.just(mockClient1) : Mono.just(mockClient2);
            });

    when(mockDelegate.createRpcClient(address)).thenReturn(sourceMono);

    CachedRpcClientFactory factory = new CachedRpcClientFactory(mockDelegate);

    // Get a cached Mono - createRpcClient() is only called once
    Mono<RpcClient> cachedMono = factory.createRpcClient(address);

    // First subscription creates the connection and sets up cache invalidation trigger
    RpcClient client1 = cachedMono.block();
    assertNotNull(client1);
    assertEquals(mockClient1, client1);

    // Verify that onClose() was called by cacheInvalidateWhen to set up invalidation
    verify(mockClient1, atLeastOnce()).onClose();

    // Second subscription uses cached connection (same client)
    RpcClient client1Again = cachedMono.block();
    assertEquals(client1, client1Again);

    // Should have only called delegate createRpcClient() once
    verify(mockDelegate, times(1)).createRpcClient(address);

    // Subscription count should be 1 (only first subscription triggered source Mono)
    assertEquals(1, subscriptionCount.get());

    // Simulate connection close - this triggers the onClose() Mono to complete
    // which cacheInvalidateWhen uses to invalidate the cache
    Sinks.One<Void> closeSink1 = getCloseSinkForClient(mockClient1);
    closeSink1.tryEmitEmpty();

    // Give time for the async invalidation to complete
    // The cacheInvalidateWhen operator subscribes to onClose() and invalidates asynchronously
    Thread.sleep(500);

    // Third subscription after close should get a new connection (cache was invalidated)
    RpcClient client2 = cachedMono.block();
    assertNotNull(client2);

    // Subscription count should now be 2 (cache invalidation triggered second subscription to
    // source)
    assertEquals(2, subscriptionCount.get());

    // Clients should be different instances
    assertNotEquals(client1, client2);
    assertEquals(mockClient2, client2);
  }

  @Test
  public void testReconnectsAfterKeepAliveTimeout() throws InterruptedException {
    SocketAddress address = new InetSocketAddress("localhost", 8080);
    RpcClientFactory mockDelegate = mock(RpcClientFactory.class);

    RpcClient mockClient1 = createMockRpcClient();
    RpcClient mockClient2 = createMockRpcClient();

    // Use AtomicInteger to emit different clients on each subscription
    java.util.concurrent.atomic.AtomicInteger subscriptionCount =
        new java.util.concurrent.atomic.AtomicInteger(0);

    Mono<RpcClient> sourceMono =
        Mono.defer(
            () -> {
              int count = subscriptionCount.getAndIncrement();
              return count == 0 ? Mono.just(mockClient1) : Mono.just(mockClient2);
            });

    when(mockDelegate.createRpcClient(address)).thenReturn(sourceMono);

    CachedRpcClientFactory factory = new CachedRpcClientFactory(mockDelegate);

    // Establish initial connection via cached Mono
    Mono<RpcClient> cachedMono = factory.createRpcClient(address);
    RpcClient client1 = cachedMono.block();
    assertNotNull(client1);

    // Simulate keep-alive timeout by closing the connection
    Sinks.One<Void> closeSink1 = getCloseSinkForClient(mockClient1);
    closeSink1.tryEmitEmpty();

    // Give time for the async invalidation to complete
    Thread.sleep(500);

    // Next subscription should reconnect (cache was invalidated)
    RpcClient client2 = cachedMono.block();
    assertNotNull(client2);
    assertNotEquals(client1, client2);

    // Verify reconnection occurred - subscription count should be 2
    assertEquals(2, subscriptionCount.get());
  }

  @Test
  public void testConcurrentSubscriptionsToSameMono() throws InterruptedException {
    SocketAddress address = new InetSocketAddress("localhost", 8080);
    RpcClientFactory mockDelegate = mock(RpcClientFactory.class);
    RpcClient mockClient = createMockRpcClient();

    when(mockDelegate.createRpcClient(address)).thenReturn(Mono.just(mockClient));

    CachedRpcClientFactory factory = new CachedRpcClientFactory(mockDelegate);

    // Create one cached Mono that will be shared
    Mono<RpcClient> cachedMono = factory.createRpcClient(address);

    // Simulate multiple threads subscribing to the same Mono
    int threadCount = 10;
    Thread[] threads = new Thread[threadCount];
    RpcClient[] results = new RpcClient[threadCount];

    for (int i = 0; i < threadCount; i++) {
      final int index = i;
      threads[i] =
          new Thread(
              () -> {
                results[index] = cachedMono.block();
              });
    }

    // Start all threads at once
    for (Thread thread : threads) {
      thread.start();
    }

    // Wait for all to complete
    for (Thread thread : threads) {
      thread.join();
    }

    // All threads should get the same client (cached)
    for (RpcClient result : results) {
      assertNotNull(result);
      assertEquals(mockClient, result);
    }

    // Delegate should only be called once despite concurrent subscriptions
    verify(mockDelegate, times(1)).createRpcClient(address);
  }

  @Test
  public void testHandlesImmediateConnectionError() {
    SocketAddress address = new InetSocketAddress("localhost", 8080);
    RpcClientFactory mockDelegate = mock(RpcClientFactory.class);
    RuntimeException connectionError = new RuntimeException("Connection failed");

    // First attempt fails
    when(mockDelegate.createRpcClient(address))
        .thenReturn(Mono.error(connectionError))
        .thenReturn(Mono.just(createMockRpcClient()));

    CachedRpcClientFactory factory = new CachedRpcClientFactory(mockDelegate);

    // Get a cached Mono that will error
    Mono<RpcClient> errorMono = factory.createRpcClient(address);

    // First subscription should fail
    StepVerifier.create(errorMono).expectError(RuntimeException.class).verify();

    // Second subscription to the same Mono should also fail (errors are cached)
    StepVerifier.create(errorMono).expectError(RuntimeException.class).verify();

    // But errors are NOT cached indefinitely - after first subscription,
    // the same Mono can be subscribed to again
    verify(mockDelegate, times(1)).createRpcClient(address);

    // Creating a NEW cached Mono should try again and succeed
    RpcClient client = factory.createRpcClient(address).block();
    assertNotNull(client);

    // Verify delegate was called twice total
    verify(mockDelegate, times(2)).createRpcClient(address);
  }

  @Test
  public void testAllowsLoadBalancingWithSeparateCachedMonos() {
    SocketAddress address = new InetSocketAddress("localhost", 8080);
    RpcClientFactory mockDelegate = mock(RpcClientFactory.class);

    RpcClient mockClient1 = createMockRpcClient();
    RpcClient mockClient2 = createMockRpcClient();
    RpcClient mockClient3 = createMockRpcClient();

    when(mockDelegate.createRpcClient(address))
        .thenReturn(Mono.just(mockClient1))
        .thenReturn(Mono.just(mockClient2))
        .thenReturn(Mono.just(mockClient3));

    CachedRpcClientFactory factory = new CachedRpcClientFactory(mockDelegate);

    // Simulate load balancing by calling createRpcClient() multiple times
    // Each call should create a separate cached Mono for connection pooling
    Mono<RpcClient> mono1 = factory.createRpcClient(address);
    Mono<RpcClient> mono2 = factory.createRpcClient(address);
    Mono<RpcClient> mono3 = factory.createRpcClient(address);

    // Subscribe to each Mono
    RpcClient client1 = mono1.block();
    RpcClient client2 = mono2.block();
    RpcClient client3 = mono3.block();

    // Should get different clients (connection pooling)
    assertNotNull(client1);
    assertNotNull(client2);
    assertNotNull(client3);
    assertNotEquals(client1, client2);
    assertNotEquals(client2, client3);
    assertNotEquals(client1, client3);

    // Verify delegate was called 3 times (once for each pool slot)
    verify(mockDelegate, times(3)).createRpcClient(address);
  }

  @Test
  public void testErrorThenSuccessfulRetry() {
    SocketAddress address = new InetSocketAddress("localhost", 8080);
    RpcClientFactory mockDelegate = mock(RpcClientFactory.class);
    RuntimeException error1 = new RuntimeException("Connection failed");
    RuntimeException error2 = new RuntimeException("Still failing");
    RpcClient mockClient = createMockRpcClient();

    when(mockDelegate.createRpcClient(address))
        .thenReturn(Mono.error(error1))
        .thenReturn(Mono.error(error2))
        .thenReturn(Mono.just(mockClient));

    CachedRpcClientFactory factory = new CachedRpcClientFactory(mockDelegate);

    // First two attempts create new Monos that fail
    StepVerifier.create(factory.createRpcClient(address))
        .expectError(RuntimeException.class)
        .verify();

    StepVerifier.create(factory.createRpcClient(address))
        .expectError(RuntimeException.class)
        .verify();

    // Third attempt creates a new Mono that succeeds
    Mono<RpcClient> successMono = factory.createRpcClient(address);
    RpcClient client = successMono.block();
    assertNotNull(client);
    assertEquals(mockClient, client);

    // Subscribing to the same successful Mono again uses the cache
    RpcClient client2 = successMono.block();
    assertEquals(client, client2);

    // Verify: 3 creation attempts (2 errors + 1 success), no 4th call since we reused successMono
    verify(mockDelegate, times(3)).createRpcClient(address);
  }

  // Static map to store close sinks for each mock client
  private final java.util.Map<RpcClient, Sinks.One<Void>> closeSinkMap = new java.util.HashMap<>();

  private RpcClient createMockRpcClient() {
    RpcClient mockClient = mock(RpcClient.class);
    Sinks.One<Void> closeSink = Sinks.one();
    closeSinkMap.put(mockClient, closeSink);
    when(mockClient.onClose()).thenReturn(closeSink.asMono());
    when(mockClient.isDisposed()).thenReturn(false); // Alive by default
    when(mockClient.toString()).thenReturn("MockRpcClient@" + System.identityHashCode(mockClient));
    return mockClient;
  }

  private Sinks.One<Void> getCloseSinkForClient(RpcClient mockClient) {
    return closeSinkMap.get(mockClient);
  }
}
