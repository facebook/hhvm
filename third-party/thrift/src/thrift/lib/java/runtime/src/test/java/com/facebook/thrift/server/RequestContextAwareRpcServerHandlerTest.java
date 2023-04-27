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

package com.facebook.thrift.server;

import com.facebook.nifty.core.RequestContext;
import com.facebook.nifty.core.RequestContexts;
import com.facebook.swift.service.ThriftServerConfig;
import com.facebook.thrift.client.RpcClientFactory;
import com.facebook.thrift.client.RpcOptions;
import com.facebook.thrift.client.RpcOptions.Builder;
import com.facebook.thrift.client.ThriftClientConfig;
import com.facebook.thrift.example.ping.PingRequest;
import com.facebook.thrift.example.ping.PingResponse;
import com.facebook.thrift.example.ping.PingService;
import com.facebook.thrift.example.ping.PingServiceRpcServerHandler;
import com.facebook.thrift.legacy.server.LegacyServerTransport;
import com.facebook.thrift.legacy.server.LegacyServerTransportFactory;
import com.google.common.util.concurrent.Futures;
import io.airlift.units.Duration;
import java.net.InetSocketAddress;
import java.util.Collections;
import java.util.HashMap;
import java.util.concurrent.TimeUnit;
import org.apache.thrift.ProtocolId;
import org.junit.Assert;
import org.junit.Test;
import org.mockito.Mockito;
import reactor.core.publisher.Mono;

public class RequestContextAwareRpcServerHandlerTest {
  @Test
  public void testRequestContextCanBeFoundAsync() {
    PingService.Async mock = Mockito.mock(PingService.Async.class);
    Mockito.when(mock.ping(Mockito.any(PingRequest.class)))
        .thenAnswer(
            invocation -> {
              String s = RequestContexts.getCurrentContext().getRequestHeader().get("message");
              return Futures.immediateFuture(new PingResponse.Builder().setResponse(s).build());
            });

    System.out.println("create server handler");
    RpcServerHandler serverHandler =
        new RequestContextAwareRpcServerHandler(
            new PingServiceRpcServerHandler(mock, Collections.emptyList()));

    System.out.println("starting server");
    LegacyServerTransportFactory transportFactory =
        new LegacyServerTransportFactory(new ThriftServerConfig().setEnableJdkSsl(false));
    LegacyServerTransport transport = transportFactory.createServerTransport(serverHandler).block();
    InetSocketAddress address = (InetSocketAddress) transport.getAddress();

    System.out.println("creating client");

    RpcClientFactory factory =
        RpcClientFactory.builder()
            .setDisableLoadBalancing(true)
            .setThriftClientConfig(
                new ThriftClientConfig()
                    .setDisableSSL(true)
                    .setRequestTimeout(Duration.succinctDuration(1, TimeUnit.DAYS)))
            .build();

    PingService client =
        PingService.clientBuilder().setProtocolId(ProtocolId.BINARY).build(factory, address);

    String s = String.valueOf(System.nanoTime());
    HashMap<String, String> map = new HashMap<>();
    map.put("message", s);
    RpcOptions message = new Builder().setRequestHeaders(map).build();

    PingResponse ping = client.ping(PingRequest.defaultInstance(), message);
    Assert.assertEquals(s, ping.getResponse());
  }

  @Test
  public void testRequestContextCanBeFoundBlocking() {
    PingService mock = Mockito.mock(PingService.class);
    Mockito.when(mock.ping(Mockito.any(PingRequest.class)))
        .thenAnswer(
            invocation -> {
              String s = RequestContexts.getCurrentContext().getRequestHeader().get("message");
              return new PingResponse.Builder().setResponse(s).build();
            });

    System.out.println("create server handler");
    RpcServerHandler serverHandler =
        new RequestContextAwareRpcServerHandler(
            new PingServiceRpcServerHandler(mock, Collections.emptyList()));

    System.out.println("starting server");
    LegacyServerTransportFactory transportFactory =
        new LegacyServerTransportFactory(new ThriftServerConfig().setEnableJdkSsl(false));
    LegacyServerTransport transport = transportFactory.createServerTransport(serverHandler).block();
    InetSocketAddress address = (InetSocketAddress) transport.getAddress();

    System.out.println("creating client");

    RpcClientFactory factory =
        RpcClientFactory.builder()
            .setDisableLoadBalancing(true)
            .setThriftClientConfig(
                new ThriftClientConfig()
                    .setDisableSSL(true)
                    .setRequestTimeout(Duration.succinctDuration(1, TimeUnit.DAYS)))
            .build();

    PingService client =
        PingService.clientBuilder().setProtocolId(ProtocolId.BINARY).build(factory, address);

    String s = String.valueOf(System.nanoTime());
    HashMap<String, String> map = new HashMap<>();
    map.put("message", s);
    RpcOptions message = new Builder().setRequestHeaders(map).build();

    PingResponse ping = client.ping(PingRequest.defaultInstance(), message);
    Assert.assertEquals(s, ping.getResponse());
  }

  @Test
  public void testRequestContextCanBeFoundReactive() {
    PingService.Reactive mock = Mockito.mock(PingService.Reactive.class);
    Mockito.when(mock.ping(Mockito.any(PingRequest.class)))
        .thenAnswer(
            invocation -> {
              return Mono.deferContextual(
                  contextView -> {
                    RequestContext context = RequestContext.fromContextView(contextView);
                    String message = context.getRequestHeader().get("message");
                    return Mono.just(new PingResponse.Builder().setResponse(message).build());
                  });
            });

    System.out.println("create server handler");
    RpcServerHandler serverHandler =
        new RequestContextAwareRpcServerHandler(
            new PingServiceRpcServerHandler(mock, Collections.emptyList()));

    System.out.println("starting server");
    LegacyServerTransportFactory transportFactory =
        new LegacyServerTransportFactory(new ThriftServerConfig().setEnableJdkSsl(false));
    LegacyServerTransport transport = transportFactory.createServerTransport(serverHandler).block();
    InetSocketAddress address = (InetSocketAddress) transport.getAddress();

    System.out.println("creating client");

    RpcClientFactory factory =
        RpcClientFactory.builder()
            .setDisableLoadBalancing(true)
            .setThriftClientConfig(
                new ThriftClientConfig()
                    .setDisableSSL(true)
                    .setRequestTimeout(Duration.succinctDuration(1, TimeUnit.DAYS)))
            .build();

    PingService client =
        PingService.clientBuilder().setProtocolId(ProtocolId.BINARY).build(factory, address);

    String s = String.valueOf(System.nanoTime());
    HashMap<String, String> map = new HashMap<>();
    map.put("message", s);
    RpcOptions message = new Builder().setRequestHeaders(map).build();

    PingResponse ping = client.ping(PingRequest.defaultInstance(), message);
    Assert.assertEquals(s, ping.getResponse());
  }
}
