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

import com.facebook.swift.service.ThriftServerConfig;
import com.facebook.thrift.client.ThriftClientConfig;
import com.facebook.thrift.example.blocking.BlockingTestService;
import com.facebook.thrift.example.blocking.BlockingTestServiceRpcServerHandler;
import com.facebook.thrift.example.blocking.PingRequest;
import com.facebook.thrift.legacy.client.LegacyRpcClientFactory;
import com.facebook.thrift.legacy.server.LegacyServerTransport;
import com.facebook.thrift.legacy.server.LegacyServerTransportFactory;
import com.facebook.thrift.legacy.server.testservices.AsyncBlockingTestService;
import io.airlift.units.Duration;
import java.net.SocketAddress;
import java.util.Collections;
import java.util.concurrent.TimeUnit;
import org.apache.thrift.ProtocolId;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;

public class TestBlockingBehavior {
  BlockingTestService.Reactive client;

  @Before
  public void setup() {
    BlockingTestServiceRpcServerHandler serverHandler =
        new BlockingTestServiceRpcServerHandler(
            new AsyncBlockingTestService(), Collections.emptyList());

    LegacyServerTransportFactory transportFactory =
        new LegacyServerTransportFactory(new ThriftServerConfig().setSslEnabled(false));

    LegacyServerTransport transport = transportFactory.createServerTransport(serverHandler).block();
    Assert.assertFalse(transport.isDisposed());

    SocketAddress address = transport.getAddress();

    LegacyRpcClientFactory rpcClientFactory =
        new LegacyRpcClientFactory(
            new ThriftClientConfig()
                .setDisableSSL(true)
                .setRequestTimeout(Duration.succinctDuration(1, TimeUnit.DAYS)));

    client =
        BlockingTestService.Reactive.clientBuilder()
            .setProtocolId(ProtocolId.BINARY)
            .build(rpcClientFactory, address);
  }

  @Test
  public void testShouldAllowBlockingCodeToRunInsideAsyncCodeByDefault() throws Exception {
    client
        .generatePingWithBlockingCode(new PingRequest.Builder().setRequest("ping").build())
        .block();
  }
}
