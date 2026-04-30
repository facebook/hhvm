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

package com.facebook.thrift.transport.unified;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertNotEquals;
import static org.junit.jupiter.api.Assertions.assertNotNull;
import static org.junit.jupiter.api.Assertions.assertNotSame;
import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.junit.jupiter.api.Assertions.assertTrue;
import static org.junit.jupiter.api.Assertions.fail;

import com.facebook.nifty.core.RequestContext;
import com.facebook.nifty.core.RequestContexts;
import com.facebook.swift.service.ThriftServerConfig;
import com.facebook.thrift.client.RpcClientFactory;
import com.facebook.thrift.client.ThriftClientConfig;
import com.facebook.thrift.rsocket.server.TestServiceHandler;
import com.facebook.thrift.server.RpcServerHandler;
import com.facebook.thrift.test.rocket.TestException;
import com.facebook.thrift.test.rocket.TestRequest;
import com.facebook.thrift.test.rocket.TestResponse;
import com.facebook.thrift.test.rocket.TestService;
import com.facebook.thrift.util.RpcServerUtils;
import com.facebook.thrift.util.SPINiftyMetrics;
import io.airlift.units.Duration;
import java.net.InetSocketAddress;
import java.net.SocketAddress;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicReference;
import org.apache.thrift.ProtocolId;
import org.apache.thrift.TApplicationException;
import org.junit.jupiter.api.AfterAll;
import org.junit.jupiter.api.AfterEach;
import org.junit.jupiter.api.BeforeAll;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;
import reactor.core.publisher.Flux;
import reactor.core.publisher.Mono;
import reactor.test.StepVerifier;

/**
 * Comprehensive tests for UnifiedServerTransport.
 *
 * <p>These tests validate: 1. Transport creation and lifecycle management 2. Metrics tracking 3.
 * Connection limits 4. Both RSocket and Header protocol paths (via ALPN) 5. Request/response and
 * streaming operations
 *
 * <p>The UnifiedServerTransport uses SSL with ALPN to negotiate between "rs" (RSocket) and "header"
 * protocols, then configures the appropriate pipeline.
 */
public class UnifiedServerTransportTest {

  private UnifiedServerTransport transport;
  private SocketAddress serverAddress;
  private RpcServerHandler rpcServerHandler;
  private TestServiceHandler testServiceHandler;
  private ThriftServerConfig serverConfig;
  private TestService.Reactive client;

  @BeforeAll
  public static void setUpClass() throws Exception {
    // Initialize self-signed certificates for testing
    TestCertificateUtil.initialize();
  }

  @AfterAll
  public static void tearDownClass() {
    // Clean up certificate files
    TestCertificateUtil.cleanup();
  }

  @BeforeEach
  public void setUp() throws Exception {
    testServiceHandler = new TestServiceHandler();
    rpcServerHandler = TestService.Reactive.serverHandlerBuilder(testServiceHandler).build();

    // Create server config with SSL enabled and self-signed certificates
    serverConfig =
        new ThriftServerConfig()
            .setSslEnabled(true)
            .setEnableJdkSsl(false)
            .setKeyFile(TestCertificateUtil.getKeyFilePath())
            .setCertFile(TestCertificateUtil.getCertFilePath())
            .setCAFile(TestCertificateUtil.getCAFilePath())
            .setEnableAlpn(true)
            .setTaskExpirationTimeout(Duration.valueOf("60s"))
            .setConnectionLimit(10);

    serverAddress = new InetSocketAddress("localhost", RpcServerUtils.findFreePort());
  }

  @AfterEach
  public void tearDown() {
    if (transport != null) {
      transport.dispose();
      transport = null;
    }
  }

  /**
   * Test that UnifiedServerTransport can be created successfully and starts listening on the
   * specified address.
   */
  @Test
  public void testTransportCreation() {
    transport =
        UnifiedServerTransport.createNewInstance(
                serverAddress, rpcServerHandler, serverConfig, new SPINiftyMetrics())
            .block();

    assertNotNull(transport, "Transport should be created");
    assertNotNull(transport.getAddress(), "Transport should have an address");
    assertNotNull(transport.getNiftyMetrics(), "Transport should have metrics");
  }

  /** Test that createNewInstance returns a Mono that completes successfully. */
  @Test
  public void testTransportCreationAsMono() {
    StepVerifier.create(
            UnifiedServerTransport.createNewInstance(
                serverAddress, rpcServerHandler, serverConfig, new SPINiftyMetrics()))
        .assertNext(
            t -> {
              assertNotNull(t, "Transport should not be null");
              assertNotNull(t.getAddress(), "Transport should have an address");
              assertNotNull(t.getNiftyMetrics(), "Transport should have metrics");
              transport = t; // Store for cleanup
            })
        .verifyComplete();
  }

  /**
   * Test RSocket connection path via ALPN negotiation. This validates that the transport accepts
   * connections when ALPN negotiates "rs" protocol.
   */
  @Test
  public void testRSocketConnectionPath() throws Exception {
    transport =
        UnifiedServerTransport.createNewInstance(
                serverAddress, rpcServerHandler, serverConfig, new SPINiftyMetrics())
            .block();
    assertNotNull(transport);

    // Create RSocket client - uses "rs" protocol via ALPN
    RpcClientFactory factory = createRSocketClientFactory();
    client =
        TestService.Reactive.clientBuilder()
            .setProtocolId(ProtocolId.COMPACT)
            .build(factory, serverAddress);

    // Make a request/response call to verify RSocket path works
    TestRequest request = new TestRequest.Builder().setIntField(42).setStrField("rsocket").build();

    Mono<TestResponse> response = client.requestResponse(request);

    StepVerifier.create(response)
        .assertNext(
            resp -> {
              assertEquals(42, resp.getIntField());
              assertEquals("rsocket", resp.getStrField());
            })
        .verifyComplete();
  }

  /**
   * Test Header protocol connection path via ALPN negotiation. This validates that the transport
   * accepts connections when ALPN negotiates "header" protocol.
   */
  @Test
  public void testHeaderConnectionPath() throws Exception {
    transport =
        UnifiedServerTransport.createNewInstance(
                serverAddress, rpcServerHandler, serverConfig, new SPINiftyMetrics())
            .block();
    assertNotNull(transport);

    // Create Header client - uses "header" protocol via ALPN
    RpcClientFactory factory = createHeaderClientFactory();
    client =
        TestService.Reactive.clientBuilder()
            .setProtocolId(ProtocolId.COMPACT)
            .build(factory, serverAddress);

    // Make a request/response call to verify Header path works
    TestRequest request = new TestRequest.Builder().setIntField(100).setStrField("header").build();

    Mono<TestResponse> response = client.requestResponse(request);

    StepVerifier.create(response)
        .assertNext(
            resp -> {
              assertEquals(100, resp.getIntField());
              assertEquals("header", resp.getStrField());
            })
        .verifyComplete();
  }

  /**
   * Test RSocket streaming capability. This validates that streaming operations work correctly over
   * the RSocket path.
   */
  @Test
  public void testRSocketStreamingPath() throws Exception {
    transport =
        UnifiedServerTransport.createNewInstance(
                serverAddress, rpcServerHandler, serverConfig, new SPINiftyMetrics())
            .block();
    assertNotNull(transport);

    RpcClientFactory factory = createRSocketClientFactory();
    client =
        TestService.Reactive.clientBuilder()
            .setProtocolId(ProtocolId.COMPACT)
            .build(factory, serverAddress);

    TestRequest request = new TestRequest.Builder().setIntField(0).setStrField("stream").build();

    Flux<TestResponse> response = client.streamResponse(request);

    StepVerifier.create(response).expectNextCount(10).verifyComplete();
  }

  /**
   * Test that Header protocol does not support streaming. This validates that attempting to use
   * streaming over the Header path via ALPN throws an exception, since Header protocol only
   * supports request/response patterns.
   */
  @Test
  public void testHeaderStreamingNotSupported() throws Exception {
    transport =
        UnifiedServerTransport.createNewInstance(
                serverAddress, rpcServerHandler, serverConfig, new SPINiftyMetrics())
            .block();
    assertNotNull(transport);

    // Create Header client - uses "header" protocol via ALPN
    RpcClientFactory factory = createHeaderClientFactory();
    client =
        TestService.Reactive.clientBuilder()
            .setProtocolId(ProtocolId.COMPACT)
            .build(factory, serverAddress);

    TestRequest request =
        new TestRequest.Builder().setIntField(0).setStrField("header-stream").build();

    Flux<TestResponse> response = client.streamResponse(request);

    // Header protocol does not support streaming - expect an error
    StepVerifier.create(response).expectError().verify();
  }

  /** Test that the transport properly initializes and tracks metrics. */
  @Test
  public void testMetricsInitialization() {
    transport =
        UnifiedServerTransport.createNewInstance(
                serverAddress, rpcServerHandler, serverConfig, new SPINiftyMetrics())
            .block();
    assertNotNull(transport);

    SPINiftyMetrics metrics = transport.getNiftyMetrics();
    assertNotNull(metrics, "Metrics should be available");

    // Verify metrics are initialized
    assertTrue(metrics.getChannelCount() >= 0, "Channel count should be non-negative");
    assertTrue(
        metrics.getRejectedConnections() >= 0, "Rejected connections should be non-negative");
  }

  /**
   * Test that metrics accurately track connection lifecycle. This validates that channel count
   * increases when connections are established and decreases when they are closed.
   */
  @Test
  public void testMetricsTrackConnectionLifecycle() throws Exception {
    transport =
        UnifiedServerTransport.createNewInstance(
                serverAddress, rpcServerHandler, serverConfig, new SPINiftyMetrics())
            .block();
    assertNotNull(transport);

    SPINiftyMetrics metrics = transport.getNiftyMetrics();
    assertNotNull(metrics, "Metrics should be available");

    // Get initial channel count (should be 0 or very low)
    int initialChannelCount = metrics.getChannelCount();
    assertEquals(
        0,
        initialChannelCount,
        "Initial channel count should be non-negative, got: " + initialChannelCount);

    RpcClientFactory factory = createRSocketClientFactory();
    TestService.Reactive client1 = null;
    TestService.Reactive client2 = null;

    try {
      // Create first client connection
      client1 =
          TestService.Reactive.clientBuilder()
              .setProtocolId(ProtocolId.COMPACT)
              .build(factory, serverAddress);

      // Make a request to establish the connection
      TestRequest request1 =
          new TestRequest.Builder().setIntField(1).setStrField("metrics-test-1").build();

      StepVerifier.create(client1.requestResponse(request1))
          .assertNext(
              resp -> {
                assertEquals(1, resp.getIntField());
                assertEquals("metrics-test-1", resp.getStrField());
              })
          .verifyComplete();

      // Verify channel count increased
      int countAfterFirstConnection = metrics.getChannelCount();
      assertTrue(
          countAfterFirstConnection > initialChannelCount,
          "Channel count should increase after first connection. Initial: "
              + initialChannelCount
              + ", Current: "
              + countAfterFirstConnection);

      // Create second client connection
      client2 =
          TestService.Reactive.clientBuilder()
              .setProtocolId(ProtocolId.COMPACT)
              .build(factory, serverAddress);

      // Make a request to establish the second connection
      TestRequest request2 =
          new TestRequest.Builder().setIntField(2).setStrField("metrics-test-2").build();

      StepVerifier.create(client2.requestResponse(request2))
          .assertNext(
              resp -> {
                assertEquals(2, resp.getIntField());
                assertEquals("metrics-test-2", resp.getStrField());
              })
          .verifyComplete();

      // Verify channel count increased again
      int countAfterSecondConnection = metrics.getChannelCount();
      assertTrue(
          countAfterSecondConnection > countAfterFirstConnection,
          "Channel count should increase after second connection. After first: "
              + countAfterFirstConnection
              + ", After second: "
              + countAfterSecondConnection);

      // Close first client
      client1.dispose();
      client1 = null;

      // Give it a moment for the connection to fully close
      Thread.sleep(1000);

      // Verify channel count decreased
      int countAfterFirstClose = metrics.getChannelCount();
      assertTrue(
          countAfterFirstClose < countAfterSecondConnection,
          "Channel count should decrease after closing first connection. Before close: "
              + countAfterSecondConnection
              + ", After close: "
              + countAfterFirstClose);

      // Close second client
      client2.dispose();
      client2 = null;

      // Give it a moment for the connection to fully close
      Thread.sleep(100);

      // Verify channel count decreased again
      int finalChannelCount = metrics.getChannelCount();
      assertTrue(
          finalChannelCount < countAfterFirstClose,
          "Channel count should decrease after closing second connection. Before close: "
              + countAfterFirstClose
              + ", After close: "
              + finalChannelCount);

      // Should be back to initial count or very close
      assertTrue(
          finalChannelCount <= initialChannelCount + 1,
          "Final channel count should be close to initial count. Initial: "
              + initialChannelCount
              + ", Final: "
              + finalChannelCount);

    } finally {
      // Clean up any remaining connections
      if (client1 != null) {
        try {
          client1.dispose();
        } catch (Exception e) {
          // Ignore cleanup errors
        }
      }
      if (client2 != null) {
        try {
          client2.dispose();
        } catch (Exception e) {
          // Ignore cleanup errors
        }
      }
    }
  }

  /**
   * Test that exceptions thrown by the service handler are properly propagated to clients. This
   * validates error handling for both declared exceptions (TestException) and undeclared exceptions
   * (IllegalArgumentException) over RSocket protocol.
   */
  @Test
  public void testErrorPropagationRSocket() throws Exception {
    transport =
        UnifiedServerTransport.createNewInstance(
                serverAddress, rpcServerHandler, serverConfig, new SPINiftyMetrics())
            .block();
    assertNotNull(transport);

    RpcClientFactory factory = createRSocketClientFactory();
    client =
        TestService.Reactive.clientBuilder()
            .setProtocolId(ProtocolId.COMPACT)
            .build(factory, serverAddress);

    TestRequest request = new TestRequest.Builder().setIntField(1).setStrField("error").build();

    // Test declared exception propagation (TestException)
    Flux<TestResponse> declaredExceptionResponse = client.streamDeclaredException(request);
    StepVerifier.create(declaredExceptionResponse).expectError(TestException.class).verify();

    // Test declared exception propagation with direct throw
    Flux<TestResponse> declaredExceptionResponse2 = client.streamDeclaredException2(request);
    StepVerifier.create(declaredExceptionResponse2)
        .expectError(TApplicationException.class)
        .verify();

    // Test undeclared exception propagation (IllegalArgumentException wrapped in
    // TApplicationException)
    Flux<TestResponse> undeclaredExceptionResponse = client.streamUndeclaredException(request);
    StepVerifier.create(undeclaredExceptionResponse)
        .expectErrorMatches(
            t ->
                t instanceof TApplicationException
                    && t.getMessage() != null
                    && t.getMessage().contains("exc"))
        .verify();

    // Test undeclared exception propagation with direct throw
    Flux<TestResponse> undeclaredExceptionResponse2 = client.streamUndeclaredException2(request);
    StepVerifier.create(undeclaredExceptionResponse2)
        .expectErrorMatches(
            t ->
                t instanceof TApplicationException
                    && t.getMessage() != null
                    && t.getMessage().contains("exc"))
        .verify();
  }

  /**
   * Test that exceptions thrown by the service handler are properly propagated to clients over
   * Header protocol. Header protocol should also properly handle and propagate exceptions.
   */
  @Test
  public void testErrorPropagationHeader() throws Exception {
    transport =
        UnifiedServerTransport.createNewInstance(
                serverAddress, rpcServerHandler, serverConfig, new SPINiftyMetrics())
            .block();
    assertNotNull(transport);

    RpcClientFactory factory = createHeaderClientFactory();
    client =
        TestService.Reactive.clientBuilder()
            .setProtocolId(ProtocolId.COMPACT)
            .build(factory, serverAddress);

    TestRequest request = new TestRequest.Builder().setIntField(1).setStrField("error").build();

    // Test declared exception propagation over Header protocol
    Flux<TestResponse> declaredExceptionResponse = client.streamDeclaredException(request);
    StepVerifier.create(declaredExceptionResponse)
        .expectError(UnsupportedOperationException.class)
        .verify();

    // Test undeclared exception propagation over Header protocol
    Flux<TestResponse> undeclaredExceptionResponse = client.streamUndeclaredException(request);
    StepVerifier.create(undeclaredExceptionResponse).expectError().verify();
  }

  /**
   * Test concurrent request handling. This validates that the server can handle multiple
   * simultaneous requests from different clients correctly without data corruption or errors.
   */
  @Test
  public void testConcurrentRequests() throws Exception {
    transport =
        UnifiedServerTransport.createNewInstance(
                serverAddress, rpcServerHandler, serverConfig, new SPINiftyMetrics())
            .block();
    assertNotNull(transport);

    RpcClientFactory factory = createRSocketClientFactory();
    final int numClients = 5;
    final int requestsPerClient = 10;
    TestService.Reactive[] clients = new TestService.Reactive[numClients];

    try {
      // Create multiple clients
      for (int i = 0; i < numClients; i++) {
        clients[i] =
            TestService.Reactive.clientBuilder()
                .setProtocolId(ProtocolId.COMPACT)
                .build(factory, serverAddress);
      }

      // Create concurrent requests from all clients
      Flux<TestResponse>[] responses = new Flux[numClients * requestsPerClient];
      int requestIndex = 0;

      for (int clientIdx = 0; clientIdx < numClients; clientIdx++) {
        final int clientId = clientIdx;
        for (int reqIdx = 0; reqIdx < requestsPerClient; reqIdx++) {
          final int requestId = reqIdx;
          TestRequest request =
              new TestRequest.Builder()
                  .setIntField(clientId * 100 + requestId)
                  .setStrField("client-" + clientId + "-req-" + requestId)
                  .build();

          responses[requestIndex++] = clients[clientId].requestResponse(request).flux();
        }
      }

      // Merge all concurrent requests and verify all complete successfully
      Flux<TestResponse> allResponses = Flux.merge(responses);

      StepVerifier.create(allResponses)
          .expectNextCount(numClients * requestsPerClient)
          .verifyComplete();

      // Verify each response has correct data by making individual requests
      for (int clientIdx = 0; clientIdx < numClients; clientIdx++) {
        final int clientId = clientIdx;
        TestRequest request =
            new TestRequest.Builder()
                .setIntField(clientId * 100)
                .setStrField("verify-client-" + clientId)
                .build();

        StepVerifier.create(clients[clientId].requestResponse(request))
            .assertNext(
                resp -> {
                  assertEquals(clientId * 100, resp.getIntField());
                  assertEquals("verify-client-" + clientId, resp.getStrField());
                })
            .verifyComplete();
      }

    } finally {
      // Clean up all client connections
      for (TestService.Reactive c : clients) {
        if (c != null) {
          try {
            c.dispose();
          } catch (Exception e) {
            // Ignore cleanup errors
          }
        }
      }
    }
  }

  /**
   * Test that connections are properly cleaned up when a client disconnects. This validates that
   * resources (channels, metrics) are released when clients close their connections.
   */
  @Test
  public void testConnectionCleanupOnDisconnect() throws Exception {
    transport =
        UnifiedServerTransport.createNewInstance(
                serverAddress, rpcServerHandler, serverConfig, new SPINiftyMetrics())
            .block();
    assertNotNull(transport);

    SPINiftyMetrics metrics = transport.getNiftyMetrics();
    assertNotNull(metrics, "Metrics should be available");

    // Get initial channel count
    int initialChannelCount = metrics.getChannelCount();
    assertEquals(0, initialChannelCount, "Initial channel count should be 0");

    RpcClientFactory factory = createRSocketClientFactory();
    TestService.Reactive client1 = null;
    TestService.Reactive client2 = null;
    TestService.Reactive client3 = null;

    try {
      // Create first client and establish connection
      client1 =
          TestService.Reactive.clientBuilder()
              .setProtocolId(ProtocolId.COMPACT)
              .build(factory, serverAddress);

      TestRequest request1 =
          new TestRequest.Builder().setIntField(1).setStrField("cleanup-test-1").build();
      StepVerifier.create(client1.requestResponse(request1))
          .assertNext(
              resp -> {
                assertEquals(1, resp.getIntField());
                assertEquals("cleanup-test-1", resp.getStrField());
              })
          .verifyComplete();

      // Verify channel count increased
      int countAfterFirst = metrics.getChannelCount();
      assertTrue(
          countAfterFirst > initialChannelCount,
          "Channel count should be greater than initial after first connection");

      // Create second client
      client2 =
          TestService.Reactive.clientBuilder()
              .setProtocolId(ProtocolId.COMPACT)
              .build(factory, serverAddress);

      TestRequest request2 =
          new TestRequest.Builder().setIntField(2).setStrField("cleanup-test-2").build();
      StepVerifier.create(client2.requestResponse(request2))
          .assertNext(
              resp -> {
                assertEquals(2, resp.getIntField());
                assertEquals("cleanup-test-2", resp.getStrField());
              })
          .verifyComplete();

      // Verify channel count increased again
      int countAfterSecond = metrics.getChannelCount();
      assertTrue(
          countAfterSecond > countAfterFirst,
          "Channel count should increase after second connection");

      // Create third client
      client3 =
          TestService.Reactive.clientBuilder()
              .setProtocolId(ProtocolId.COMPACT)
              .build(factory, serverAddress);

      TestRequest request3 =
          new TestRequest.Builder().setIntField(3).setStrField("cleanup-test-3").build();
      StepVerifier.create(client3.requestResponse(request3))
          .assertNext(
              resp -> {
                assertEquals(3, resp.getIntField());
                assertEquals("cleanup-test-3", resp.getStrField());
              })
          .verifyComplete();

      // Verify channel count increased to 3
      int countAfterThird = metrics.getChannelCount();
      assertTrue(
          countAfterThird > countAfterSecond,
          "Channel count should increase after third connection");

      // Now disconnect clients one by one and verify cleanup
      client1.dispose();
      client1 = null;
      Thread.sleep(500); // Wait for cleanup

      int countAfterFirstDisconnect = metrics.getChannelCount();
      assertTrue(
          countAfterFirstDisconnect < countAfterThird,
          "Channel count should decrease after first disconnect. Before: "
              + countAfterThird
              + ", After: "
              + countAfterFirstDisconnect);

      client2.dispose();
      client2 = null;
      Thread.sleep(500); // Wait for cleanup

      int countAfterSecondDisconnect = metrics.getChannelCount();
      assertTrue(
          countAfterSecondDisconnect < countAfterFirstDisconnect,
          "Channel count should decrease after second disconnect. Before: "
              + countAfterFirstDisconnect
              + ", After: "
              + countAfterSecondDisconnect);

      client3.dispose();
      client3 = null;
      Thread.sleep(500); // Wait for cleanup

      int finalChannelCount = metrics.getChannelCount();
      assertEquals(
          initialChannelCount,
          finalChannelCount,
          "Final channel count should return to initial count after all disconnects");

    } finally {
      // Clean up any remaining connections
      if (client1 != null) {
        try {
          client1.dispose();
        } catch (Exception e) {
          // Ignore cleanup errors
        }
      }
      if (client2 != null) {
        try {
          client2.dispose();
        } catch (Exception e) {
          // Ignore cleanup errors
        }
      }
      if (client3 != null) {
        try {
          client3.dispose();
        } catch (Exception e) {
          // Ignore cleanup errors
        }
      }
    }
  }

  /**
   * Test that disposing the transport prevents new connections while allowing existing connections
   * to continue working. This validates that dispose() stops accepting new connections but doesn't
   * kill active ones.
   */
  @Test
  public void testGracefulShutdownWithActiveConnections() throws Exception {
    transport =
        UnifiedServerTransport.createNewInstance(
                serverAddress, rpcServerHandler, serverConfig, new SPINiftyMetrics())
            .block();
    assertNotNull(transport);

    RpcClientFactory factory = createRSocketClientFactory();
    TestService.Reactive client1 = null;
    TestService.Reactive client2 = null;
    TestService.Reactive client3 = null;

    try {
      // Create and establish first client connection
      client1 =
          TestService.Reactive.clientBuilder()
              .setProtocolId(ProtocolId.COMPACT)
              .build(factory, serverAddress);

      TestRequest request1 =
          new TestRequest.Builder().setIntField(1).setStrField("shutdown-test-1").build();
      StepVerifier.create(client1.requestResponse(request1))
          .assertNext(
              resp -> {
                assertEquals(1, resp.getIntField());
                assertEquals("shutdown-test-1", resp.getStrField());
              })
          .verifyComplete();

      // Create and establish second client connection
      client2 =
          TestService.Reactive.clientBuilder()
              .setProtocolId(ProtocolId.COMPACT)
              .build(factory, serverAddress);

      TestRequest request2 =
          new TestRequest.Builder().setIntField(2).setStrField("shutdown-test-2").build();
      StepVerifier.create(client2.requestResponse(request2))
          .assertNext(
              resp -> {
                assertEquals(2, resp.getIntField());
                assertEquals("shutdown-test-2", resp.getStrField());
              })
          .verifyComplete();

      // Verify we have active connections
      SPINiftyMetrics metrics = transport.getNiftyMetrics();
      int activeConnectionsBeforeDispose = metrics.getChannelCount();
      assertTrue(
          activeConnectionsBeforeDispose > 0,
          "Should have active connections before dispose, got: " + activeConnectionsBeforeDispose);

      // Dispose the transport - this should prevent new connections but keep existing ones alive
      transport.dispose();

      // Verify existing connections still work after dispose
      TestRequest request3 =
          new TestRequest.Builder().setIntField(3).setStrField("after-dispose-1").build();
      StepVerifier.create(client1.requestResponse(request3))
          .assertNext(
              resp -> {
                assertEquals(3, resp.getIntField());
                assertEquals("after-dispose-1", resp.getStrField());
              })
          .verifyComplete();

      TestRequest request4 =
          new TestRequest.Builder().setIntField(4).setStrField("after-dispose-2").build();
      StepVerifier.create(client2.requestResponse(request4))
          .assertNext(
              resp -> {
                assertEquals(4, resp.getIntField());
                assertEquals("after-dispose-2", resp.getStrField());
              })
          .verifyComplete();

      // Verify active connections are still there
      int activeConnectionsAfterDispose = metrics.getChannelCount();
      assertEquals(
          activeConnectionsBeforeDispose,
          activeConnectionsAfterDispose,
          "Existing connections should still be active after dispose");

      // Now try to create a NEW connection - this should fail
      try {
        client3 =
            TestService.Reactive.clientBuilder()
                .setProtocolId(ProtocolId.COMPACT)
                .build(factory, serverAddress);

        TestRequest request5 =
            new TestRequest.Builder().setIntField(5).setStrField("new-connection").build();
        client3.requestResponse(request5).block(java.time.Duration.ofSeconds(2));
        fail("New connection should fail after transport is disposed");
      } catch (Exception e) {
        // Expected - server is no longer accepting new connections
      }

      // Verify onClose completes
      StepVerifier.create(transport.onClose())
          .expectComplete()
          .verify(java.time.Duration.ofSeconds(5));

    } finally {
      // Clean up client connections
      if (client1 != null) {
        try {
          client1.dispose();
        } catch (Exception e) {
          // Ignore cleanup errors
        }
      }
      if (client2 != null) {
        try {
          client2.dispose();
        } catch (Exception e) {
          // Ignore cleanup errors
        }
      }
      if (client3 != null) {
        try {
          client3.dispose();
        } catch (Exception e) {
          // Ignore cleanup errors
        }
      }
      // Set transport to null so tearDown doesn't try to dispose it again
      transport = null;
    }
  }

  /**
   * Test connection limit enforcement by verifying the transport respects the configured limit.
   * This test creates multiple clients to verify that connections beyond the limit are rejected.
   */
  @Test
  public void testConnectionLimitConfiguration() throws Exception {
    // Create transport with a connection limit of 3 for easier testing
    final int connectionLimit = 3;
    ThriftServerConfig limitedConfig =
        new ThriftServerConfig()
            .setSslEnabled(true)
            .setEnableJdkSsl(false)
            .setKeyFile(TestCertificateUtil.getKeyFilePath())
            .setCertFile(TestCertificateUtil.getCertFilePath())
            .setCAFile(TestCertificateUtil.getCAFilePath())
            .setEnableAlpn(true)
            .setConnectionLimit(connectionLimit);

    transport =
        UnifiedServerTransport.createNewInstance(
                serverAddress, rpcServerHandler, limitedConfig, new SPINiftyMetrics())
            .block();
    assertNotNull(transport);

    SPINiftyMetrics metrics = transport.getNiftyMetrics();
    assertNotNull(metrics, "Metrics should be available");

    RpcClientFactory factory = createRSocketClientFactory();
    TestService.Reactive[] clients = new TestService.Reactive[connectionLimit + 2];

    try {
      // Create connections up to the limit - these should succeed
      for (int i = 0; i < connectionLimit; i++) {
        clients[i] =
            TestService.Reactive.clientBuilder()
                .setProtocolId(ProtocolId.COMPACT)
                .build(factory, serverAddress);

        // Make a successful request to establish the connection
        TestRequest request =
            new TestRequest.Builder().setIntField(i).setStrField("connection-" + i).build();

        int c = i;
        StepVerifier.create(clients[i].requestResponse(request))
            .assertNext(
                resp -> {
                  assertEquals(c, resp.getIntField());
                  assertEquals("connection-" + c, resp.getStrField());
                })
            .verifyComplete();
      }

      // Verify we have the expected number of active connections
      int activeConnections = metrics.getChannelCount();
      assertTrue(
          activeConnections <= connectionLimit && activeConnections > 0,
          "Should have "
              + connectionLimit
              + " or fewer active connections, got: "
              + activeConnections);

      // Get baseline rejected connection count
      long initialRejectedCount = metrics.getRejectedConnections();

      // Try to create additional connections beyond the limit
      // These should be rejected and increment the rejected connection counter
      for (int i = connectionLimit; i < connectionLimit + 2; i++) {
        clients[i] =
            TestService.Reactive.clientBuilder()
                .setProtocolId(ProtocolId.COMPACT)
                .build(factory, serverAddress);

        TestRequest request =
            new TestRequest.Builder().setIntField(i).setStrField("rejected-" + i).build();

        TestService.Reactive client = clients[i];
        // This request should fail because the connection limit is reached
        assertThrows(
            Exception.class,
            () -> client.requestResponse(request).block(java.time.Duration.ofSeconds(5)));
      }

      // Verify that rejected connection count increased
      long finalRejectedCount = metrics.getRejectedConnections();
      assertTrue(
          finalRejectedCount > initialRejectedCount,
          "Rejected connection count should have increased from "
              + initialRejectedCount
              + " but is "
              + finalRejectedCount);

    } finally {
      // Clean up all client connections
      for (TestService.Reactive c : clients) {
        if (c != null) {
          try {
            c.dispose();
          } catch (Exception e) {
            // Ignore cleanup errors
          }
        }
      }
    }
  }

  /** Test that transport can be properly disposed and cleaned up. */
  @Test
  public void testTransportDisposal() {
    transport =
        UnifiedServerTransport.createNewInstance(
                serverAddress, rpcServerHandler, serverConfig, new SPINiftyMetrics())
            .block();
    assertNotNull(transport);

    transport.dispose();

    // Verify onClose() completes after dispose
    StepVerifier.create(transport.onClose())
        .expectComplete()
        .verify(java.time.Duration.ofSeconds(5));
  }

  /** Test that the transport returns the correct address after binding. */
  @Test
  public void testGetAddress() {
    transport =
        UnifiedServerTransport.createNewInstance(
                serverAddress, rpcServerHandler, serverConfig, new SPINiftyMetrics())
            .block();
    assertNotNull(transport);

    SocketAddress address = transport.getAddress();
    assertNotNull(address, "Address should not be null");
    assertTrue(address instanceof InetSocketAddress, "Address should be InetSocketAddress");
  }

  /** Test void response methods work correctly. */
  @Test
  public void testVoidResponseMethod() throws Exception {
    transport =
        UnifiedServerTransport.createNewInstance(
                serverAddress, rpcServerHandler, serverConfig, new SPINiftyMetrics())
            .block();
    assertNotNull(transport);

    RpcClientFactory factory = createRSocketClientFactory();
    client =
        TestService.Reactive.clientBuilder()
            .setProtocolId(ProtocolId.COMPACT)
            .build(factory, serverAddress);

    TestRequest request =
        new TestRequest.Builder().setIntField(99).setStrField("void-test").build();

    Mono<Void> response = client.requestResponseVoid(request);

    StepVerifier.create(response).verifyComplete();

    // Verify the handler received the request
    assertEquals(99, TestServiceHandler.inputParameter.getIntField());
    assertEquals("void-test", TestServiceHandler.inputParameter.getStrField());
  }

  /** Test that onClose() Mono completes when transport is disposed. */
  @Test
  public void testOnCloseCompletesOnDisposal() {
    transport =
        UnifiedServerTransport.createNewInstance(
                serverAddress, rpcServerHandler, serverConfig, new SPINiftyMetrics())
            .block();
    assertNotNull(transport);

    Mono<Void> onClose = transport.onClose();

    // Initially, onClose should not complete
    assertNotNull(onClose, "onClose should return a Mono");

    // Dispose the transport
    transport.dispose();

    // Now onClose should complete
    StepVerifier.create(onClose).expectComplete().verify(java.time.Duration.ofSeconds(5));
  }

  /**
   * Test concurrent request handling over Header protocol. Unlike RSocket, the Header protocol does
   * not support request multiplexing on a single connection. This test validates that the server
   * correctly handles concurrent connections, each sending one request at a time.
   */
  @Test
  public void testConcurrentHeaderRequests() throws Exception {
    transport =
        UnifiedServerTransport.createNewInstance(
                serverAddress, rpcServerHandler, serverConfig, new SPINiftyMetrics())
            .block();
    assertNotNull(transport);

    RpcClientFactory factory = createHeaderClientFactory();
    final int numClients = 10;
    TestService.Reactive[] clients = new TestService.Reactive[numClients];

    try {
      // Create multiple clients using Header protocol (one connection each)
      for (int i = 0; i < numClients; i++) {
        clients[i] =
            TestService.Reactive.clientBuilder()
                .setProtocolId(ProtocolId.COMPACT)
                .build(factory, serverAddress);
      }

      // Send one concurrent request per client (tests concurrent connections)
      Flux<TestResponse>[] responses = new Flux[numClients];
      for (int i = 0; i < numClients; i++) {
        final int clientId = i;
        TestRequest request =
            new TestRequest.Builder()
                .setIntField(clientId)
                .setStrField("header-client-" + clientId)
                .build();
        responses[i] = clients[i].requestResponse(request).flux();
      }

      // Merge all concurrent requests and verify all complete successfully
      Flux<TestResponse> allResponses = Flux.merge(responses);

      StepVerifier.create(allResponses).expectNextCount(numClients).verifyComplete();

    } finally {
      // Clean up all client connections
      for (TestService.Reactive c : clients) {
        if (c != null) {
          try {
            c.dispose();
          } catch (Exception e) {
            // Ignore cleanup errors
          }
        }
      }
    }
  }

  /** Test that multiple transports can be created on different ports. */
  @Test
  public void testMultipleTransportInstances() {
    SocketAddress address1 = new InetSocketAddress("localhost", RpcServerUtils.findFreePort());
    SocketAddress address2 = new InetSocketAddress("localhost", RpcServerUtils.findFreePort());

    UnifiedServerTransport transport1 =
        UnifiedServerTransport.createNewInstance(
                address1, rpcServerHandler, serverConfig, new SPINiftyMetrics())
            .block();
    UnifiedServerTransport transport2 =
        UnifiedServerTransport.createNewInstance(
                address2, rpcServerHandler, serverConfig, new SPINiftyMetrics())
            .block();

    try {
      assertNotNull(transport1, "First transport should be created");
      assertNotNull(transport2, "Second transport should be created");
      assertNotSame(transport1, transport2, "Transports should be different instances");
      assertNotEquals(
          transport1.getAddress(),
          transport2.getAddress(),
          "Transports should have different addresses");
    } finally {
      if (transport1 != null) {
        transport1.dispose();
      }
      if (transport2 != null) {
        transport2.dispose();
      }
    }
  }

  /**
   * Test that RequestContext is correctly propagated to user service handler code via the generated
   * RpcServerHandler's ThreadLocal management, without the transport layer needing to set it.
   *
   * <p>This validates that the generated code (via Mono.defer + setCurrentContext) correctly
   * provides the RequestContext to user code on the execution thread, for both Header and RSocket
   * transport paths.
   */
  @Test
  public void testRequestContextPropagationViaHeaderTransport() throws Exception {
    AtomicReference<RequestContext> capturedContext = new AtomicReference<>();

    // Create a handler that captures the RequestContext visible to user code
    TestService.Reactive contextCapturingHandler =
        new TestServiceHandler() {
          @Override
          public Mono<TestResponse> requestResponse(TestRequest testRequest) {
            capturedContext.set(RequestContexts.getCurrentContext());
            return super.requestResponse(testRequest);
          }
        };

    rpcServerHandler = TestService.Reactive.serverHandlerBuilder(contextCapturingHandler).build();

    transport =
        UnifiedServerTransport.createNewInstance(
                serverAddress, rpcServerHandler, serverConfig, new SPINiftyMetrics())
            .block();
    assertNotNull(transport);

    // Use Header client — exercises ThriftConnectionAcceptor path
    RpcClientFactory factory = createHeaderClientFactory();
    client =
        TestService.Reactive.clientBuilder()
            .setProtocolId(ProtocolId.COMPACT)
            .build(factory, transport.getAddress());

    TestRequest request =
        new TestRequest.Builder().setIntField(42).setStrField("context-test").build();

    StepVerifier.create(client.requestResponse(request))
        .assertNext(
            resp -> {
              assertEquals(42, resp.getIntField());
              assertEquals("context-test", resp.getStrField());
            })
        .verifyComplete();

    // Verify the handler saw a valid RequestContext on its execution thread
    RequestContext ctx = capturedContext.get();
    assertNotNull(ctx, "RequestContext should be available to user handler code via ThreadLocal");
    assertNotNull(
        ctx.getConnectionContext(),
        "RequestContext should have a ConnectionContext with connection info");
    assertNotNull(
        ctx.getConnectionContext().getRemoteAddress(),
        "ConnectionContext should have the client's remote address");
  }

  /**
   * Test that RequestContext is correctly propagated via the RSocket transport path. This ensures
   * parity between Header and RSocket paths for context propagation.
   */
  @Test
  public void testRequestContextPropagationViaRSocketTransport() throws Exception {
    AtomicReference<RequestContext> capturedContext = new AtomicReference<>();

    TestService.Reactive contextCapturingHandler =
        new TestServiceHandler() {
          @Override
          public Mono<TestResponse> requestResponse(TestRequest testRequest) {
            capturedContext.set(RequestContexts.getCurrentContext());
            return super.requestResponse(testRequest);
          }
        };

    rpcServerHandler = TestService.Reactive.serverHandlerBuilder(contextCapturingHandler).build();

    transport =
        UnifiedServerTransport.createNewInstance(
                serverAddress, rpcServerHandler, serverConfig, new SPINiftyMetrics())
            .block();
    assertNotNull(transport);

    // Use RSocket client — exercises ThriftServerRSocket path
    RpcClientFactory factory = createRSocketClientFactory();
    client =
        TestService.Reactive.clientBuilder()
            .setProtocolId(ProtocolId.COMPACT)
            .build(factory, transport.getAddress());

    TestRequest request =
        new TestRequest.Builder().setIntField(99).setStrField("rsocket-context-test").build();

    StepVerifier.create(client.requestResponse(request))
        .assertNext(
            resp -> {
              assertEquals(99, resp.getIntField());
              assertEquals("rsocket-context-test", resp.getStrField());
            })
        .verifyComplete();

    RequestContext ctx = capturedContext.get();
    assertNotNull(ctx, "RequestContext should be available to user handler code via ThreadLocal");
  }

  /**
   * Helper method to create RpcClientFactory for RSocket connections. This creates a client that
   * will use ALPN to negotiate "rs" protocol and trust the self-signed certificate.
   */
  private RpcClientFactory createRSocketClientFactory() throws Exception {

    // Configure client with same certificates for mutual trust
    ThriftClientConfig clientConfig =
        new ThriftClientConfig()
            .setDisableSSL(false)
            .setEnableJdkSsl(false)
            .setCertFile(TestCertificateUtil.getCertFilePath())
            .setKeyFile(TestCertificateUtil.getKeyFilePath())
            .setCAFile(TestCertificateUtil.getCAFilePath())
            .setRequestTimeout(Duration.succinctDuration(30, TimeUnit.SECONDS));

    return RpcClientFactory.builder()
        .setDisableRSocket(false)
        .setDisableLoadBalancing(true)
        .setDisableReconnectingClient(true)
        .setThriftClientConfig(clientConfig)
        .build();
  }

  /**
   * Helper method to create RpcClientFactory for Header connections. This creates a client that
   * will use ALPN to negotiate "header" protocol and trust the self-signed certificate.
   */
  private RpcClientFactory createHeaderClientFactory() throws Exception {
    // Configure client with same certificates for mutual trust
    ThriftClientConfig clientConfig =
        new ThriftClientConfig()
            .setDisableSSL(false)
            .setEnableJdkSsl(false)
            .setCertFile(TestCertificateUtil.getCertFilePath())
            .setKeyFile(TestCertificateUtil.getKeyFilePath())
            .setCAFile(TestCertificateUtil.getCAFilePath())
            .setRequestTimeout(Duration.succinctDuration(30, TimeUnit.SECONDS));

    return RpcClientFactory.builder()
        .setDisableRSocket(true)
        .setDisableLoadBalancing(true)
        .setDisableReconnectingClient(true)
        .setThriftClientConfig(clientConfig)
        .build();
  }
}
