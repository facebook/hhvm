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
import com.facebook.swift.service.ThriftEventHandler;
import com.facebook.swift.service.ThriftServerConfig;
import com.facebook.thrift.client.RpcClientFactory;
import com.facebook.thrift.client.ThriftClientConfig;
import com.facebook.thrift.example.ping.PingRequest;
import com.facebook.thrift.example.ping.PingService;
import com.facebook.thrift.example.ping.PingServiceRpcServerHandler;
import com.facebook.thrift.legacy.server.LegacyServerTransport;
import com.facebook.thrift.legacy.server.LegacyServerTransportFactory;
import com.facebook.thrift.legacy.server.testservices.BlockingPingService;
import com.facebook.thrift.rsocket.server.RSocketServerTransport;
import com.facebook.thrift.rsocket.server.RSocketServerTransportFactory;
import com.google.common.collect.ImmutableList;
import io.airlift.units.Duration;
import java.net.InetSocketAddress;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;
import org.apache.thrift.ProtocolId;
import org.junit.jupiter.api.Assertions;
import org.junit.jupiter.api.Test;

public class TestThriftEventHandlerCallback {

  private static final String EXPECTED_OUTPUT_STRING =
      "Test ThriftEventHandler.done() callback for method: %s_%s";
  private static final Integer MAX_WAIT_TIME_MS = 5000;
  private static final Integer INTERVAL_WAIT_TIME_MS = 100;

  private RpcServerHandler initializePingRpcServerHandler(ThriftEventHandler thriftEventHandler) {
    return new PingServiceRpcServerHandler(
        new BlockingPingService(), ImmutableList.of(thriftEventHandler));
  }

  private void runPingTestWithDoneCallback(
      TestDoneThriftEventHandler testDoneThriftEventHandler,
      ServerTransport transport,
      boolean isDisableRSocket)
      throws InterruptedException, TimeoutException {
    InetSocketAddress address = (InetSocketAddress) transport.getAddress();

    RpcClientFactory factory =
        RpcClientFactory.builder()
            .setDisableLoadBalancing(true)
            .setDisableRSocket(isDisableRSocket)
            .setThriftClientConfig(
                new ThriftClientConfig()
                    .setDisableSSL(true)
                    .setRequestTimeout(Duration.succinctDuration(1, TimeUnit.DAYS)))
            .build();

    PingService client =
        PingService.clientBuilder().setProtocolId(ProtocolId.BINARY).build(factory, address);

    long elapsedTime;
    int numIterations = 5;

    for (int i = 0; i < numIterations; i++) {
      client.ping(PingRequest.defaultInstance());
      long startTime = System.currentTimeMillis();
      // We need to wait for an asynchronous thread to run TestDoneThriftEventHandler.done()
      do {
        if (testDoneThriftEventHandler.outputString.equals(
            String.format(EXPECTED_OUTPUT_STRING, "PingService.ping", i))) break;

        Thread.sleep(INTERVAL_WAIT_TIME_MS);

        elapsedTime = System.currentTimeMillis() - startTime;

        if (elapsedTime >= MAX_WAIT_TIME_MS)
          throw new TimeoutException(
              "Expected: "
                  + String.format(EXPECTED_OUTPUT_STRING, "PingService.ping", i)
                  + " but got: "
                  + testDoneThriftEventHandler.outputString);

      } while (elapsedTime < MAX_WAIT_TIME_MS);
    }

    Assertions.assertEquals(
        String.format(EXPECTED_OUTPUT_STRING, "PingService.ping", numIterations - 1),
        testDoneThriftEventHandler.getOutputString());
  }

  @Test
  public void testLegacySingleRequestSingleResponse_DoneCallback()
      throws TimeoutException, InterruptedException {
    TestDoneThriftEventHandler testDoneThriftEventHandler = new TestDoneThriftEventHandler();
    RpcServerHandler serverHandler = initializePingRpcServerHandler(testDoneThriftEventHandler);

    LegacyServerTransportFactory transportFactory =
        new LegacyServerTransportFactory(new ThriftServerConfig().setEnableJdkSsl(false));
    LegacyServerTransport legacyServerTransport =
        transportFactory.createServerTransport(serverHandler).block();

    runPingTestWithDoneCallback(testDoneThriftEventHandler, legacyServerTransport, true);
  }

  @Test
  public void testRSocketSingleRequestSingleResponse_DoneCallback()
      throws InterruptedException, TimeoutException {
    TestDoneThriftEventHandler testDoneThriftEventHandler = new TestDoneThriftEventHandler();
    RpcServerHandler serverHandler = initializePingRpcServerHandler(testDoneThriftEventHandler);

    RSocketServerTransportFactory transportFactory =
        new RSocketServerTransportFactory(
            new ThriftServerConfig().setSslEnabled(false).setEnableJdkSsl(false));
    RSocketServerTransport rSocketTransport =
        transportFactory.createServerTransport(serverHandler).block();

    runPingTestWithDoneCallback(testDoneThriftEventHandler, rSocketTransport, false);
  }

  @Test
  public void testLegacySingleRequestSingleResponse_RequestContextAvailableInCallbacks() {
    RequestContextAssertingEventHandler handler = new RequestContextAssertingEventHandler();
    RpcServerHandler serverHandler =
        new PingServiceRpcServerHandler(new BlockingPingService(), ImmutableList.of(handler));

    LegacyServerTransportFactory transportFactory =
        new LegacyServerTransportFactory(new ThriftServerConfig().setEnableJdkSsl(false));
    LegacyServerTransport transport = transportFactory.createServerTransport(serverHandler).block();

    InetSocketAddress address = (InetSocketAddress) transport.getAddress();
    RpcClientFactory factory =
        RpcClientFactory.builder()
            .setDisableLoadBalancing(true)
            .setDisableRSocket(true)
            .setThriftClientConfig(
                new ThriftClientConfig()
                    .setDisableSSL(true)
                    .setRequestTimeout(Duration.succinctDuration(1, TimeUnit.DAYS)))
            .build();

    PingService client =
        PingService.clientBuilder().setProtocolId(ProtocolId.BINARY).build(factory, address);

    // ping() is blocking — by the time it returns, all server-side callbacks have completed
    client.ping(PingRequest.defaultInstance());

    Assertions.assertTrue(
        handler.contextAvailableInGetContext, "RequestContext should be available in getContext()");
    Assertions.assertTrue(
        handler.contextAvailableInPreRead, "RequestContext should be available in preRead()");
    Assertions.assertTrue(
        handler.contextAvailableInPostRead, "RequestContext should be available in postRead()");
    Assertions.assertTrue(
        handler.contextAvailableInPreWrite, "RequestContext should be available in preWrite()");
  }

  @Test
  public void testRSocketSingleRequestSingleResponse_RequestContextAvailableInCallbacks() {
    RequestContextAssertingEventHandler handler = new RequestContextAssertingEventHandler();
    RpcServerHandler serverHandler =
        new PingServiceRpcServerHandler(new BlockingPingService(), ImmutableList.of(handler));

    RSocketServerTransportFactory transportFactory =
        new RSocketServerTransportFactory(
            new ThriftServerConfig().setSslEnabled(false).setEnableJdkSsl(false));
    RSocketServerTransport transport =
        transportFactory.createServerTransport(serverHandler).block();

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

    // ping() is blocking — by the time it returns, all server-side callbacks have completed
    client.ping(PingRequest.defaultInstance());

    Assertions.assertTrue(
        handler.contextAvailableInGetContext, "RequestContext should be available in getContext()");
    Assertions.assertTrue(
        handler.contextAvailableInPreRead, "RequestContext should be available in preRead()");
    Assertions.assertTrue(
        handler.contextAvailableInPostRead, "RequestContext should be available in postRead()");
    Assertions.assertTrue(
        handler.contextAvailableInPreWrite, "RequestContext should be available in preWrite()");
  }

  private static class RequestContextAssertingEventHandler extends ThriftEventHandler {
    volatile boolean contextAvailableInGetContext;
    volatile boolean contextAvailableInPreRead;
    volatile boolean contextAvailableInPostRead;
    volatile boolean contextAvailableInPreWrite;

    @Override
    public Object getContext(String methodName, RequestContext requestContext) {
      contextAvailableInGetContext = RequestContexts.getCurrentContext() != null;
      return null;
    }

    @Override
    public void preRead(Object context, String methodName) {
      contextAvailableInPreRead = RequestContexts.getCurrentContext() != null;
    }

    @Override
    public void postRead(Object context, String methodName, Object[] args) {
      contextAvailableInPostRead = RequestContexts.getCurrentContext() != null;
    }

    @Override
    public void preWrite(Object context, String methodName, Object result) {
      contextAvailableInPreWrite = RequestContexts.getCurrentContext() != null;
    }
  }

  private static class TestDoneThriftEventHandler extends ThriftEventHandler {

    private String outputString;
    private int counter;

    public TestDoneThriftEventHandler() {
      outputString = "";
      counter = 0;
    }

    @Override
    public void done(Object context, String methodName) {
      outputString = String.format(EXPECTED_OUTPUT_STRING, methodName, counter);
      counter++;
    }

    public String getOutputString() {
      return outputString;
    }
  }
}
