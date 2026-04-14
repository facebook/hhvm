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
  }
}
