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
import com.facebook.thrift.example.ping.PingService;
import com.facebook.thrift.example.ping.PingServiceRpcServerHandler;
import com.facebook.thrift.legacy.server.LegacyServerTransport;
import com.facebook.thrift.legacy.server.LegacyServerTransportFactory;
import com.facebook.thrift.legacy.server.testservices.ReactivePingService;
import java.net.ServerSocket;
import java.net.SocketAddress;
import java.util.Collections;
import java.util.concurrent.atomic.AtomicReference;
import org.junit.Test;
import reactor.core.Exceptions;
import reactor.core.publisher.Flux;
import reactor.core.publisher.Mono;
import reactor.test.StepVerifier;

public class ReconnectingRpcClientMonoTest {
  private static SocketAddress getFreePort() {
    try {
      ServerSocket serverSocket = new ServerSocket(0);
      SocketAddress address = serverSocket.getLocalSocketAddress();
      serverSocket.close();

      return address;
    } catch (Exception e) {
      throw Exceptions.propagate(e);
    }
  }

  @Test
  public void testPingPongStartingClientBeforeServer() {
    SocketAddress freePort = getFreePort();

    ThriftClientConfig clientConfig = new ThriftClientConfig();
    clientConfig.setDisableSSL(true);

    RpcClientFactory factory =
        RpcClientFactory.builder().setThriftClientConfig(clientConfig).build();

    PingService.Reactive client = PingService.Reactive.clientBuilder().build(factory, freePort);

    PingServiceRpcServerHandler rpcServerHandler =
        new PingServiceRpcServerHandler(new ReactivePingService(), Collections.emptyList());
    ThriftServerConfig thriftServerConfig = new ThriftServerConfig();
    thriftServerConfig.setSslEnabled(false);
    LegacyServerTransportFactory transportFactory =
        new LegacyServerTransportFactory(thriftServerConfig);
    Mono<? extends LegacyServerTransport> serverTransport =
        transportFactory.createServerTransport(freePort, rpcServerHandler);

    AtomicReference<LegacyServerTransport> transport = new AtomicReference<>();

    StepVerifier.create(
            client
                .ping(PingRequest.defaultInstance())
                .doOnSubscribe(s -> System.out.println("subscribed!!!")))
        .then(
            () -> {
              LegacyServerTransport t = serverTransport.block();
              transport.set(t);
            })
        .expectNextCount(1)
        .verifyComplete();

    LegacyServerTransport legacyServerTransport = transport.get();
    assert legacyServerTransport != null;
    legacyServerTransport.dispose();
  }

  @Test
  public void tesPingPongWithManyRequests() {
    SocketAddress freePort = getFreePort();

    ThriftClientConfig clientConfig = new ThriftClientConfig();
    clientConfig.setDisableSSL(true);

    RpcClientFactory factory =
        RpcClientFactory.builder().setThriftClientConfig(clientConfig).build();

    PingService.Reactive client = PingService.Reactive.clientBuilder().build(factory, freePort);

    PingServiceRpcServerHandler rpcServerHandler =
        new PingServiceRpcServerHandler(new ReactivePingService(), Collections.emptyList());
    ThriftServerConfig thriftServerConfig = new ThriftServerConfig();
    thriftServerConfig.setSslEnabled(false);
    LegacyServerTransportFactory transportFactory =
        new LegacyServerTransportFactory(thriftServerConfig);
    Mono<? extends LegacyServerTransport> serverTransport =
        transportFactory.createServerTransport(freePort, rpcServerHandler);

    AtomicReference<LegacyServerTransport> transport = new AtomicReference<>();

    Flux<?> flux =
        Flux.range(1, 10_000)
            .flatMap(
                i -> client.ping(new PingRequest.Builder().setRequest(String.valueOf(i)).build()));

    StepVerifier.create(flux)
        .then(
            () -> {
              LegacyServerTransport t = serverTransport.block();
              transport.set(t);
            })
        .expectNextCount(10_000)
        .verifyComplete();

    LegacyServerTransport legacyServerTransport = transport.get();
    assert legacyServerTransport != null;
    legacyServerTransport.dispose();
  }
}
