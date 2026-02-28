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

import com.facebook.swift.service.ThriftServerConfig;
import com.facebook.thrift.example.ping.PingRequest;
import com.facebook.thrift.example.ping.PingServiceReactiveClient;
import com.facebook.thrift.example.ping.PingServiceRpcServerHandler;
import com.facebook.thrift.legacy.client.LegacyRpcClientFactory;
import com.facebook.thrift.legacy.server.LegacyServerTransportFactory;
import com.facebook.thrift.server.ServerTransport;
import com.facebook.thrift.testservices.BlockingPingService;
import io.airlift.units.Duration;
import java.net.ServerSocket;
import java.net.SocketAddress;
import java.util.Collections;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicReference;
import org.apache.thrift.ProtocolId;
import org.junit.Assert;
import org.junit.Test;
import org.mockito.Mockito;
import reactor.core.Exceptions;
import reactor.core.publisher.Flux;
import reactor.core.publisher.Mono;
import reactor.core.publisher.Sinks;
import reactor.core.scheduler.Schedulers;
import reactor.test.StepVerifier;
import reactor.util.function.Tuple2;
import reactor.util.function.Tuples;

public class FusableReconnectingRpcClientMonoTest {

  private static Tuple2<Integer, SocketAddress> getFreePort() {
    try {
      ServerSocket serverSocket = new ServerSocket(0);
      SocketAddress address = serverSocket.getLocalSocketAddress();
      int port = serverSocket.getLocalPort();
      serverSocket.close();

      return Tuples.of(port, address);
    } catch (Exception e) {
      throw Exceptions.propagate(e);
    }
  }

  private static ServerTransport createServerTransport(int port) {
    ThriftServerConfig config = new ThriftServerConfig().setPort(port).setEnableJdkSsl(false);
    PingServiceRpcServerHandler serverHandler =
        new PingServiceRpcServerHandler(new BlockingPingService(), Collections.emptyList());
    LegacyServerTransportFactory factory = new LegacyServerTransportFactory(config);
    return factory.createServerTransport(serverHandler).block();
  }

  @Test
  public void testGetClientFromReconnectingRpcClientMono() {
    Tuple2<Integer, SocketAddress> freePort = getFreePort();
    ServerTransport service = createServerTransport(freePort.getT1());
    try {
      LegacyRpcClientFactory nettyFactory =
          new LegacyRpcClientFactory(
              new ThriftClientConfig()
                  .setDisableSSL(true)
                  .setRequestTimeout(Duration.succinctDuration(1, TimeUnit.DAYS)));

      ReconnectingRpcClientFactory factory = new ReconnectingRpcClientFactory(nettyFactory, true);
      Mono<RpcClient> rpcClient = factory.createRpcClient(freePort.getT2());
      Assert.assertTrue(rpcClient instanceof FusableReconnectingRpcClientMono);
      RpcClient block = rpcClient.flatMap(Mono::just).block();
      Assert.assertNotNull(block);
    } finally {
      service.dispose();
    }
  }

  @Test
  public void testPingPongThrowReconnectingRpcClientMono() {
    Tuple2<Integer, SocketAddress> freePort = getFreePort();
    ServerTransport service = createServerTransport(freePort.getT1());
    try {
      LegacyRpcClientFactory nettyFactory =
          new LegacyRpcClientFactory(
              new ThriftClientConfig()
                  .setDisableSSL(true)
                  .setRequestTimeout(Duration.succinctDuration(1, TimeUnit.DAYS)));

      ReconnectingRpcClientFactory factory = new ReconnectingRpcClientFactory(nettyFactory, true);
      Mono<RpcClient> rpcClient = factory.createRpcClient(freePort.getT2());

      PingServiceReactiveClient client =
          new PingServiceReactiveClient(ProtocolId.BINARY, rpcClient);

      StepVerifier.create(client.ping(PingRequest.defaultInstance()))
          .expectNextCount(1)
          .verifyComplete();

      StepVerifier.create(client.ping(PingRequest.defaultInstance()))
          .expectNextCount(1)
          .verifyComplete();
    } finally {
      service.dispose();
    }
  }

  @Test
  public void testReconnectingWithRunningService() {
    Tuple2<Integer, SocketAddress> freePort = getFreePort();
    ServerTransport service = createServerTransport(freePort.getT1());
    LegacyRpcClientFactory nettyFactory =
        new LegacyRpcClientFactory(
            new ThriftClientConfig()
                .setDisableSSL(true)
                .setRequestTimeout(Duration.succinctDuration(1, TimeUnit.DAYS)));

    ReconnectingRpcClientFactory factory = new ReconnectingRpcClientFactory(nettyFactory, true);
    Mono<RpcClient> rpcClient = factory.createRpcClient(freePort.getT2());

    PingServiceReactiveClient client = new PingServiceReactiveClient(ProtocolId.BINARY, rpcClient);

    StepVerifier.create(client.ping(PingRequest.defaultInstance()))
        .expectNextCount(1)
        .verifyComplete();

    service.dispose();

    ServerTransport finalService = createServerTransport(freePort.getT1());
    StepVerifier.create(client.ping(PingRequest.defaultInstance()))
        .expectNextCount(1)
        .verifyComplete();
    finalService.dispose();
  }

  @Test
  public void testBeforeServerIsStarted() {
    Tuple2<Integer, SocketAddress> freePort = getFreePort();
    AtomicReference<ServerTransport> serviceRef = new AtomicReference<>();

    LegacyRpcClientFactory nettyFactory =
        new LegacyRpcClientFactory(
            new ThriftClientConfig()
                .setDisableSSL(true)
                .setRequestTimeout(Duration.succinctDuration(1, TimeUnit.DAYS)));

    ReconnectingRpcClientFactory factory = new ReconnectingRpcClientFactory(nettyFactory, true);
    Mono<RpcClient> rpcClient = factory.createRpcClient(freePort.getT2());

    PingServiceReactiveClient client = new PingServiceReactiveClient(ProtocolId.BINARY, rpcClient);

    StepVerifier.create(
            client
                .ping(PingRequest.defaultInstance())
                .doOnSubscribe(
                    s ->
                        Assert.assertNull(
                            "client should be subscribed to before service is created",
                            serviceRef.get())))
        .then(
            () -> {
              ServerTransport service = createServerTransport(freePort.getT1());
              serviceRef.set(service);
            })
        .expectNextCount(1)
        .then(() -> serviceRef.get().dispose())
        .verifyComplete();
  }

  @Test
  public void testReconnectingWithMultipleClients() {
    Tuple2<Integer, SocketAddress> freePort = getFreePort();

    ServerTransport service = createServerTransport(freePort.getT1());
    AtomicReference<ServerTransport> serviceRef = new AtomicReference<>();

    LegacyRpcClientFactory nettyFactory =
        new LegacyRpcClientFactory(
            new ThriftClientConfig()
                .setDisableSSL(true)
                .setRequestTimeout(Duration.succinctDuration(1, TimeUnit.DAYS)));

    ReconnectingRpcClientFactory factory = new ReconnectingRpcClientFactory(nettyFactory, true);
    PingServiceReactiveClient c1 =
        new PingServiceReactiveClient(ProtocolId.BINARY, factory.createRpcClient(freePort.getT2()));
    PingServiceReactiveClient c2 =
        new PingServiceReactiveClient(ProtocolId.BINARY, factory.createRpcClient(freePort.getT2()));
    PingServiceReactiveClient c3 =
        new PingServiceReactiveClient(ProtocolId.BINARY, factory.createRpcClient(freePort.getT2()));
    PingServiceReactiveClient c4 =
        new PingServiceReactiveClient(ProtocolId.BINARY, factory.createRpcClient(freePort.getT2()));

    StepVerifier.create(
            Flux.merge(
                    c1.ping(PingRequest.defaultInstance()),
                    c2.ping(PingRequest.defaultInstance()),
                    c3.ping(PingRequest.defaultInstance()),
                    c4.ping(PingRequest.defaultInstance()))
                .then())
        .verifyComplete();

    service.dispose();

    StepVerifier.create(
            Flux.merge(
                    c1.ping(PingRequest.defaultInstance()),
                    c2.ping(PingRequest.defaultInstance()),
                    c3.ping(PingRequest.defaultInstance()),
                    c4.ping(PingRequest.defaultInstance()))
                .then())
        .then(
            () -> {
              ServerTransport s = createServerTransport(freePort.getT1());
              serviceRef.set(s);
            })
        .verifyComplete();

    serviceRef.get().dispose();
  }

  @Test
  public void testReconnectingWithMultipleThreads() {
    DummyRpcClientFactory df = new DummyRpcClientFactory();

    ReconnectingRpcClientFactory factory = new ReconnectingRpcClientFactory(df, true);
    SocketAddress mock = Mockito.mock(SocketAddress.class);
    int count = 10000;

    Flux<RpcClient> sequential =
        Flux.range(0, count)
            .flatMap(__ -> factory.createRpcClient(mock).subscribeOn(Schedulers.parallel()));

    StepVerifier.create(sequential)
        .then(df::emitClient)
        .expectNextCount(50)
        .then(df::closeRpcClient)
        .then(df::emitClient)
        .expectNextCount(50)
        .then(df::closeRpcClient)
        .then(df::emitClient)
        .expectNextCount(count - 100)
        .verifyComplete();
  }

  private class DummyRpcClientFactory implements RpcClientFactory {

    private volatile Sinks.One<RpcClient> sink;

    public DummyRpcClientFactory() {
      sink = Sinks.one();
    }

    synchronized Sinks.One<RpcClient> get() {
      return sink;
    }

    synchronized void update() {
      sink = Sinks.one();
    }

    void emitClient() {
      get().tryEmitValue(new DummyRpcClient());
    }

    void closeRpcClient() {
      RpcClient block = get().asMono().block();
      update();
      block.dispose();
    }

    @Override
    public Mono<RpcClient> createRpcClient(SocketAddress socketAddress) {
      return get().asMono();
    }
  }

  private class DummyRpcClient implements RpcClient {

    private Sinks.Empty<Void> sink = Sinks.empty();

    @Override
    public Mono<Void> onClose() {
      return sink.asMono();
    }

    @Override
    public void dispose() {
      sink.tryEmitEmpty();
    }
  }
}
