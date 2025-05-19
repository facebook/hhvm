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

package com.facebook.thrift.loadbalancing;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNull;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoInteractions;
import static org.mockito.Mockito.when;
import static org.mockito.MockitoAnnotations.openMocks;

import com.facebook.nifty.core.RequestContext;
import com.facebook.thrift.payload.ServerRequestPayload;
import com.facebook.thrift.payload.ServerResponsePayload;
import com.facebook.thrift.server.RpcServerHandler;
import com.facebook.thrift.util.NettyNiftyRequestContext;
import com.google.common.collect.ImmutableMap;
import java.util.function.Function;
import org.apache.commons.lang3.RandomUtils;
import org.apache.thrift.ResponseRpcMetadata;
import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.mockito.Mock;
import reactor.core.publisher.Mono;
import reactor.test.StepVerifier;

public class LoadHeaderRpcServerHandlerTest {
  @Mock RpcServerHandler rpcServerHandler;
  @Mock Function<String, Long> counterProvider;
  @Mock LoadHeaderSupplier loadSupplier;
  @Mock ServerResponsePayload serverResponsePayload;

  private static final String CUSTOM_COUNTER = "foo.bar";

  private static final RequestContext REQUEST_CTX_NO_HEADER =
      new NettyNiftyRequestContext(ImmutableMap.of(), null);

  private static final ServerRequestPayload PAYLOAD_NO_HEADER =
      ServerRequestPayload.create(null, null, REQUEST_CTX_NO_HEADER, 1);

  private static final RequestContext REQUEST_CTX_DEFAULT_HEADER =
      new NettyNiftyRequestContext(ImmutableMap.of("load", "default"), null);

  private static final ServerRequestPayload PAYLOAD_DEFAULT_HEADER =
      ServerRequestPayload.create(null, null, REQUEST_CTX_DEFAULT_HEADER, 1);

  private static final RequestContext REQUEST_CTX_CUSTOM_HEADER =
      new NettyNiftyRequestContext(ImmutableMap.of("load", CUSTOM_COUNTER), null);

  private static final ServerRequestPayload PAYLOAD_CUSTOM_HEADER =
      ServerRequestPayload.create(null, null, REQUEST_CTX_CUSTOM_HEADER, 1);

  private AutoCloseable closeable;
  private LoadHeaderRpcServerHandler loadHeaderHandler;

  @Before
  public void setUp() throws Exception {
    closeable = openMocks(this);

    loadHeaderHandler =
        new LoadHeaderRpcServerHandler(rpcServerHandler, counterProvider, loadSupplier);

    when(serverResponsePayload.getResponseRpcMetadata())
        .thenReturn(ResponseRpcMetadata.defaultInstance());

    when(rpcServerHandler.singleRequestSingleResponse(any(ServerRequestPayload.class)))
        .thenReturn(Mono.just(serverResponsePayload));
  }

  @After
  public void tearDown() throws Exception {
    closeable.close();
  }

  @Test
  public void testNoLoadHeader() {
    ServerResponsePayload payload =
        loadHeaderHandler.singleRequestSingleResponse(PAYLOAD_NO_HEADER).block();

    assertNull(payload.getResponseRpcMetadata().getLoad());

    verifyNoInteractions(loadSupplier);
    verifyNoInteractions(counterProvider);
  }

  @Test
  public void test_ReturnsDefaultLoad_WhenDefaultSupplied() {
    Long expectedLoad = RandomUtils.nextLong();
    when(loadSupplier.getLoad()).thenReturn(expectedLoad);

    ServerResponsePayload payload =
        loadHeaderHandler.singleRequestSingleResponse(PAYLOAD_DEFAULT_HEADER).block();

    assertEquals(payload.getResponseRpcMetadata().getLoad(), expectedLoad);
    assertEquals(
        payload.getResponseRpcMetadata().getOtherMetadata().get("load"),
        String.valueOf(expectedLoad));
  }

  @Test
  public void test_ReturnsCustomCounter_WhenCustomStringSupplied() {
    Long expectedLoad = RandomUtils.nextLong();
    when(counterProvider.apply(eq(CUSTOM_COUNTER))).thenReturn(expectedLoad);

    ServerResponsePayload payload =
        loadHeaderHandler.singleRequestSingleResponse(PAYLOAD_CUSTOM_HEADER).block();

    assertEquals(payload.getResponseRpcMetadata().getLoad(), expectedLoad);
    assertEquals(
        payload.getResponseRpcMetadata().getOtherMetadata().get("load"),
        String.valueOf(expectedLoad));
  }

  @Test
  public void test_LoadSupplierIsCalled_WhenDefaultIsApplied() {
    Long expectedLoad = RandomUtils.nextLong();
    when(loadSupplier.getLoad()).thenReturn(expectedLoad);

    ServerResponsePayload payload =
        loadHeaderHandler.singleRequestSingleResponse(PAYLOAD_DEFAULT_HEADER).block();

    assertEquals(payload.getResponseRpcMetadata().getLoad(), expectedLoad);
    assertEquals(
        payload.getResponseRpcMetadata().getOtherMetadata().get("load"),
        String.valueOf(expectedLoad));

    verify(loadSupplier, times(1)).onRequest();
    verify(loadSupplier, times(1)).getLoad();
    verify(loadSupplier, times(1)).doFinally();

    verifyNoInteractions(counterProvider);
  }

  @Test
  public void test_CounterProviderIsCalled_WhenCustomLoadHeaderSupplier() {
    Long expectedLoad = RandomUtils.nextLong();
    when(counterProvider.apply(eq(CUSTOM_COUNTER))).thenReturn(expectedLoad);

    ServerResponsePayload payload =
        loadHeaderHandler.singleRequestSingleResponse(PAYLOAD_CUSTOM_HEADER).block();

    assertEquals(payload.getResponseRpcMetadata().getLoad(), expectedLoad);
    assertEquals(
        payload.getResponseRpcMetadata().getOtherMetadata().get("load"),
        String.valueOf(expectedLoad));

    verify(counterProvider, times(1)).apply(eq(CUSTOM_COUNTER));

    verifyNoInteractions(loadSupplier);
  }

  @Test
  public void test_LoadSupplierIsCalled_WhenExceptionIsThrown() {
    Long expectedLoad = RandomUtils.nextLong();
    when(loadSupplier.getLoad()).thenReturn(expectedLoad);

    when(rpcServerHandler.singleRequestSingleResponse(any(ServerRequestPayload.class)))
        .thenReturn(Mono.error(new RuntimeException("BOOM")));

    StepVerifier.create(loadHeaderHandler.singleRequestSingleResponse(PAYLOAD_DEFAULT_HEADER))
        .expectError()
        .verify();

    verify(loadSupplier, times(1)).onRequest();
    verify(loadSupplier, times(0)).getLoad();
    verify(loadSupplier, times(1)).doFinally();

    verifyNoInteractions(counterProvider);
  }

  @Test
  public void test_CounterProviderIsCalled_WhenExceptionIsThrown() {
    Long expectedLoad = RandomUtils.nextLong();
    when(counterProvider.apply(eq(CUSTOM_COUNTER))).thenReturn(expectedLoad);

    when(rpcServerHandler.singleRequestSingleResponse(any(ServerRequestPayload.class)))
        .thenReturn(Mono.error(new RuntimeException("BOOM")));

    StepVerifier.create(loadHeaderHandler.singleRequestSingleResponse(PAYLOAD_CUSTOM_HEADER))
        .expectError()
        .verify();

    verify(counterProvider, times(0)).apply(eq(CUSTOM_COUNTER));

    verifyNoInteractions(loadSupplier);
  }
}
