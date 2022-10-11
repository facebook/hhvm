// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

package com.facebook.thrift.client;

import static org.junit.Assert.assertEquals;

import com.facebook.swift.service.ThriftServerConfig;
import com.facebook.thrift.example.ping.PingRequest;
import com.facebook.thrift.example.ping.PingResponse;
import com.facebook.thrift.example.ping.PingService;
import com.facebook.thrift.legacy.server.LegacyServerTransport;
import com.facebook.thrift.legacy.server.LegacyServerTransportFactory;
import com.facebook.thrift.legacy.server.testservices.BlockingPingService;
import com.facebook.thrift.server.RpcServerHandler;
import java.net.InetSocketAddress;
import java.util.Map;
import org.apache.thrift.ProtocolId;
import org.junit.Test;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class InstrumentedRpcClientTest {
  private static final Logger logger = LoggerFactory.getLogger(SimpleThriftClientTest.class);

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
    ThriftClientStatsHolder.getThriftClientStats().reset();
    logger.info("create server handler");
    RpcServerHandler serverHandler =
        PingService.serverHandlerBuilder(new BlockingPingService()).build();

    logger.info("starting server");
    LegacyServerTransportFactory transportFactory =
        new LegacyServerTransportFactory(new ThriftServerConfig().setSslEnabled(false));
    LegacyServerTransport transport = transportFactory.createServerTransport(serverHandler).block();
    assert transport != null;
    InetSocketAddress address = (InetSocketAddress) transport.getAddress();

    logger.info("server started at -> " + address.toString());

    logger.info("creating client");
    RpcClientFactory clientFactory =
        RpcClientFactory.builder()
            .setDisableTimeout(true)
            .setThriftClientConfig(new ThriftClientConfig().setDisableSSL(true))
            .build();

    PingService pingService =
        PingService.clientBuilder().setProtocolId(ProtocolId.BINARY).build(clientFactory, address);
    for (int i = 0; i < n; i++) {
      PingRequest pingRequest = new PingRequest.Builder().setRequest(i + "ping").build();
      PingResponse pingResponse = pingService.ping(pingRequest);
      assertEquals(pingResponse.getResponse(), i + "ping_pong_" + i);
    }

    Map<String, Long> counters = ThriftClientStatsHolder.getThriftClientStats().getCounters();

    assertEquals(n, counters.get("thrift_client.ping.num_reads.sum").intValue());
    assertEquals(n, counters.get("thrift_client.ping.num_writes.sum").intValue());

    transport.dispose();
  }
}
