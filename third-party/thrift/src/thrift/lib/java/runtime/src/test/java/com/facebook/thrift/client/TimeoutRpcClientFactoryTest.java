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

import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;

import com.facebook.thrift.payload.ClientRequestPayload;
import io.airlift.units.Duration;
import java.net.SocketAddress;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;
import org.junit.Assert;
import org.junit.Test;
import org.mockito.Mockito;
import reactor.core.publisher.Mono;
import reactor.test.StepVerifier;

public class TimeoutRpcClientFactoryTest {
  @Test(expected = NullPointerException.class)
  public void testThrowExceptionWhenDelegateFactoryIsNull() {
    new TimeoutRpcClientFactory(null, new ThriftClientConfig());
  }

  @Test(expected = NullPointerException.class)
  public void testThrowExceptionWhenConfigIsNull() {
    new TimeoutRpcClientFactory(Mockito.mock(RpcClientFactory.class), null);
  }

  @Test
  public void testValidateTimeout() {
    try {
      TimeoutRpcClientFactory.validateTimeout(1);
    } catch (IllegalArgumentException ignore) {
      fail();
    }

    try {
      TimeoutRpcClientFactory.validateTimeout(0);
      fail();
    } catch (IllegalArgumentException ignore) {
    }
  }

  @Test(expected = IllegalArgumentException.class)
  public void testThrowExceptionWhenDefaultTimeoutIsInvalid() {
    ThriftClientConfig config = new ThriftClientConfig();
    Duration duration = Duration.succinctDuration(0, TimeUnit.SECONDS);
    config.setRequestTimeout(duration);
    new TimeoutRpcClientFactory(Mockito.mock(RpcClientFactory.class), config);
  }

  @Test
  public void testTimeoutRpcClientShouldWrap() {
    SocketAddress address = Mockito.mock(SocketAddress.class);
    RpcClient delegateClient = Mockito.mock(RpcClient.class);
    RpcClientFactory delegate = Mockito.mock(RpcClientFactory.class);
    Mockito.when(delegate.createRpcClient(address)).thenReturn(Mono.just(delegateClient));

    TimeoutRpcClientFactory factory =
        new TimeoutRpcClientFactory(delegate, new ThriftClientConfig());
    RpcClient block = factory.createRpcClient(address).block();

    assertNotNull(block);
    assertTrue(block instanceof TimeoutRpcClientFactory.TimeoutRpcClient);
  }

  @Test
  @SuppressWarnings({"rawtypes", "unchecked"})
  public void testCalculateTime() {
    ClientRequestPayload payload = Mockito.mock(ClientRequestPayload.class);
    RpcOptions options = RpcOptions.EMPTY;
    SocketAddress address = Mockito.mock(SocketAddress.class);
    RpcClient delegateClient = Mockito.mock(RpcClient.class);
    Mockito.when(delegateClient.singleRequestSingleResponse(payload, options))
        .thenReturn(Mono.never());
    RpcClientFactory delegate = Mockito.mock(RpcClientFactory.class);
    Mockito.when(delegate.createRpcClient(address)).thenReturn(Mono.just(delegateClient));

    TimeoutRpcClientFactory factory =
        new TimeoutRpcClientFactory(
            delegate,
            new ThriftClientConfig()
                .setRequestTimeout(Duration.succinctDuration(1, TimeUnit.SECONDS)));

    TimeoutRpcClientFactory.TimeoutRpcClient rpcClient =
        (TimeoutRpcClientFactory.TimeoutRpcClient) factory.createRpcClient(address).block();
    long l = rpcClient.calculateRequestTimeout(RpcOptions.EMPTY);
    System.out.println(l);

    Assert.assertEquals(1_000, l);

    RpcOptions r = new RpcOptions.Builder().setClientTimeoutMs(1).setQueueTimeoutMs(0).build();

    l = rpcClient.calculateRequestTimeout(r);
    Assert.assertEquals(1, l);

    r = new RpcOptions.Builder().setClientTimeoutMs(0).setQueueTimeoutMs(1).build();

    l = rpcClient.calculateRequestTimeout(r);
    Assert.assertEquals(1, l);

    r = new RpcOptions.Builder().setClientTimeoutMs(1).setQueueTimeoutMs(1).build();

    l = rpcClient.calculateRequestTimeout(r);
    Assert.assertEquals(2, l);
  }

  @Test(timeout = 10_000)
  @SuppressWarnings({"rawtypes", "unchecked"})
  public void testTimeoutWithDefaultTimeout() {
    ClientRequestPayload payload = Mockito.mock(ClientRequestPayload.class);
    RpcOptions options = RpcOptions.EMPTY;
    SocketAddress address = Mockito.mock(SocketAddress.class);
    RpcClient delegateClient = Mockito.mock(RpcClient.class);
    Mockito.when(delegateClient.singleRequestSingleResponse(payload, options))
        .thenReturn(Mono.never());
    RpcClientFactory delegate = Mockito.mock(RpcClientFactory.class);
    Mockito.when(delegate.createRpcClient(address)).thenReturn(Mono.just(delegateClient));

    TimeoutRpcClientFactory factory =
        new TimeoutRpcClientFactory(
            delegate,
            new ThriftClientConfig()
                .setRequestTimeout(Duration.succinctDuration(1, TimeUnit.SECONDS)));

    StepVerifier.create(
            factory
                .createRpcClient(address)
                .flatMap(rpcClient -> rpcClient.singleRequestSingleResponse(payload, options)))
        .verifyError(TimeoutException.class);
  }

  @Test(timeout = 10_000)
  public void testConnectionTimeout() {
    SocketAddress address = Mockito.mock(SocketAddress.class);
    RpcClientFactory delegate = Mockito.mock(RpcClientFactory.class);
    Mockito.when(delegate.createRpcClient(address)).thenReturn(Mono.never());

    TimeoutRpcClientFactory factory =
        new TimeoutRpcClientFactory(
            delegate, new ThriftClientConfig().setConnectTimeout(Duration.valueOf("10ms")));

    StepVerifier.create(factory.createRpcClient(address)).verifyError(TimeoutException.class);
  }
}
