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

package com.facebook.swift.service;

import static org.assertj.core.api.Assertions.assertThat;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.when;

import com.facebook.nifty.core.RequestContext;
import com.facebook.thrift.example.ping.ExtendedPing;
import com.facebook.thrift.example.ping.ExtendedPingRpcServerHandler;
import com.facebook.thrift.example.ping.PingRequest;
import com.facebook.thrift.example.ping.PingResponse;
import com.facebook.thrift.example.ping.PingService;
import com.facebook.thrift.example.ping.PingServiceRpcServerHandler;
import com.facebook.thrift.payload.Reader;
import com.facebook.thrift.payload.ServerRequestPayload;
import com.facebook.thrift.payload.ServerResponsePayload;
import com.facebook.thrift.payload.Writer;
import com.facebook.thrift.protocol.ByteBufTProtocol;
import com.facebook.thrift.protocol.TProtocolType;
import com.facebook.thrift.util.NettyNiftyRequestContext;
import io.netty.buffer.ByteBuf;
import io.netty.buffer.Unpooled;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.function.Function;
import org.apache.thrift.ProtocolId;
import org.apache.thrift.RequestRpcMetadata;
import org.apache.thrift.RpcKind;
import org.apache.thrift.TApplicationException;
import org.apache.thrift.TException;
import org.junit.jupiter.api.Test;
import reactor.core.publisher.Mono;

/**
 * Locks down two invariants the generated dispatcher must hold:
 *
 * <ol>
 *   <li>{@code preprocess} fires exactly once per request (inheritance forwards to super before
 *       building the child's chain so the lifecycle does not double-fire).
 *   <li>{@code done()} fires exactly once on EVERY path -- normal completion, preprocess throws
 *       {@code TApplicationException} (shed), preprocess throws other {@code Throwable}, user
 *       handler throws.
 * </ol>
 */
public class InheritancePreprocessTest {

  /** Counts every lifecycle hook the dispatcher invokes on this handler. */
  private static final class Counting extends ThriftEventHandler {
    final AtomicInteger getContext = new AtomicInteger();
    final AtomicInteger preprocess = new AtomicInteger();
    final AtomicInteger preWriteException = new AtomicInteger();
    final AtomicInteger undeclaredUserException = new AtomicInteger();
    final AtomicInteger postWriteException = new AtomicInteger();
    final AtomicInteger done = new AtomicInteger();
    volatile TException preprocessThrow;

    @Override
    public Object getContext(String methodName, RequestContext requestContext) {
      getContext.incrementAndGet();
      return null;
    }

    @Override
    public void preprocess(Object context, String methodName) throws TException {
      preprocess.incrementAndGet();
      if (preprocessThrow != null) {
        throw preprocessThrow;
      }
    }

    @Override
    public void preWriteException(Object context, String methodName, Throwable t) {
      preWriteException.incrementAndGet();
    }

    @Override
    public void undeclaredUserException(Object o, String methodName, Throwable t) {
      undeclaredUserException.incrementAndGet();
    }

    @Override
    public void postWriteException(Object context, String methodName, Throwable t) {
      postWriteException.incrementAndGet();
    }

    @Override
    public void done(Object context, String methodName) {
      done.incrementAndGet();
    }
  }

  // ---------------------------------------------------------------------------
  // Inheritance: lifecycle fires once at the level that owns the method.
  // ---------------------------------------------------------------------------

  @Test
  public void inheritedMethod_lifecycleFiresOnceAtParent() {
    Counting handler = new Counting();
    ExtendedPing.Reactive delegate = mock(ExtendedPing.Reactive.class);
    when(delegate.ping(any()))
        .thenReturn(Mono.just(new PingResponse.Builder().setResponse("ok").build()));
    ExtendedPingRpcServerHandler rpc =
        new ExtendedPingRpcServerHandler(delegate, Collections.singletonList(handler));

    rpc.singleRequestSingleResponse(payload("ping")).block();

    assertThat(handler.getContext.get()).as("getContext fires once").isEqualTo(1);
    assertThat(handler.preprocess.get()).as("preprocess fires once").isEqualTo(1);
    assertThat(handler.done.get()).as("done fires once").isEqualTo(1);
  }

  @Test
  public void childOwnMethod_lifecycleFiresOnceAtChild() {
    Counting handler = new Counting();
    ExtendedPing.Reactive delegate = mock(ExtendedPing.Reactive.class);
    when(delegate.pingExtended(any())).thenReturn(Mono.empty());
    ExtendedPingRpcServerHandler rpc =
        new ExtendedPingRpcServerHandler(delegate, Collections.singletonList(handler));

    rpc.singleRequestSingleResponse(payload("pingExtended")).block();

    assertThat(handler.getContext.get()).isEqualTo(1);
    assertThat(handler.preprocess.get()).isEqualTo(1);
    assertThat(handler.done.get()).isEqualTo(1);
  }

  // ---------------------------------------------------------------------------
  // done() fires exactly once on EVERY path (the user's invariant).
  // ---------------------------------------------------------------------------

  @Test
  public void normalCompletion_doneFiresOnce() {
    Counting handler = new Counting();
    PingService.Reactive delegate = mock(PingService.Reactive.class);
    when(delegate.ping(any()))
        .thenReturn(Mono.just(new PingResponse.Builder().setResponse("ok").build()));
    PingServiceRpcServerHandler rpc =
        new PingServiceRpcServerHandler(delegate, Collections.singletonList(handler));

    rpc.singleRequestSingleResponse(payload("ping")).block();

    assertThat(handler.preprocess.get()).isEqualTo(1);
    assertThat(handler.done.get()).isEqualTo(1);
  }

  @Test
  public void preprocessThrowsTApplicationException_firesWriteHooksAndDone() {
    Counting handler = new Counting();
    handler.preprocessThrow = new TApplicationException(TApplicationException.LOADSHEDDING, "shed");
    PingService.Reactive delegate = mock(PingService.Reactive.class);
    PingServiceRpcServerHandler rpc =
        new PingServiceRpcServerHandler(delegate, Collections.singletonList(handler));

    ServerResponsePayload response = rpc.singleRequestSingleResponse(payload("ping")).block();
    assertThat(response).as("dispatcher returns an error response, not null").isNotNull();
    // The chain's preWriteException / undeclaredUserException / postWriteException fire when the
    // response is serialized -- simulate that here by invoking the writer.
    invokeWriter(response.getDataWriter());

    assertThat(handler.preprocess.get()).isEqualTo(1);
    assertThat(handler.preWriteException.get())
        .as("preWriteException must fire for TApplicationException shed")
        .isEqualTo(1);
    assertThat(handler.undeclaredUserException.get()).isEqualTo(1);
    assertThat(handler.postWriteException.get()).isEqualTo(1);
    assertThat(handler.done.get()).isEqualTo(1);
  }

  @Test
  public void preprocessThrowsThrowable_isWrappedAsInternalError() {
    Counting handler = new Counting();
    // Throw something that is NOT TApplicationException -- this used to become Mono.error and
    // bypass the Thrift error response. Now it should be wrapped as INTERNAL_ERROR.
    handler.preprocessThrow = new TException("preprocess crash");
    PingService.Reactive delegate = mock(PingService.Reactive.class);
    PingServiceRpcServerHandler rpc =
        new PingServiceRpcServerHandler(delegate, Collections.singletonList(handler));

    ServerResponsePayload response = rpc.singleRequestSingleResponse(payload("ping")).block();
    assertThat(response)
        .as("dispatcher emits a structured Thrift error response, not Mono.error")
        .isNotNull();
    invokeWriter(response.getDataWriter());

    assertThat(handler.preprocess.get()).isEqualTo(1);
    // Throwable path now goes through internalErrorResponse helper -> fromTApplicationException
    // with the chain, so the exception write hooks fire.
    assertThat(handler.preWriteException.get())
        .as("preWriteException must fire for Throwable shed (not just TApplicationException)")
        .isEqualTo(1);
    assertThat(handler.undeclaredUserException.get()).isEqualTo(1);
    assertThat(handler.postWriteException.get()).isEqualTo(1);
    assertThat(handler.done.get()).isEqualTo(1);
  }

  /** Simulates the transport serializing the response payload, which is what fires write hooks. */
  private static void invokeWriter(Writer writer) {
    ByteBuf buf = Unpooled.buffer();
    try {
      ByteBufTProtocol protocol = TProtocolType.fromProtocolId(ProtocolId.COMPACT).apply(buf);
      writer.write(protocol);
    } finally {
      buf.release();
    }
  }

  @Test
  public void chainConstructionFails_returnsInternalErrorResponse() {
    // A handler whose getContext throws causes ContextChain construction itself to fail. The
    // dispatcher must still send a structured Thrift INTERNAL_ERROR to the client rather than
    // bare Mono.error. No done() fires because no chain was built.
    ThriftEventHandler crashingHandler =
        new ThriftEventHandler() {
          @Override
          public Object getContext(String methodName, RequestContext requestContext) {
            throw new RuntimeException("getContext crash");
          }
        };
    PingService.Reactive delegate = mock(PingService.Reactive.class);
    PingServiceRpcServerHandler rpc =
        new PingServiceRpcServerHandler(delegate, Collections.singletonList(crashingHandler));

    ServerResponsePayload response = rpc.singleRequestSingleResponse(payload("ping")).block();

    assertThat(response)
        .as("dispatcher returns a structured INTERNAL_ERROR response, not Mono.error")
        .isNotNull();
    // Sanity: the response writer encodes successfully without a chain (the helper passes null,
    // which is the safe path through fromTApplicationException).
    invokeWriter(response.getDataWriter());
  }

  @Test
  public void userHandlerThrows_doneFiresOnce() {
    Counting handler = new Counting();
    PingService.Reactive delegate = mock(PingService.Reactive.class);
    when(delegate.ping(any())).thenReturn(Mono.error(new RuntimeException("handler boom")));
    PingServiceRpcServerHandler rpc =
        new PingServiceRpcServerHandler(delegate, Collections.singletonList(handler));

    rpc.singleRequestSingleResponse(payload("ping")).block();

    assertThat(handler.preprocess.get()).isEqualTo(1);
    assertThat(handler.done.get()).as("done MUST fire even when user handler throws").isEqualTo(1);
  }

  // ---------------------------------------------------------------------------
  // helpers
  // ---------------------------------------------------------------------------

  private static ServerRequestPayload payload(String method) {
    Function<List<Reader>, List<Object>> readerTransformer =
        readers -> Arrays.asList(new PingRequest.Builder().setRequest("hi").build());
    return ServerRequestPayload.create(
        readerTransformer,
        new RequestRpcMetadata.Builder()
            .setName(method)
            .setKind(RpcKind.SINGLE_REQUEST_SINGLE_RESPONSE)
            .build(),
        new NettyNiftyRequestContext(null, null),
        1);
  }
}
