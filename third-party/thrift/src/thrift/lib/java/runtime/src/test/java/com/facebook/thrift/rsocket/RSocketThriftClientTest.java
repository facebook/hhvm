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

package com.facebook.thrift.rsocket;

import static org.junit.Assert.assertEquals;

import com.facebook.swift.service.ThriftServerConfig;
import com.facebook.thrift.client.RpcClientFactory;
import com.facebook.thrift.client.ThriftClientConfig;
import com.facebook.thrift.example.ping.CustomException;
import com.facebook.thrift.example.ping.PingRequest;
import com.facebook.thrift.example.ping.PingResponse;
import com.facebook.thrift.example.ping.PingService;
import com.facebook.thrift.example.ping.PingServiceRpcServerHandler;
import com.facebook.thrift.legacy.server.testservices.BlockingPingService;
import com.facebook.thrift.legacy.server.testservices.ReactivePingService;
import com.facebook.thrift.rsocket.server.RSocketServerTransport;
import com.facebook.thrift.rsocket.server.RSocketServerTransportFactory;
import com.facebook.thrift.server.RpcServerHandler;
import com.facebook.thrift.util.FutureUtil;
import com.facebook.thrift.util.SPINiftyMetrics;
import com.google.common.util.concurrent.ListenableFuture;
import io.airlift.units.Duration;
import java.net.InetSocketAddress;
import java.nio.charset.StandardCharsets;
import java.util.Collections;
import java.util.concurrent.TimeUnit;
import org.apache.thrift.ProtocolId;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;
import reactor.core.publisher.Flux;
import reactor.test.StepVerifier;

public class RSocketThriftClientTest {

  @Rule public ExpectedException expectedException = ExpectedException.none();

  @Test(expected = CustomException.class)
  public void testPingException() {
    System.out.println("create server handler");
    RpcServerHandler serverHandler =
        new PingServiceRpcServerHandler(new BlockingPingService(), Collections.emptyList());

    System.out.println("starting server");
    RSocketServerTransportFactory transportFactory =
        new RSocketServerTransportFactory(new ThriftServerConfig().setEnableJdkSsl(false));
    RSocketServerTransport transport =
        transportFactory.createServerTransport(serverHandler).block();
    InetSocketAddress address = (InetSocketAddress) transport.getAddress();

    System.out.println("creating client");

    RpcClientFactory factory =
        RpcClientFactory.builder()
            .setDisableLoadBalancing(true)
            .setDisableRSocket(false)
            .setThriftClientConfig(
                new ThriftClientConfig()
                    .setDisableSSL(true)
                    .setRequestTimeout(Duration.succinctDuration(1, TimeUnit.DAYS)))
            .build();

    PingService client =
        PingService.clientBuilder().setProtocolId(ProtocolId.BINARY).build(factory, address);

    client.pingException(new PingRequest.Builder().setRequest("ping").build());
  }

  @Test
  public void testPingVoidBlocking() {
    System.out.println("create server handler");
    RpcServerHandler serverHandler =
        new PingServiceRpcServerHandler(new BlockingPingService(), Collections.emptyList());

    System.out.println("starting server");
    RSocketServerTransportFactory transportFactory =
        new RSocketServerTransportFactory(new ThriftServerConfig().setEnableJdkSsl(false));
    RSocketServerTransport transport =
        transportFactory.createServerTransport(serverHandler).block();
    InetSocketAddress address = (InetSocketAddress) transport.getAddress();

    System.out.println("creating client");

    RpcClientFactory factory =
        RpcClientFactory.builder()
            .setDisableLoadBalancing(true)
            .setDisableRSocket(false)
            .setThriftClientConfig(
                new ThriftClientConfig()
                    .setDisableSSL(true)
                    .setRequestTimeout(Duration.succinctDuration(1, TimeUnit.DAYS)))
            .build();

    PingService client =
        PingService.clientBuilder().setProtocolId(ProtocolId.BINARY).build(factory, address);

    client.pingVoid(new PingRequest.Builder().setRequest("ping").build());
  }

  @Test
  public void testZeroPort() {
    RpcServerHandler serverHandler =
        new PingServiceRpcServerHandler(new BlockingPingService(), Collections.emptyList());

    RSocketServerTransportFactory transportFactory =
        new RSocketServerTransportFactory(new ThriftServerConfig().setEnableJdkSsl(false));
    RSocketServerTransport transport =
        transportFactory
            .createServerTransport(new InetSocketAddress(0), serverHandler, new SPINiftyMetrics())
            .block();
    InetSocketAddress address = (InetSocketAddress) transport.getAddress();

    RpcClientFactory factory =
        RpcClientFactory.builder()
            .setDisableLoadBalancing(true)
            .setDisableRSocket(false)
            .setThriftClientConfig(
                new ThriftClientConfig()
                    .setDisableSSL(true)
                    .setRequestTimeout(Duration.succinctDuration(1, TimeUnit.DAYS)))
            .build();

    PingService client =
        PingService.clientBuilder().setProtocolId(ProtocolId.BINARY).build(factory, address);

    client.pingVoid(new PingRequest.Builder().setRequest("ping").build());
  }

  @Test
  public void testPingVoidAsync() throws Exception {
    System.out.println("create server handler");
    RpcServerHandler serverHandler =
        new PingServiceRpcServerHandler(new BlockingPingService(), Collections.emptyList());

    System.out.println("starting server");
    RSocketServerTransportFactory transportFactory =
        new RSocketServerTransportFactory(new ThriftServerConfig().setEnableJdkSsl(false));
    RSocketServerTransport transport =
        transportFactory.createServerTransport(serverHandler).block();
    InetSocketAddress address = (InetSocketAddress) transport.getAddress();

    System.out.println("creating client");

    RpcClientFactory factory =
        RpcClientFactory.builder()
            .setDisableLoadBalancing(true)
            .setDisableRSocket(false)
            .setThriftClientConfig(
                new ThriftClientConfig()
                    .setDisableSSL(true)
                    .setRequestTimeout(Duration.succinctDuration(1, TimeUnit.DAYS)))
            .build();

    PingService.Async client =
        PingService.Async.clientBuilder().setProtocolId(ProtocolId.BINARY).build(factory, address);
    client.pingVoid(new PingRequest.Builder().setRequest("ping").build()).get();
  }

  @Test
  public void testPingVoidReactive() {
    System.out.println("create server handler");
    RpcServerHandler serverHandler =
        new PingServiceRpcServerHandler(new BlockingPingService(), Collections.emptyList());

    System.out.println("starting server");
    RSocketServerTransportFactory transportFactory =
        new RSocketServerTransportFactory(new ThriftServerConfig().setEnableJdkSsl(false));
    RSocketServerTransport transport =
        transportFactory.createServerTransport(serverHandler).block();
    InetSocketAddress address = (InetSocketAddress) transport.getAddress();

    System.out.println("creating client");

    RpcClientFactory factory =
        RpcClientFactory.builder()
            .setDisableLoadBalancing(true)
            .setDisableRSocket(false)
            .setThriftClientConfig(
                new ThriftClientConfig()
                    .setDisableSSL(true)
                    .setRequestTimeout(Duration.succinctDuration(1, TimeUnit.DAYS)))
            .build();

    PingService.Reactive client =
        PingService.Reactive.clientBuilder()
            .setProtocolId(ProtocolId.BINARY)
            .build(factory, address);
    client.pingVoid(new PingRequest.Builder().setRequest("ping").build()).block();
  }

  // Set 50000 to global constant so it's easier to maintain.

  private static final int EXHUAUST_CLIENT_CALL_THRESHOLD = 50000;

  @Test
  public void testSocketExhaustWhenDestExist() {
    System.out.println("create server handler");
    RpcServerHandler serverHandler =
        new PingServiceRpcServerHandler(new BlockingPingService(), Collections.emptyList());

    System.out.println("starting server");
    RSocketServerTransportFactory transportFactory =
        new RSocketServerTransportFactory(new ThriftServerConfig().setEnableJdkSsl(false));
    RSocketServerTransport transport =
        transportFactory.createServerTransport(serverHandler).block();
    InetSocketAddress address = (InetSocketAddress) transport.getAddress();

    System.out.println("creating client");

    RpcClientFactory factory =
        RpcClientFactory.builder()
            .setDisableLoadBalancing(true)
            .setDisableRSocket(false)
            .setDisableReconnectingClient(true)
            .setThriftClientConfig(
                new ThriftClientConfig()
                    .setDisableSSL(true)
                    .setRequestTimeout(Duration.succinctDuration(1, TimeUnit.DAYS)))
            .build();
    PingService.Reactive client =
        PingService.Reactive.clientBuilder()
            .setProtocolId(ProtocolId.BINARY)
            .build(factory, address);

    for (int i = 0; i < EXHUAUST_CLIENT_CALL_THRESHOLD; i++) {
      client.ping(new PingRequest.Builder().setRequest("ping").build()).block();
      System.out.println("Sent call " + i);
    }
  }

  @Test
  public void testSocketExhaustWhenDestComesUp() {
    //  Create Client factory
    RpcClientFactory factory =
        RpcClientFactory.builder()
            .setDisableLoadBalancing(true)
            .setDisableRSocket(false)
            .setDisableReconnectingClient(true)
            .setThriftClientConfig(
                new ThriftClientConfig()
                    .setDisableSSL(true)
                    .setRequestTimeout(Duration.succinctDuration(1, TimeUnit.DAYS)))
            .build();
    InetSocketAddress address = new InetSocketAddress(12345);
    PingService.Reactive client =
        PingService.Reactive.clientBuilder()
            .setProtocolId(ProtocolId.BINARY)
            .build(factory, address);

    for (int i = 0; i < EXHUAUST_CLIENT_CALL_THRESHOLD; i++) {
      try {
        client.ping(new PingRequest.Builder().setRequest("ping").build()).block();
      } catch (Exception e) {
      }
    }

    System.out.println("create server handler");
    RpcServerHandler serverHandler =
        new PingServiceRpcServerHandler(new BlockingPingService(), Collections.emptyList());

    System.out.println("starting server");
    RSocketServerTransportFactory transportFactory =
        new RSocketServerTransportFactory(new ThriftServerConfig().setEnableJdkSsl(false));
    RSocketServerTransport transport =
        transportFactory.createServerTransport(serverHandler).block();
    address = (InetSocketAddress) transport.getAddress();

    System.out.println("creating client");
    client =
        PingService.Reactive.clientBuilder()
            .setProtocolId(ProtocolId.BINARY)
            .build(factory, address);

    // These calls should succeed.
    for (int i = 0; i < EXHUAUST_CLIENT_CALL_THRESHOLD; i++) {
      client.ping(new PingRequest.Builder().setRequest("ping").build()).block();
      System.out.println("Sent call " + i);
    }
  }

  @Test
  public void testPingBinary() {
    System.out.println("create server handler");
    RpcServerHandler serverHandler =
        new PingServiceRpcServerHandler(new BlockingPingService(), Collections.emptyList());

    System.out.println("starting server");
    RSocketServerTransportFactory transportFactory =
        new RSocketServerTransportFactory(new ThriftServerConfig().setSslEnabled(false));
    RSocketServerTransport transport =
        transportFactory.createServerTransport(serverHandler).block();
    InetSocketAddress address = (InetSocketAddress) transport.getAddress();

    System.out.println("creating client");

    RpcClientFactory factory =
        RpcClientFactory.builder()
            .setDisableLoadBalancing(true)
            .setDisableRSocket(false)
            .setThriftClientConfig(
                new ThriftClientConfig()
                    .setDisableSSL(true)
                    .setRequestTimeout(Duration.succinctDuration(1, TimeUnit.DAYS)))
            .build();

    PingService client =
        PingService.clientBuilder().setProtocolId(ProtocolId.BINARY).build(factory, address);

    byte[] result = client.pingBinary(new PingRequest.Builder().setRequest("ping").build());
    assertEquals("hello!", new String(result, StandardCharsets.UTF_8));
  }

  @Test
  public void test1StreamingPing() {
    sendNPings(1);
  }

  @Test
  public void test10StreamingPing() {
    sendNPings(10);
  }

  @Test
  public void test100StreamingPing() {
    sendNPings(100);
  }

  @Test
  public void test1000StreamingPing() {
    sendNPings(1_000);
  }

  @Test
  public void test10_000StreamingPing() {
    sendNPings(10_000);
  }

  public void sendNPings(int numOfPings) {
    System.out.println("create server handler");
    RpcServerHandler serverHandler =
        new PingServiceRpcServerHandler(new ReactivePingService(), Collections.emptyList());

    System.out.println("starting server");
    RSocketServerTransportFactory transportFactory =
        new RSocketServerTransportFactory(new ThriftServerConfig().setSslEnabled(false));
    RSocketServerTransport transport =
        transportFactory.createServerTransport(serverHandler).block();
    InetSocketAddress address = (InetSocketAddress) transport.getAddress();

    System.out.println("creating client");

    RpcClientFactory factory =
        RpcClientFactory.builder()
            .setDisableLoadBalancing(true)
            .setDisableRSocket(false)
            .setThriftClientConfig(
                new ThriftClientConfig()
                    .setDisableSSL(true)
                    .setRequestTimeout(Duration.succinctDuration(1, TimeUnit.DAYS)))
            .build();

    PingService.Reactive client =
        PingService.Reactive.clientBuilder()
            .setProtocolId(ProtocolId.BINARY)
            .build(factory, address);

    Flux<PingResponse> ping =
        client
            .streamOfPings(new PingRequest.Builder().setRequest("ping").build(), numOfPings)
            .timeout(java.time.Duration.ofSeconds(30));

    StepVerifier.create(ping).expectNextCount(numOfPings).verifyComplete();
  }

  @Test
  public void test1BlockingPingPongs() {
    sendNBlockingPingPongs(1);
  }

  @Test
  public void test10BlockingPingPongs() {
    sendNBlockingPingPongs(10);
  }

  @Test
  public void test100BlockingPingPongs() {
    sendNBlockingPingPongs(100);
  }

  @Test
  public void test1000BlockingPingPongs() {
    sendNBlockingPingPongs(1000);
  }

  private void sendNBlockingPingPongs(int n) {
    System.out.println("create server handler");
    RpcServerHandler serverHandler =
        new PingServiceRpcServerHandler(new BlockingPingService(), Collections.emptyList());

    System.out.println("starting server");
    RSocketServerTransportFactory transportFactory =
        new RSocketServerTransportFactory(new ThriftServerConfig().setEnableJdkSsl(false));
    RSocketServerTransport transport =
        transportFactory.createServerTransport(serverHandler).block();
    InetSocketAddress address = (InetSocketAddress) transport.getAddress();

    System.out.println("creating client");

    RpcClientFactory factory =
        RpcClientFactory.builder()
            .setDisableLoadBalancing(true)
            .setDisableRSocket(false)
            .setThriftClientConfig(
                new ThriftClientConfig()
                    .setDisableSSL(true)
                    .setRequestTimeout(Duration.succinctDuration(1, TimeUnit.DAYS)))
            .build();

    PingService pingService =
        PingService.clientBuilder().setProtocolId(ProtocolId.BINARY).build(factory, address);

    for (int i = 0; i < n; i++) {
      PingRequest pingRequest = new PingRequest.Builder().setRequest(i + "ping").build();
      PingResponse pingResponse = pingService.ping(pingRequest);
      assertEquals(pingResponse.getResponse(), i + "ping_pong_" + i);
    }
  }

  @Test
  public void test1AsyncPingPongs() {
    sendNAsyncPingPongs(1);
  }

  @Test
  public void test10AsyncPingPongs() {
    sendNAsyncPingPongs(10);
  }

  @Test
  public void test100AsyncPingPongs() {
    sendNAsyncPingPongs(100);
  }

  @Test
  public void test1000AsyncPingPongs() {
    sendNAsyncPingPongs(1000);
  }

  @Test
  public void test10_000AsyncPingPongs() {
    sendNAsyncPingPongs(10_000);
  }

  private void sendNAsyncPingPongs(int n) {
    System.out.println("create server handler");
    RpcServerHandler serverHandler =
        new PingServiceRpcServerHandler(new BlockingPingService(), Collections.emptyList());

    System.out.println("starting server");
    RSocketServerTransportFactory transportFactory =
        new RSocketServerTransportFactory(new ThriftServerConfig().setEnableJdkSsl(false));
    RSocketServerTransport transport =
        transportFactory.createServerTransport(serverHandler).block();
    InetSocketAddress address = (InetSocketAddress) transport.getAddress();

    System.out.println("server started on address " + address);

    System.out.println("creating client");

    RpcClientFactory factory =
        RpcClientFactory.builder()
            .setDisableLoadBalancing(true)
            .setDisableRSocket(false)
            .setThriftClientConfig(
                new ThriftClientConfig()
                    .setDisableSSL(true)
                    .setRequestTimeout(Duration.succinctDuration(1, TimeUnit.DAYS)))
            .build();

    PingService.Async pingService =
        PingService.Async.clientBuilder().setProtocolId(ProtocolId.BINARY).build(factory, address);
    System.out.println("new client created, sending ping");

    Flux.range(0, n)
        .flatMap(
            i -> {
              PingRequest pingRequest = new PingRequest.Builder().setRequest("ping").build();
              ListenableFuture<PingResponse> ping = pingService.ping(pingRequest);
              return FutureUtil.toMono(ping);
            },
            32)
        .blockLast();
  }
}
