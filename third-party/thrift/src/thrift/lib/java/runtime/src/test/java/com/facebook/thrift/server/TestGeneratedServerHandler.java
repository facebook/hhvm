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

import static org.hamcrest.CoreMatchers.instanceOf;
import static org.hamcrest.CoreMatchers.is;

import com.facebook.swift.service.ThriftServerConfig;
import com.facebook.thrift.client.ThriftClientConfig;
import com.facebook.thrift.example.ping.CustomException;
import com.facebook.thrift.example.ping.PingRequest;
import com.facebook.thrift.example.ping.PingService;
import com.facebook.thrift.example.ping.PingServiceReactiveClient;
import com.facebook.thrift.example.ping.PingServiceRpcServerHandler;
import com.facebook.thrift.legacy.client.LegacyRpcClientFactory;
import com.facebook.thrift.legacy.server.LegacyServerTransport;
import com.facebook.thrift.legacy.server.LegacyServerTransportFactory;
import com.facebook.thrift.legacy.server.testservices.BlockingPingService;
import io.airlift.units.Duration;
import java.net.InetSocketAddress;
import java.util.Collections;
import java.util.concurrent.TimeUnit;
import org.apache.thrift.ProtocolId;
import org.apache.thrift.TApplicationException;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import reactor.test.StepVerifier;

public class TestGeneratedServerHandler {
  PingService.Reactive client;

  @Before
  public void setup() {
    PingServiceRpcServerHandler serverHandler =
        new PingServiceRpcServerHandler(new BlockingPingService(), Collections.emptyList());
    LegacyServerTransportFactory transportFactory =
        new LegacyServerTransportFactory(new ThriftServerConfig().setSslEnabled(false));
    LegacyServerTransport serverTransport =
        transportFactory.createServerTransport(serverHandler).block();
    InetSocketAddress address = (InetSocketAddress) serverTransport.getAddress();

    LegacyRpcClientFactory rpcClientFactory =
        new LegacyRpcClientFactory(
            new ThriftClientConfig()
                .setDisableSSL(true)
                .setRequestTimeout(Duration.succinctDuration(1, TimeUnit.DAYS)));
    this.client =
        new PingServiceReactiveClient(
            ProtocolId.BINARY, rpcClientFactory.createRpcClient(address).cache());
  }

  @Test
  public void testPing() {
    StepVerifier.create(client.ping(new PingRequest.Builder().setRequest("ping").build()))
        .consumeNextWith(response -> Assert.assertThat(response.getResponse(), is("ping_pong_0")))
        .verifyComplete();
  }

  @Test
  public void testPingWithCustomException() {
    StepVerifier.create(client.pingException(new PingRequest.Builder().setRequest("ping").build()))
        .consumeErrorWith(
            error -> {
              Assert.assertThat(error, instanceOf(CustomException.class));
              Assert.assertThat(error.getMessage(), is("custom exception"));
            });
  }

  @Test
  public void testPingWithApplicationException() {
    StepVerifier.create(client.pingException(new PingRequest.Builder().setRequest("npe").build()))
        .consumeErrorWith(
            error -> {
              Assert.assertThat(error, instanceOf(TApplicationException.class));
              Assert.assertThat(error.getMessage(), is("npe from pingException"));
            });
  }
}
