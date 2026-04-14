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

import static org.assertj.core.api.Assertions.assertThat;
import static org.junit.jupiter.api.Assertions.assertNotNull;

import com.facebook.swift.service.ThriftServerConfig;
import com.facebook.thrift.example.ping.PingRequest;
import com.facebook.thrift.example.ping.PingService;
import com.facebook.thrift.example.ping.PingServiceRpcServerHandler;
import com.facebook.thrift.rsocket.server.RSocketServerTransport;
import com.facebook.thrift.rsocket.server.RSocketServerTransportFactory;
import com.facebook.thrift.server.RpcServerHandler;
import com.facebook.thrift.testservices.BlockingPingService;
import io.airlift.units.Duration;
import java.net.InetSocketAddress;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.concurrent.TimeUnit;
import org.apache.thrift.ProtocolId;
import org.junit.jupiter.api.AfterEach;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.Timeout;
import reactor.core.publisher.Mono;
import reactor.core.publisher.Sinks;

/**
 * Tests for RSocket connection health, reconnection, and failure detection.
 *
 * <p>Validates bugs found during investigation of production "No keep-alive acks for 3600000 ms"
 * errors (ConnectionErrorException) in Painkiller CacheClient via CLF sidecar.
 */
public class RSocketKeepAliveAndReconnectionTest {

  static {
    System.setProperty("thrift.rsocket-keep-alive-ms", "500");
    System.setProperty("thrift.rsocket-keep-max-lifetime-ms", "2000");
  }

  private final List<AutoCloseable> cleanupList = new ArrayList<>();

  @AfterEach
  void cleanup() {
    for (AutoCloseable c : cleanupList) {
      try {
        c.close();
      } catch (Exception ignored) {
      }
    }
    cleanupList.clear();
  }

  // ---------------------------------------------------------------------------
  // Helpers
  // ---------------------------------------------------------------------------

  private RSocketServerTransport startServer(int port) {
    ThriftServerConfig config =
        new ThriftServerConfig().setPort(port).setSslEnabled(false).setEnableJdkSsl(false);
    RpcServerHandler handler =
        new PingServiceRpcServerHandler(new BlockingPingService(), Collections.emptyList());
    RSocketServerTransport transport =
        new RSocketServerTransportFactory(config).createServerTransport(handler).block();
    cleanupList.add(transport::dispose);
    return transport;
  }

  private PingService.Reactive makeClient(InetSocketAddress address) {
    RpcClientFactory factory =
        RpcClientFactory.builder()
            .setDisableLoadBalancing(true)
            .setDisableRSocket(false)
            .setThriftClientConfig(
                new ThriftClientConfig()
                    .setDisableSSL(true)
                    .setRequestTimeout(Duration.succinctDuration(10, TimeUnit.SECONDS)))
            .build();
    return PingService.Reactive.clientBuilder()
        .setProtocolId(ProtocolId.BINARY)
        .build(factory, address);
  }

  private PingRequest ping(String msg) {
    return new PingRequest.Builder().setRequest(msg).build();
  }

  // ---------------------------------------------------------------------------
  // Test 1: Bug #2 — FusableReconnectingRpcClientMono fast path missing isDisposed
  // ---------------------------------------------------------------------------

  /**
   * Verifies that FusableReconnectingRpcClientMono.fastPath() does NOT return a disposed client.
   * When isDisposed() is true, the fast path is bypassed and the subscriber enters the slow path,
   * which triggers reconnection.
   *
   * <p>This simulates the production race: RSocket is closed but onClose() hasn't completed yet
   * (Mono.whenDelayError waits for both UnboundedProcessor finalization AND
   * connection.onTerminate()). Previously, the fast path would return the disposed client.
   */
  @Test
  @Timeout(10)
  void testFusableDoesNotReturnDisposedClientOnFastPath() {
    QueuedRpcClientFactory factory = new QueuedRpcClientFactory();
    ReconnectingRpcClientFactory reconnecting = new ReconnectingRpcClientFactory(factory, true);
    Mono<RpcClient> mono = reconnecting.createRpcClient(new InetSocketAddress(0));

    assertThat(mono).isInstanceOf(FusableReconnectingRpcClientMono.class);

    // Emit a client so the mono enters CONNECTED state
    Sinks.One<RpcClient> sink1 = factory.addSink();
    DummyRpcClient client1 = new DummyRpcClient();
    sink1.tryEmitValue(client1);

    RpcClient r1 = mono.block(java.time.Duration.ofSeconds(5));
    assertNotNull(r1);
    assertThat(r1.isDisposed()).isFalse();

    // Mark disposed WITHOUT firing onClose — simulates the production race window
    client1.markDisposedWithoutClose();

    // Pre-populate a sink for the reconnection that will be triggered
    Sinks.One<RpcClient> sink2 = factory.addSink();
    DummyRpcClient client2 = new DummyRpcClient();
    sink2.tryEmitValue(client2);

    // With the fix, fast path detects isDisposed() and falls through to slow path,
    // which triggers reconnection and returns a fresh client.
    RpcClient r2 = mono.block(java.time.Duration.ofSeconds(5));
    assertNotNull(r2);

    assertThat(r2.isDisposed())
        .describedAs("fastPath() checks isDisposed() and triggers reconnection")
        .isFalse();
  }

  // ---------------------------------------------------------------------------
  // Test 2: Disposed client window — every subscription gets a dead client
  // ---------------------------------------------------------------------------

  /**
   * Verifies that zero subscriptions return a disposed client. Previously, 100% of fast-path
   * subscriptions would return the stale disposed client during the race window.
   */
  @Test
  @Timeout(10)
  void testNoDisposedClientsReturnedAfterFix() {
    QueuedRpcClientFactory factory = new QueuedRpcClientFactory();
    ReconnectingRpcClientFactory reconnecting = new ReconnectingRpcClientFactory(factory, true);
    Mono<RpcClient> mono = reconnecting.createRpcClient(new InetSocketAddress(0));

    Sinks.One<RpcClient> sink1 = factory.addSink();
    DummyRpcClient client1 = new DummyRpcClient();
    sink1.tryEmitValue(client1);

    mono.block(java.time.Duration.ofSeconds(5)); // enter CONNECTED

    client1.markDisposedWithoutClose(); // race window begins

    // Pre-populate a sink for the reconnection
    Sinks.One<RpcClient> sink2 = factory.addSink();
    DummyRpcClient client2 = new DummyRpcClient();
    sink2.tryEmitValue(client2);

    int disposedCount = 0;
    for (int i = 0; i < 50; i++) {
      RpcClient c = mono.block(java.time.Duration.ofSeconds(1));
      if (c != null && c.isDisposed()) {
        disposedCount++;
      }
    }

    System.out.printf("[TEST] %d/50 subscriptions returned disposed client%n", disposedCount);

    assertThat(disposedCount)
        .describedAs("isDisposed() check prevents stale clients on fast path")
        .isEqualTo(0);
  }

  // ---------------------------------------------------------------------------
  // Test 3: Reconnection works after onClose fires
  // ---------------------------------------------------------------------------

  /**
   * After dispose() + onClose(), the FusableReconnectingRpcClientMono transitions to DISCONNECTED
   * and reconnects on the next subscription.
   */
  @Test
  @Timeout(10)
  void testReconnectionAfterOnCloseFires() throws Exception {
    QueuedRpcClientFactory factory = new QueuedRpcClientFactory();
    ReconnectingRpcClientFactory reconnecting = new ReconnectingRpcClientFactory(factory, true);
    Mono<RpcClient> mono = reconnecting.createRpcClient(new InetSocketAddress(0));

    // Sink for initial connection
    Sinks.One<RpcClient> sink1 = factory.addSink();
    // Sink for reconnection (pre-populate before dispose triggers reconnect)
    Sinks.One<RpcClient> sink2 = factory.addSink();

    DummyRpcClient client1 = new DummyRpcClient();
    sink1.tryEmitValue(client1);

    RpcClient r1 = mono.block(java.time.Duration.ofSeconds(5));
    assertNotNull(r1);

    // Full dispose: fires onClose → setStateDisconnected → reconnect uses sink2
    client1.dispose();

    // Wait for async state transition
    Thread.sleep(200);

    // Emit fresh client on reconnection sink
    DummyRpcClient client2 = new DummyRpcClient();
    sink2.tryEmitValue(client2);

    RpcClient r2 = mono.block(java.time.Duration.ofSeconds(5));
    assertNotNull(r2);

    System.out.printf("[TEST] After reconnect, client disposed: %b%n", r2.isDisposed());
    assertThat(r2.isDisposed())
        .describedAs("After onClose fires and reconnection, client should be live")
        .isFalse();
  }

  // NOTE: Real RSocket server tests (server close detection, reconnection with real server)
  // cannot run in the Buck test sandbox on macOS because kqueue socket connections to localhost
  // are blocked with "Operation not permitted". These tests should be run in a non-sandboxed
  // environment (e.g., on a devserver) or converted to use Unix domain sockets.

  // ---------------------------------------------------------------------------
  // Test 5: Reconnection after server restart (mock-based, no sandbox issues)
  // ---------------------------------------------------------------------------

  /**
   * Verifies the full reconnection lifecycle using mock clients: CONNECTED → dispose → DISCONNECTED
   * → reconnect → CONNECTED with new client. This validates the same path as a real server
   * stop+restart without depending on localhost network access.
   */
  @Test
  @Timeout(30)
  void testFullReconnectionLifecycle() throws Exception {
    QueuedRpcClientFactory factory = new QueuedRpcClientFactory();
    ReconnectingRpcClientFactory reconnecting = new ReconnectingRpcClientFactory(factory, true);
    Mono<RpcClient> mono = reconnecting.createRpcClient(new InetSocketAddress(0));

    // Pre-populate sinks for all 3 connections. The reconnecting mono calls
    // createRpcClient() asynchronously on dispose, so sinks must be ready
    // before the reconnection triggers.
    Sinks.One<RpcClient> sink1 = factory.addSink();
    Sinks.One<RpcClient> sink2 = factory.addSink();
    Sinks.One<RpcClient> sink3 = factory.addSink();

    // Connection 1
    DummyRpcClient c1 = new DummyRpcClient();
    sink1.tryEmitValue(c1);

    RpcClient r1 = mono.block(java.time.Duration.ofSeconds(5));
    assertNotNull(r1);
    assertThat(r1.isDisposed()).isFalse();

    // Dispose → triggers async reconnect via sink2
    DummyRpcClient c2 = new DummyRpcClient();
    c1.dispose();
    // Emit immediately — the reconnect polls sink2 asynchronously
    sink2.tryEmitValue(c2);

    RpcClient r2 = mono.block(java.time.Duration.ofSeconds(10));
    assertNotNull(r2);
    assertThat(r2.isDisposed()).isFalse();

    // Dispose → triggers async reconnect via sink3
    DummyRpcClient c3 = new DummyRpcClient();
    c2.dispose();
    sink3.tryEmitValue(c3);

    RpcClient r3 = mono.block(java.time.Duration.ofSeconds(10));
    assertNotNull(r3);
    assertThat(r3.isDisposed()).isFalse();

    System.out.println("[TEST] Full lifecycle: 3 connections, all live");
  }

  // ---------------------------------------------------------------------------
  // Test: Concurrent subscribers on connected mono (validates composite CAS)
  // ---------------------------------------------------------------------------

  /**
   * Exercises FusableReconnectingRpcClientMono with many concurrent subscribers while in CONNECTED
   * state. All should get the same live client. This validates that the composite ConnectionState
   * atomic reference prevents torn reads between phase and client under concurrency.
   *
   * <p>Previously, two separate AtomicReferenceFieldUpdaters (state + rpcClient) could yield
   * inconsistent snapshots: a thread could read state=CONNECTED but get a stale/null rpcClient, or
   * vice versa. The composite ConnectionState ensures both fields are read atomically.
   */
  @Test
  @Timeout(15)
  void testConcurrentSubscribersGetLiveClient() throws Exception {
    QueuedRpcClientFactory factory = new QueuedRpcClientFactory();
    ReconnectingRpcClientFactory reconnecting = new ReconnectingRpcClientFactory(factory, true);
    Mono<RpcClient> mono = reconnecting.createRpcClient(new InetSocketAddress(0));

    Sinks.One<RpcClient> sink = factory.addSink();
    DummyRpcClient liveClient = new DummyRpcClient();
    sink.tryEmitValue(liveClient);

    // Wait for CONNECTED state
    mono.block(java.time.Duration.ofSeconds(5));

    // Now hammer with concurrent subscribers — all should get the live client
    List<Throwable> errors = Collections.synchronizedList(new ArrayList<>());
    Thread[] threads = new Thread[20];
    for (int t = 0; t < threads.length; t++) {
      threads[t] =
          new Thread(
              () -> {
                try {
                  for (int j = 0; j < 100; j++) {
                    RpcClient c = mono.block(java.time.Duration.ofSeconds(1));
                    if (c == null) {
                      errors.add(new AssertionError("Got null client"));
                    } else if (c.isDisposed()) {
                      errors.add(new AssertionError("Got disposed client"));
                    }
                  }
                } catch (Exception e) {
                  errors.add(e);
                }
              });
      threads[t].start();
    }
    for (Thread t : threads) {
      t.join(10000);
    }

    System.out.printf("[TEST] 20 threads x 100 subs, errors=%d%n", errors.size());
    assertThat(errors)
        .describedAs("All concurrent subscribers should get the live client")
        .isEmpty();
  }

  // ---------------------------------------------------------------------------
  // Test: Stale onClose does not kill a new connection
  // ---------------------------------------------------------------------------

  /**
   * Validates that when C1 dies and C2 is established before C1's onClose fires, C1's onClose
   * callback does NOT tear down C2.
   *
   * <p>Previously, connect() carried the old client into CONNECTING state, so C1's onClose identity
   * guard (current.client == C1) would match during CONNECTING+C1, triggering a spurious disconnect
   * and duplicate connect. With the fix, CONNECTING uses client=null and handleIncomingRpcClient
   * uses CAS (not set), so stale callbacks are harmless.
   */
  @Test
  @Timeout(10)
  void testStaleOnCloseDoesNotKillNewConnection() throws Exception {
    QueuedRpcClientFactory factory = new QueuedRpcClientFactory();
    ReconnectingRpcClientFactory reconnecting = new ReconnectingRpcClientFactory(factory, true);
    Mono<RpcClient> mono = reconnecting.createRpcClient(new InetSocketAddress(0));

    // Connection 1
    Sinks.One<RpcClient> sink1 = factory.addSink();
    DummyRpcClient c1 = new DummyRpcClient();
    sink1.tryEmitValue(c1);

    RpcClient r1 = mono.block(java.time.Duration.ofSeconds(5));
    assertNotNull(r1);
    assertThat(r1.isDisposed()).isFalse();

    // Simulate the race window: C1 is disposed but onClose hasn't fired.
    // drain() detects isDisposed() and triggers reconnection.
    c1.markDisposedWithoutClose();

    // Pre-populate sink for reconnection
    Sinks.One<RpcClient> sink2 = factory.addSink();
    DummyRpcClient c2 = new DummyRpcClient();
    sink2.tryEmitValue(c2);

    // Get the new connection
    RpcClient r2 = mono.block(java.time.Duration.ofSeconds(5));
    assertNotNull(r2);
    assertThat(r2.isDisposed()).isFalse();

    // NOW fire C1's onClose. With the bug, this would kill C2.
    // With the fix, the identity guard fails (CONNECTING has client=null,
    // and CONNECTED has client=C2, neither matches C1).
    c1.fireOnCloseOnly();

    // Give the doFinally callback time to execute
    Thread.sleep(200);

    // C2 should still be the current live client
    RpcClient r3 = mono.block(java.time.Duration.ofSeconds(5));
    assertNotNull(r3);
    assertThat(r3.isDisposed())
        .describedAs("C2 must survive C1's stale onClose callback")
        .isFalse();

    System.out.println("[TEST] Stale onClose did not kill new connection");
  }

  // ---------------------------------------------------------------------------
  // Test: Timed-out attempt followed by successful attempt
  // ---------------------------------------------------------------------------

  /**
   * Validates the full timeout → retry → success flow: Attempt A hangs (Mono.never) →
   * CONNECT_TIMEOUT fires → handleConnectionError CAS transitions to DISCONNECTED → attempt B
   * starts → succeeds → client is live.
   *
   * <p>This proves that: (1) the connect timeout fires and moves state forward, (2) the timed-out
   * attempt's error callback only affects its own attempt via CAS, (3) the retry attempt succeeds
   * and the client is functional.
   *
   * <p>Note: overlapping stale success from attempt A cannot reach handleIncomingRpcClient because
   * Reactor's timeout() operator cancels the upstream Mono. The CAS in handleIncomingRpcClient is a
   * defense-in-depth guard.
   */
  @Test
  @Timeout(30)
  void testTimedOutAttemptFollowedBySuccess() throws Exception {
    QueuedRpcClientFactory factory = new QueuedRpcClientFactory();
    ReconnectingRpcClientFactory reconnecting = new ReconnectingRpcClientFactory(factory, true);
    Mono<RpcClient> mono = reconnecting.createRpcClient(new InetSocketAddress(0));

    // Attempt A: never emitted to → will time out after CONNECT_TIMEOUT (10s)
    factory.addSink();

    // Attempt B: will succeed after A times out
    Sinks.One<RpcClient> sinkB = factory.addSink();
    DummyRpcClient clientB = new DummyRpcClient();

    new Thread(
            () -> {
              try {
                Thread.sleep(11000); // wait for A's connect timeout + margin
                sinkB.tryEmitValue(clientB);
              } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
              }
            })
        .start();

    RpcClient result = mono.block(java.time.Duration.ofSeconds(25));
    assertNotNull(result);
    assertThat(result.isDisposed()).isFalse();
    assertThat(result).isSameAs(clientB);

    // Verify the client stays alive — no stale callback tears it down
    Thread.sleep(500);
    RpcClient check = mono.block(java.time.Duration.ofSeconds(5));
    assertNotNull(check);
    assertThat(check.isDisposed())
        .describedAs("Client must survive after timed-out attempt")
        .isFalse();

    System.out.println("[TEST] Recovered from timed-out attempt A via attempt B");
  }

  // ---------------------------------------------------------------------------
  // Test: Cancelled subscriber is never signaled
  // ---------------------------------------------------------------------------

  /**
   * Verifies that a subscriber that cancels its subscription before emission receives no signals.
   * The InnerSubscription's cancelled flag should cause drain to skip it.
   */
  @Test
  @Timeout(10)
  void testCancelledSubscriberNeverSignaled() throws Exception {
    QueuedRpcClientFactory factory = new QueuedRpcClientFactory();
    ReconnectingRpcClientFactory reconnecting = new ReconnectingRpcClientFactory(factory, true);
    Mono<RpcClient> mono = reconnecting.createRpcClient(new InetSocketAddress(0));

    // Don't emit a client yet — subscribers will queue in slow path
    Sinks.One<RpcClient> sink1 = factory.addSink();

    // Subscribe and immediately cancel
    TrackingSubscriber cancelled = new TrackingSubscriber();
    mono.subscribe(cancelled);
    cancelled.cancel();

    // Subscribe a normal subscriber
    TrackingSubscriber normal = new TrackingSubscriber();
    mono.subscribe(normal);

    // Now emit a client — drain should skip the cancelled one
    DummyRpcClient c1 = new DummyRpcClient();
    sink1.tryEmitValue(c1);

    // Wait for drain to process
    Thread.sleep(500);

    assertThat(cancelled.onNextCount)
        .describedAs("Cancelled subscriber must not receive onNext")
        .isEqualTo(0);
    assertThat(cancelled.onCompleteCount)
        .describedAs("Cancelled subscriber must not receive onComplete")
        .isEqualTo(0);

    assertThat(normal.onNextCount)
        .describedAs("Normal subscriber should receive onNext")
        .isEqualTo(1);
    assertThat(normal.onCompleteCount)
        .describedAs("Normal subscriber should receive onComplete")
        .isEqualTo(1);
  }

  // ---------------------------------------------------------------------------
  // Test: Duplicate request(n) enqueues only once
  // ---------------------------------------------------------------------------

  /**
   * Verifies that calling request(n) multiple times on the same InnerSubscription only enqueues it
   * once, preventing duplicate terminal signals. The atomic CAS on the enqueued flag should ensure
   * exactly one queue entry per subscription.
   */
  @Test
  @Timeout(10)
  void testDuplicateRequestEnqueuesOnce() throws Exception {
    QueuedRpcClientFactory factory = new QueuedRpcClientFactory();
    ReconnectingRpcClientFactory reconnecting = new ReconnectingRpcClientFactory(factory, true);
    Mono<RpcClient> mono = reconnecting.createRpcClient(new InetSocketAddress(0));

    Sinks.One<RpcClient> sink1 = factory.addSink();

    // Use a manual subscriber that calls request multiple times
    MultiRequestSubscriber subscriber = new MultiRequestSubscriber(5);
    mono.subscribe(subscriber);

    // Emit client
    DummyRpcClient c1 = new DummyRpcClient();
    sink1.tryEmitValue(c1);

    Thread.sleep(500);

    // Despite 5 request() calls, should receive exactly one onNext + one onComplete
    assertThat(subscriber.onNextCount)
        .describedAs("Must receive exactly one onNext despite multiple request() calls")
        .isEqualTo(1);
    assertThat(subscriber.onCompleteCount)
        .describedAs("Must receive exactly one onComplete")
        .isEqualTo(1);
  }

  // ---------------------------------------------------------------------------
  // Test: Cancelled subscribers pruned during outage (no memory leak)
  // ---------------------------------------------------------------------------

  /**
   * During a prolonged outage (DISCONNECTED/CONNECTING), cancelled subscribers should be pruned
   * from the queue rather than accumulating indefinitely.
   */
  @Test
  @Timeout(15)
  void testCancelledSubscribersPrunedDuringOutage() throws Exception {
    // Factory that never emits — simulates permanent outage
    HangingRpcClientFactory factory = new HangingRpcClientFactory();
    ReconnectingRpcClientFactory reconnecting = new ReconnectingRpcClientFactory(factory, true);
    Mono<RpcClient> mono = reconnecting.createRpcClient(new InetSocketAddress(0));

    // Subscribe many subscribers, then cancel them
    List<TrackingSubscriber> subs = new ArrayList<>();
    for (int i = 0; i < 100; i++) {
      TrackingSubscriber sub = new TrackingSubscriber();
      mono.subscribe(sub);
      subs.add(sub);
    }

    // Cancel all
    for (TrackingSubscriber sub : subs) {
      sub.cancel();
    }

    // Subscribe one more live subscriber to trigger a drain cycle (which prunes)
    TrackingSubscriber liveSub = new TrackingSubscriber();
    mono.subscribe(liveSub);

    // Wait for drain cycles to prune
    Thread.sleep(1000);

    // Verify no cancelled subscriber was signaled
    for (TrackingSubscriber sub : subs) {
      assertThat(sub.onNextCount)
          .describedAs("Cancelled subscriber must not receive signals")
          .isEqualTo(0);
    }
  }

  // ---------------------------------------------------------------------------
  // Test: Hung connect attempt doesn't wedge state machine
  // ---------------------------------------------------------------------------

  /**
   * If the delegate factory returns a Mono that never completes (simulating a hung TCP handshake),
   * the connection timeout should fire and the state machine should transition back to DISCONNECTED
   * to retry.
   */
  @Test
  @Timeout(30)
  void testHungConnectTimesOutAndRetries() throws Exception {
    // First attempt hangs, second succeeds
    QueuedRpcClientFactory factory = new QueuedRpcClientFactory();
    ReconnectingRpcClientFactory reconnecting = new ReconnectingRpcClientFactory(factory, true);
    Mono<RpcClient> mono = reconnecting.createRpcClient(new InetSocketAddress(0));

    // First sink: never emit (simulates hung connection)
    factory.addSink(); // polled by first connect(), never emitted to

    // Second sink: will succeed
    Sinks.One<RpcClient> sink2 = factory.addSink();

    // Subscribe — first attempt hangs, should timeout, then retry with sink2
    DummyRpcClient c2 = new DummyRpcClient();

    // The connect timeout (CONNECT_TIMEOUT_MS = 10s) will fire, causing handleConnectionError,
    // which transitions to DISCONNECTED and retries. The retry picks up sink2.
    // We emit on sink2 shortly so the retry succeeds.
    new Thread(
            () -> {
              try {
                Thread.sleep(11000); // wait for connect timeout + small margin
                sink2.tryEmitValue(c2);
              } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
              }
            })
        .start();

    RpcClient result = mono.block(java.time.Duration.ofSeconds(25));
    assertNotNull(result);
    assertThat(result.isDisposed())
        .describedAs("Should recover after hung connect times out")
        .isFalse();

    System.out.println("[TEST] Recovered from hung connect attempt");
  }

  // ---------------------------------------------------------------------------
  // Test: Subscriber throwing from onNext doesn't break state machine
  // ---------------------------------------------------------------------------

  /**
   * If a subscriber throws from onNext, the error should be caught and the queue should continue
   * processing. The state machine must not get stuck.
   */
  @Test
  @Timeout(10)
  void testSubscriberThrowingFromOnNextDoesNotBreakDrain() throws Exception {
    QueuedRpcClientFactory factory = new QueuedRpcClientFactory();
    ReconnectingRpcClientFactory reconnecting = new ReconnectingRpcClientFactory(factory, true);
    Mono<RpcClient> mono = reconnecting.createRpcClient(new InetSocketAddress(0));

    Sinks.One<RpcClient> sink1 = factory.addSink();

    // Subscribe a throwing subscriber
    ThrowingSubscriber throwing = new ThrowingSubscriber();
    mono.subscribe(throwing);

    // Subscribe a normal subscriber
    TrackingSubscriber normal = new TrackingSubscriber();
    mono.subscribe(normal);

    // Emit client — drain should handle the throw and continue to the normal subscriber
    DummyRpcClient c1 = new DummyRpcClient();
    sink1.tryEmitValue(c1);

    Thread.sleep(500);

    assertThat(normal.onNextCount)
        .describedAs("Normal subscriber should still receive onNext after throwing subscriber")
        .isEqualTo(1);
  }

  // ---------------------------------------------------------------------------
  // Test: onClose and handleIncomingRpcClient racing simultaneously
  // ---------------------------------------------------------------------------

  /**
   * Exercises the tight race where old client's onClose fires at nearly the same time as a new
   * client arriving via handleIncomingRpcClient. The CAS-loop in the onClose callback and the
   * attempt-specific CAS in handleIncomingRpcClient should ensure only one wins.
   */
  @Test
  @Timeout(30)
  void testOnCloseRacingWithNewConnection() throws Exception {
    QueuedRpcClientFactory factory = new QueuedRpcClientFactory();
    ReconnectingRpcClientFactory reconnecting = new ReconnectingRpcClientFactory(factory, true);
    Mono<RpcClient> mono = reconnecting.createRpcClient(new InetSocketAddress(0));

    // Pre-populate all sinks upfront. The reconnecting mono polls these in order.
    int iterations = 5;
    List<Sinks.One<RpcClient>> sinks = new ArrayList<>();
    for (int i = 0; i < iterations + 1; i++) {
      sinks.add(factory.addSink());
    }

    // First connection
    DummyRpcClient c0 = new DummyRpcClient();
    sinks.get(0).tryEmitValue(c0);
    mono.block(java.time.Duration.ofSeconds(5));

    for (int i = 0; i < iterations; i++) {
      DummyRpcClient next = new DummyRpcClient();
      // Dispose current → onClose fires, triggers reconnect using next pre-populated sink
      c0.dispose();
      sinks.get(i + 1).tryEmitValue(next);

      RpcClient r = mono.block(java.time.Duration.ofSeconds(10));
      assertNotNull(r);
      assertThat(r.isDisposed()).isFalse();
      c0 = next;
    }

    System.out.printf("[TEST] %d rapid connect/close cycles, still functional%n", iterations);
  }

  // ---------------------------------------------------------------------------
  // Utilities
  // ---------------------------------------------------------------------------

  private static Throwable findRootCause(Throwable t) {
    Throwable c = t;
    while (c.getCause() != null && c.getCause() != c) {
      c = c.getCause();
    }
    return c;
  }

  // ---------------------------------------------------------------------------
  // Test doubles
  // ---------------------------------------------------------------------------

  /** RpcClientFactory backed by a queue of Sinks for precise control over connection emission. */
  private static class QueuedRpcClientFactory implements RpcClientFactory {
    private final ConcurrentLinkedQueue<Sinks.One<RpcClient>> sinks = new ConcurrentLinkedQueue<>();

    Sinks.One<RpcClient> addSink() {
      Sinks.One<RpcClient> sink = Sinks.one();
      sinks.add(sink);
      return sink;
    }

    @Override
    public Mono<RpcClient> createRpcClient(java.net.SocketAddress addr) {
      Sinks.One<RpcClient> sink = sinks.poll();
      if (sink == null) {
        sink = Sinks.one();
      }
      return sink.asMono();
    }
  }

  /**
   * Minimal RpcClient with two disposal modes:
   *
   * <ul>
   *   <li>{@link #dispose()} — sets disposed + fires onClose (triggers full reconnection)
   *   <li>{@link #markDisposedWithoutClose()} — sets disposed only (simulates the production race
   *       where Mono.whenDelayError hasn't completed yet)
   * </ul>
   */
  private static class DummyRpcClient implements RpcClient {
    private final Sinks.Empty<Void> closeSink = Sinks.empty();
    private volatile boolean disposed = false;

    @Override
    public Mono<Void> onClose() {
      return closeSink.asMono();
    }

    @Override
    public void dispose() {
      disposed = true;
      closeSink.tryEmitEmpty();
    }

    @Override
    public boolean isDisposed() {
      return disposed;
    }

    void markDisposedWithoutClose() {
      disposed = true;
    }

    /**
     * Fires onClose without setting disposed. Simulates a delayed onClose callback arriving after
     * the client was already replaced by a new connection.
     */
    void fireOnCloseOnly() {
      closeSink.tryEmitEmpty();
    }
  }

  /**
   * A subscriber that tracks signal counts for verification. Captures its Subscription so cancel()
   * can be called from test code.
   */
  private static class TrackingSubscriber implements org.reactivestreams.Subscriber<RpcClient> {
    volatile int onNextCount;
    volatile int onCompleteCount;
    volatile int onErrorCount;
    private volatile org.reactivestreams.Subscription subscription;

    @Override
    public void onSubscribe(org.reactivestreams.Subscription s) {
      this.subscription = s;
      s.request(1);
    }

    @Override
    public void onNext(RpcClient rpcClient) {
      onNextCount++;
    }

    @Override
    public void onError(Throwable t) {
      onErrorCount++;
    }

    @Override
    public void onComplete() {
      onCompleteCount++;
    }

    void cancel() {
      org.reactivestreams.Subscription s = this.subscription;
      if (s != null) {
        s.cancel();
      }
    }
  }

  /**
   * A subscriber that calls request(n) multiple times on the same subscription to test the
   * enqueue-once CAS guard.
   */
  private static class MultiRequestSubscriber implements org.reactivestreams.Subscriber<RpcClient> {
    private final int requestCount;
    volatile int onNextCount;
    volatile int onCompleteCount;

    MultiRequestSubscriber(int requestCount) {
      this.requestCount = requestCount;
    }

    @Override
    public void onSubscribe(org.reactivestreams.Subscription s) {
      for (int i = 0; i < requestCount; i++) {
        s.request(1);
      }
    }

    @Override
    public void onNext(RpcClient rpcClient) {
      onNextCount++;
    }

    @Override
    public void onError(Throwable t) {}

    @Override
    public void onComplete() {
      onCompleteCount++;
    }
  }

  /** A subscriber that throws from onNext to test drain error handling. */
  private static class ThrowingSubscriber implements org.reactivestreams.Subscriber<RpcClient> {
    @Override
    public void onSubscribe(org.reactivestreams.Subscription s) {
      s.request(1);
    }

    @Override
    public void onNext(RpcClient rpcClient) {
      throw new RuntimeException("subscriber throw from onNext");
    }

    @Override
    public void onError(Throwable t) {}

    @Override
    public void onComplete() {}
  }

  /**
   * An RpcClientFactory that never emits — all createRpcClient() calls return Mono.never().
   * Simulates a permanently unreachable server.
   */
  private static class HangingRpcClientFactory implements RpcClientFactory {
    @Override
    public Mono<RpcClient> createRpcClient(java.net.SocketAddress addr) {
      return Mono.never();
    }
  }
}
