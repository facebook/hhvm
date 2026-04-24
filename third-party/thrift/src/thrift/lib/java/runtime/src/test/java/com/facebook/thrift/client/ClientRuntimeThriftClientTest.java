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

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertNotSame;
import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.junit.jupiter.api.Assertions.assertTrue;

import com.facebook.swift.service.ThriftServerConfig;
import com.facebook.thrift.client.v2.manager.RpcClientManager;
import com.facebook.thrift.client.v2.manager.RpcClientManagerFactory;
import com.facebook.thrift.client.v2.transport.RpcClientFactoryV2;
import com.facebook.thrift.example.ping.CustomException;
import com.facebook.thrift.example.ping.PingRequest;
import com.facebook.thrift.example.ping.PingResponse;
import com.facebook.thrift.example.ping.PingService;
import com.facebook.thrift.example.ping.PingServiceRpcServerHandler;
import com.facebook.thrift.legacy.server.testservices.BlockingPingService;
import com.facebook.thrift.rsocket.server.RSocketServerTransportFactory;
import com.facebook.thrift.server.RpcServerHandler;
import com.facebook.thrift.server.ServerTransport;
import com.facebook.thrift.util.RpcServerUtils;
import com.facebook.thrift.util.TransportType;
import io.airlift.units.Duration;
import java.net.InetSocketAddress;
import java.nio.charset.StandardCharsets;
import java.util.Collections;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicInteger;
import org.apache.thrift.ProtocolId;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.Timeout;
import reactor.core.publisher.Flux;
import reactor.core.publisher.Mono;
import reactor.test.StepVerifier;

public class ClientRuntimeThriftClientTest {

  @Test
  @Timeout(30)
  public void testLegacyFactoryBuildKeepsReactiveClientUsableAfterDispose() {
    ServerTransport transport = createHeaderServer();
    try {
      PingService.Reactive client =
          createReactiveClient(
              createFactory(ClientRuntimeMode.LEGACY), (InetSocketAddress) transport.getAddress());

      assertPing(client, "before-close", "before-close_pong_0");

      client.dispose();

      assertFalse(client.isDisposed());
      assertPing(client, "after-close", "after-close_pong_1");
    } finally {
      transport.dispose();
    }
  }

  @Test
  @Timeout(30)
  public void testV2FactoryBuildFailsFastAfterDispose() {
    ServerTransport transport = createHeaderServer();
    try {
      PingService.Reactive client =
          createReactiveClient(
              createFactory(ClientRuntimeMode.V2), (InetSocketAddress) transport.getAddress());

      client.pingVoid(new PingRequest.Builder().setRequest("before-close-void").build()).block();
      assertPing(client, "before-close", "before-close_pong_0");

      client.dispose();

      assertTrue(client.isDisposed());
      StepVerifier.create(client.ping(new PingRequest.Builder().setRequest("after-close").build()))
          .expectError(IllegalStateException.class)
          .verify();
    } finally {
      transport.dispose();
    }
  }

  @Test
  @Timeout(30)
  public void testV2BlockingFactoryBuildOverHeaderSupportsLegacyParityCalls() {
    ServerTransport transport = createHeaderServer();
    try {
      PingService client =
          createBlockingClient(
              createFactory(ClientRuntimeMode.V2), (InetSocketAddress) transport.getAddress());

      assertBlockingClientCoreCalls(client);
    } finally {
      transport.dispose();
    }
  }

  @Test
  @Timeout(30)
  public void testV2AsyncFactoryBuildOverHeaderSupportsLegacyParityCalls() throws Exception {
    ServerTransport transport = createHeaderServer();
    try {
      PingService.Async client =
          createAsyncClient(
              createFactory(ClientRuntimeMode.V2), (InetSocketAddress) transport.getAddress());

      assertAsyncClientCoreCalls(client);
    } finally {
      transport.dispose();
    }
  }

  @Test
  @Timeout(30)
  public void testV2FactoryBuildOverHeaderSupportsReactiveServerImplementations() {
    ServerTransport transport = createHeaderServer(createReactiveServerHandler());
    try {
      PingService.Reactive client =
          createReactiveClient(
              createFactory(ClientRuntimeMode.V2), (InetSocketAddress) transport.getAddress());

      assertPing(client, "reactive-server", "reactive-server_pong_0");
    } finally {
      transport.dispose();
    }
  }

  @Test
  @Timeout(30)
  public void testV2BorrowedSourcesShareManagerButCloseIndependently() {
    ServerTransport transport = createHeaderServer();
    RpcClientManager sharedManager = null;
    try {
      sharedManager =
          createV2ManagerFactory()
              .createRpcClientManager((InetSocketAddress) transport.getAddress());

      PingService.Reactive clientA = createBorrowedReactiveClient(sharedManager);
      PingService.Reactive clientB = createBorrowedReactiveClient(sharedManager);

      assertPing(clientA, "first", "first_pong_0");

      clientA.dispose();

      assertTrue(clientA.isDisposed());
      StepVerifier.create(clientA.ping(new PingRequest.Builder().setRequest("closed").build()))
          .expectError(IllegalStateException.class)
          .verify();

      assertPing(clientB, "second", "second_pong_1");

      PingService.Reactive clientC = createBorrowedReactiveClient(sharedManager);
      assertPing(clientC, "third", "third_pong_2");

      sharedManager.dispose();

      StepVerifier.create(
              clientB.ping(new PingRequest.Builder().setRequest("after-manager").build()))
          .expectError(IllegalStateException.class)
          .verify();
      StepVerifier.create(
              clientC.ping(new PingRequest.Builder().setRequest("after-manager").build()))
          .expectError(IllegalStateException.class)
          .verify();
    } finally {
      if (sharedManager != null) {
        sharedManager.dispose();
      }
      transport.dispose();
    }
  }

  @Test
  @Timeout(30)
  public void testV2ReconnectingManagerReacquiresAfterTransportCloses() {
    ServerTransport transport = createHeaderServer();
    RpcClientManager manager = null;
    try {
      manager =
          createV2ManagerFactory()
              .createRpcClientManager((InetSocketAddress) transport.getAddress());

      RpcClient firstClient = manager.acquire().block();
      PingService.Reactive client = createBorrowedReactiveClient(manager);

      assertPing(client, "before-close", "before-close_pong_0");

      firstClient.dispose();
      firstClient.onClose().block();

      assertPing(client, "after-close", "after-close_pong_1");

      RpcClient secondClient = manager.acquire().block();
      assertNotSame(firstClient, secondClient);
    } finally {
      if (manager != null) {
        manager.dispose();
      }
      transport.dispose();
    }
  }

  @Test
  @Timeout(30)
  public void testV2BlockingFactoryBuildOverRSocketSupportsLegacyParityCalls() {
    ServerTransport transport = createRSocketServer();
    try {
      PingService client =
          createBlockingClient(
              createRSocketFactory(ClientRuntimeMode.V2, true),
              (InetSocketAddress) transport.getAddress());

      assertBlockingClientCoreCalls(client);
    } finally {
      transport.dispose();
    }
  }

  @Test
  @Timeout(30)
  public void testV2AsyncFactoryBuildOverRSocketSupportsLegacyParityCalls() throws Exception {
    ServerTransport transport = createRSocketServer();
    try {
      PingService.Async client =
          createAsyncClient(
              createRSocketFactory(ClientRuntimeMode.V2, true),
              (InetSocketAddress) transport.getAddress());

      assertAsyncClientCoreCalls(client);
    } finally {
      transport.dispose();
    }
  }

  @Test
  @Timeout(30)
  public void testV2FactoryBuildOverRSocketSupportsStreaming() {
    ServerTransport transport = createRSocketServer(createReactiveServerHandler());
    try {
      PingService.Reactive client =
          createReactiveClient(
              createRSocketFactory(ClientRuntimeMode.V2, true),
              (InetSocketAddress) transport.getAddress());

      StepVerifier.create(
              client.streamOfPings(new PingRequest.Builder().setRequest("ping").build(), 10))
          .expectNextCount(10)
          .verifyComplete();
    } finally {
      transport.dispose();
    }
  }

  @Test
  @Timeout(30)
  public void testV2FactoryBuildOverRSocketSupportsRepeatedCallsAndFailsFastAfterDispose() {
    ServerTransport transport = createRSocketServer();
    try {
      PingService.Reactive client =
          createReactiveClient(
              createRSocketFactory(ClientRuntimeMode.V2, true),
              (InetSocketAddress) transport.getAddress());

      client.pingVoid(new PingRequest.Builder().setRequest("before-close-void").build()).block();
      assertPing(client, "before-close", "before-close_pong_0");
      assertPing(client, "second-call", "second-call_pong_1");

      client.dispose();

      assertTrue(client.isDisposed());
      StepVerifier.create(client.ping(new PingRequest.Builder().setRequest("after-close").build()))
          .expectError(IllegalStateException.class)
          .verify();
    } finally {
      transport.dispose();
    }
  }

  private static void assertBlockingClientCoreCalls(PingService client) {
    client.pingVoid(new PingRequest.Builder().setRequest("blocking-void").build());
    assertBlockingPing(client, "blocking-first", "blocking-first_pong_0");
    assertBlockingPing(client, "blocking-second", "blocking-second_pong_1");
    assertEquals(
        "hello!",
        new String(
            client.pingBinary(new PingRequest.Builder().setRequest("blocking-binary").build()),
            StandardCharsets.UTF_8));
    assertThrows(
        CustomException.class,
        () -> client.pingException(new PingRequest.Builder().setRequest("blocking-error").build()));
  }

  private static void assertAsyncClientCoreCalls(PingService.Async client) throws Exception {
    client.pingVoid(new PingRequest.Builder().setRequest("async-void").build()).get();
    assertAsyncPing(client, "async-first", "async-first_pong_0");
    assertAsyncPing(client, "async-second", "async-second_pong_1");
    assertEquals(
        "hello!",
        new String(
            client.pingBinary(new PingRequest.Builder().setRequest("async-binary").build()).get(),
            StandardCharsets.UTF_8));
  }

  private static void assertBlockingPing(
      PingService client, String request, String expectedResponse) {
    assertEquals(
        expectedResponse,
        client.ping(new PingRequest.Builder().setRequest(request).build()).getResponse());
  }

  private static void assertAsyncPing(
      PingService.Async client, String request, String expectedResponse) throws Exception {
    assertEquals(
        expectedResponse,
        client.ping(new PingRequest.Builder().setRequest(request).build()).get().getResponse());
  }

  private static void assertPing(
      PingService.Reactive client, String request, String expectedResponse) {
    StepVerifier.create(client.ping(new PingRequest.Builder().setRequest(request).build()))
        .assertNext(
            response ->
                org.junit.jupiter.api.Assertions.assertEquals(
                    expectedResponse, response.getResponse()))
        .verifyComplete();
  }

  private static PingService.Reactive createReactiveClient(
      RpcClientFactory factory, InetSocketAddress address) {
    return PingService.Reactive.clientBuilder()
        .setProtocolId(ProtocolId.BINARY)
        .build(factory, address);
  }

  private static PingService createBlockingClient(
      RpcClientFactory factory, InetSocketAddress address) {
    return PingService.clientBuilder().setProtocolId(ProtocolId.BINARY).build(factory, address);
  }

  private static PingService.Async createAsyncClient(
      RpcClientFactory factory, InetSocketAddress address) {
    return PingService.Async.clientBuilder()
        .setProtocolId(ProtocolId.BINARY)
        .build(factory, address);
  }

  private static PingService.Reactive createBorrowedReactiveClient(RpcClientManager manager) {
    return PingService.Reactive.clientBuilder()
        .setProtocolId(ProtocolId.BINARY)
        .build(ClientRuntimeSelector.createBorrowedSource(manager));
  }

  private static RpcClientFactory createFactory(ClientRuntimeMode runtimeMode) {
    return createFactory(runtimeMode, false);
  }

  private static RpcClientFactory createFactory(ClientRuntimeMode runtimeMode, boolean useRSocket) {
    return createFactory(runtimeMode, useRSocket, false);
  }

  private static RpcClientFactory createFactory(
      ClientRuntimeMode runtimeMode, boolean useRSocket, boolean disableReconnectingClient) {
    return RpcClientFactory.builder()
        .setDisableLoadBalancing(true)
        .setDisableRSocket(!useRSocket)
        .setDisableReconnectingClient(disableReconnectingClient)
        .setThriftClientConfig(baseClientConfig().setClientRuntimeMode(runtimeMode))
        .build();
  }

  private static RpcClientFactory createRSocketFactory(
      ClientRuntimeMode runtimeMode, boolean disableReconnectingClient) {
    return RpcClientFactory.builder()
        .setDisableLoadBalancing(true)
        .setDisableRSocket(false)
        .setDisableReconnectingClient(disableReconnectingClient)
        .setThriftClientConfig(rSocketClientConfig().setClientRuntimeMode(runtimeMode))
        .build();
  }

  private static RpcClientManagerFactory createV2ManagerFactory() {
    return RpcClientFactoryV2.builder()
        .setDisableLoadBalancing(true)
        .setDisableRSocket(true)
        .setThriftClientConfig(baseClientConfig().setClientRuntimeMode(ClientRuntimeMode.V2))
        .buildManagerFactory();
  }

  private static ThriftClientConfig baseClientConfig() {
    return new ThriftClientConfig()
        .setDisableSSL(true)
        .setRequestTimeout(Duration.succinctDuration(1, TimeUnit.DAYS));
  }

  private static ThriftClientConfig rSocketClientConfig() {
    return new ThriftClientConfig()
        .setDisableSSL(false)
        .setEnableJdkSsl(false)
        .setRequestTimeout(Duration.succinctDuration(1, TimeUnit.DAYS));
  }

  private static ServerTransport createHeaderServer() {
    return createHeaderServer(createBlockingServerHandler());
  }

  private static ServerTransport createHeaderServer(RpcServerHandler serverHandler) {
    return createServer(TransportType.THEADER, serverHandler);
  }

  private static ServerTransport createRSocketServer() {
    return createRSocketServer(createBlockingServerHandler());
  }

  private static ServerTransport createRSocketServer(RpcServerHandler serverHandler) {
    // RSocket SSL currently assumes a dedicated listener; ALPN negotiation lands separately.
    return new RSocketServerTransportFactory(
            new ThriftServerConfig().setSslEnabled(true).setEnableJdkSsl(false))
        .createServerTransport(serverHandler)
        .block();
  }

  private static RpcServerHandler createBlockingServerHandler() {
    return new PingServiceRpcServerHandler(new BlockingPingService(), Collections.emptyList());
  }

  private static RpcServerHandler createReactiveServerHandler() {
    AtomicInteger counter = new AtomicInteger();
    return new PingServiceRpcServerHandler(
        new PingService.Reactive() {
          @Override
          public void dispose() {}

          @Override
          public Mono<PingResponse> ping(PingRequest pingRequest) {
            return Mono.just(
                new PingResponse.Builder()
                    .setResponse(pingRequest.getRequest() + "_pong_" + counter.getAndIncrement())
                    .build());
          }

          @Override
          public Mono<byte[]> pingBinary(PingRequest pingRequest) {
            return Mono.just("hello!".getBytes(StandardCharsets.UTF_8));
          }

          @Override
          public Mono<PingResponse> pingException(PingRequest pingRequest) {
            return Mono.error(
                new CustomException.Builder()
                    .setMessage("custom exception: " + pingRequest.getRequest())
                    .build());
          }

          @Override
          public Mono<Void> pingVoid(PingRequest pingRequest) {
            return Mono.empty();
          }

          @Override
          public Flux<PingResponse> streamOfPings(PingRequest request, int numberOfPings) {
            return Flux.range(1, numberOfPings)
                .map(i -> new PingResponse.Builder().setResponse("pong " + i).build());
          }
        },
        Collections.emptyList());
  }

  private static ServerTransport createServer(
      TransportType transportType, RpcServerHandler serverHandler) {
    return RpcServerUtils.createServerTransport(
            new ThriftServerConfig().setSslEnabled(false).setEnableJdkSsl(false),
            transportType,
            serverHandler)
        .block();
  }
}
