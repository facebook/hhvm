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

import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;

import com.facebook.swift.service.ThriftServerConfig;
import com.facebook.thrift.client.RpcClientFactory;
import com.facebook.thrift.client.ThriftClientConfig;
import com.facebook.thrift.example.ping.PingRequest;
import com.facebook.thrift.example.ping.PingResponse;
import com.facebook.thrift.example.ping.PingService;
import com.facebook.thrift.example.ping.PingServiceReactiveClient;
import com.facebook.thrift.example.ping.PingServiceRpcServerHandler;
import com.facebook.thrift.legacy.client.LegacyRpcClientFactory;
import com.facebook.thrift.legacy.server.LegacyServerTransport;
import com.facebook.thrift.legacy.server.LegacyServerTransportFactory;
import com.facebook.thrift.legacy.server.testservices.BlockingPingService;
import io.airlift.units.DataSize;
import io.airlift.units.Duration;
import java.net.InetSocketAddress;
import java.util.Collections;
import java.util.concurrent.TimeUnit;
import org.apache.thrift.ProtocolId;
import org.junit.Test;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class TestServerDecoder {

  private static final Logger LOG = LoggerFactory.getLogger(TestServerDecoder.class);

  @Test
  public void testClientCannotExceedMaxFrameSize() {
    DataSize maxFrameSize = new DataSize(1, DataSize.Unit.KILOBYTE);
    RpcServerHandler serverHandler =
        new PingServiceRpcServerHandler(new BlockingPingService(), Collections.emptyList());

    ThriftServerConfig serverConfig =
        new ThriftServerConfig().setMaxFrameSize(maxFrameSize).setEnableJdkSsl(false);

    LegacyServerTransportFactory transportFactory = new LegacyServerTransportFactory(serverConfig);
    LegacyServerTransport transport = transportFactory.createServerTransport(serverHandler).block();
    InetSocketAddress address = (InetSocketAddress) transport.getAddress();
    LOG.info("Server started at: {}", address);

    RpcClientFactory factory =
        RpcClientFactory.builder()
            .setDisableLoadBalancing(true)
            .setThriftClientConfig(
                new ThriftClientConfig()
                    .setDisableSSL(true)
                    .setRequestTimeout(Duration.succinctDuration(10, TimeUnit.SECONDS)))
            .build();

    PingService client =
        PingService.clientBuilder().setProtocolId(ProtocolId.BINARY).build(factory, address);

    // Create message that exceeds maxFrameSize (1KB)
    StringBuilder largeString = new StringBuilder();
    for (int i = 0; i < 10 * 1024; i++) {
      largeString.append('A');
    }

    PingRequest largeRequest = new PingRequest.Builder().setRequest(largeString.toString()).build();

    try {
      client.ping(largeRequest);
      fail(
          "Expected an exception when sending message larger than maxFrameSize, but request"
              + " succeeded. This indicates the bug where maxFrameSize is not enforced.");
    } catch (Exception e) {
      assertTrue(
          "Expected a frame size related exception",
          e.getMessage().contains("frame")
              || e.getMessage().contains("size")
              || e.getMessage().contains("large")
              || e.getCause() != null);
    } finally {
      transport.dispose();
    }
  }

  @Test
  public void testClientCanSendSmallMessage() {
    DataSize maxFrameSize = new DataSize(64, DataSize.Unit.KILOBYTE);
    RpcServerHandler serverHandler =
        new PingServiceRpcServerHandler(new BlockingPingService(), Collections.emptyList());
    LegacyServerTransportFactory transportFactory =
        new LegacyServerTransportFactory(
            new ThriftServerConfig().setMaxFrameSize(maxFrameSize).setEnableJdkSsl(false));
    LegacyServerTransport transport = transportFactory.createServerTransport(serverHandler).block();
    InetSocketAddress address = (InetSocketAddress) transport.getAddress();

    RpcClientFactory factory =
        RpcClientFactory.builder()
            .setDisableLoadBalancing(true)
            .setThriftClientConfig(
                new ThriftClientConfig()
                    .setDisableSSL(true)
                    .setRequestTimeout(Duration.succinctDuration(10, TimeUnit.SECONDS)))
            .build();
    PingService client =
        PingService.clientBuilder().setProtocolId(ProtocolId.BINARY).build(factory, address);

    PingRequest request = new PingRequest.Builder().setRequest("small message").build();
    try {
      PingResponse response = client.ping(request);
      assertTrue("Expected echo response", response.getResponse().contains("small message"));
    } catch (Exception e) {
      fail("Small message should be accepted but got exception: " + e);
    } finally {
      transport.dispose();
    }
  }

  @Test
  public void testFramedProtocolCannotExceedMaxFrameSize() {
    DataSize maxFrameSize = new DataSize(1, DataSize.Unit.KILOBYTE);
    RpcServerHandler serverHandler =
        new PingServiceRpcServerHandler(new BlockingPingService(), Collections.emptyList());

    ThriftServerConfig serverConfig =
        new ThriftServerConfig().setMaxFrameSize(maxFrameSize).setEnableJdkSsl(false);

    LegacyServerTransportFactory transportFactory = new LegacyServerTransportFactory(serverConfig);
    LegacyServerTransport transport = transportFactory.createServerTransport(serverHandler).block();
    InetSocketAddress address = (InetSocketAddress) transport.getAddress();

    // Use LegacyRpcClientFactory which uses framed protocol
    LegacyRpcClientFactory clientFactory =
        new LegacyRpcClientFactory(
            new ThriftClientConfig()
                .setDisableSSL(true)
                .setRequestTimeout(Duration.succinctDuration(10, TimeUnit.SECONDS)));

    PingServiceReactiveClient client =
        new PingServiceReactiveClient(
            ProtocolId.BINARY, clientFactory.createRpcClient(address).cache());

    // Create message that exceeds maxFrameSize (1KB)
    StringBuilder largeString = new StringBuilder();
    for (int i = 0; i < 10 * 1024; i++) {
      largeString.append('A');
    }

    PingRequest largeRequest = new PingRequest.Builder().setRequest(largeString.toString()).build();

    try {
      client.ping(largeRequest).block();
      fail(
          "Expected an exception when sending message larger than maxFrameSize on framed protocol,"
              + " but request succeeded. This indicates maxFrameSize is not enforced.");
    } catch (Exception e) {
      assertTrue(
          "Expected a frame size related exception",
          e.getMessage().contains("frame")
              || e.getMessage().contains("size")
              || e.getMessage().contains("length")
              || e.getCause() != null);
    } finally {
      transport.dispose();
    }
  }
}
