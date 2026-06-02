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

package com.facebook.thrift.payload;

import static org.assertj.core.api.Assertions.assertThat;
import static org.assertj.core.api.Assertions.assertThatThrownBy;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.when;

import com.facebook.thrift.example.ping.ExtendedPing;
import com.facebook.thrift.example.ping.ExtendedPingRpcServerHandler;
import com.facebook.thrift.example.ping.PingRequest;
import com.facebook.thrift.example.ping.PingResponse;
import com.facebook.thrift.example.ping.PingService;
import com.facebook.thrift.example.ping.PingServiceRpcServerHandler;
import com.facebook.thrift.server.RpcServerHandler;
import com.facebook.thrift.util.NettyNiftyRequestContext;
import io.netty.buffer.ByteBuf;
import io.netty.buffer.ByteBufAllocator;
import io.netty.buffer.Unpooled;
import io.netty.util.IllegalReferenceCountException;
import io.netty.util.ResourceLeakDetector;
import java.util.Collections;
import java.util.List;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.CountDownLatch;
import java.util.function.Function;
import org.apache.thrift.RequestRpcMetadata;
import org.apache.thrift.RpcKind;
import org.junit.jupiter.api.AfterAll;
import org.junit.jupiter.api.BeforeAll;
import org.junit.jupiter.api.Test;
import reactor.core.publisher.Mono;

/**
 * Locks down the request-buffer ownership contract that lets header transports release the request
 * frame right after the arguments are read (instead of at response completion).
 *
 * <p>The buffer is owned by {@link ServerRequestPayload}; the generated server handler releases it
 * via {@link ServerRequestPayload#releaseRequestData()} from inside each {@code _do<method>}
 * immediately after the synchronous read, and the transport calls the same idempotent method as a
 * backstop for paths where no generated {@code _do} runs.
 *
 * <p>Netty leak detection is forced to PARANOID for the whole class so any missed release in the
 * exercised paths is caught.
 */
public class ServerRequestPayloadReleaseTest {

  private static ResourceLeakDetector.Level previousLeakLevel;

  @BeforeAll
  static void enableParanoidLeakDetection() {
    previousLeakLevel = ResourceLeakDetector.getLevel();
    ResourceLeakDetector.setLevel(ResourceLeakDetector.Level.PARANOID);
  }

  @AfterAll
  static void restoreLeakDetection() {
    ResourceLeakDetector.setLevel(previousLeakLevel);
  }

  private static RequestRpcMetadata metadata(String name) {
    return new RequestRpcMetadata.Builder()
        .setName(name)
        .setKind(RpcKind.SINGLE_REQUEST_SINGLE_RESPONSE)
        .build();
  }

  // ---------------------------------------------------------------------------
  // Payload-level ownership semantics
  // ---------------------------------------------------------------------------

  @Test
  public void releaseRequestData_releasesOwnedBufferExactlyOnce() {
    ByteBuf buf = Unpooled.buffer().writeInt(42);
    assertThat(buf.refCnt()).isEqualTo(1);

    ServerRequestPayload payload =
        ServerRequestPayload.create(
            readers -> Collections.emptyList(), metadata("m"), null, 1, buf);

    payload.releaseRequestData();
    assertThat(buf.refCnt()).isEqualTo(0);

    // Idempotent: the transport backstop calls releaseRequestData() again after the generated
    // handler already released. This must not decrement again or throw.
    payload.releaseRequestData();
    assertThat(buf.refCnt()).isEqualTo(0);
  }

  @Test
  public void releaseRequestData_withNoOwnedBuffer_isNoop() {
    // RSocket-style payload: the transport owns the request buffer, the payload carries none.
    ServerRequestPayload payload =
        ServerRequestPayload.create(readers -> Collections.emptyList(), metadata("m"), null);
    payload.releaseRequestData(); // must not throw
  }

  @Test
  public void concurrentRelease_releasesExactlyOnce() throws Exception {
    ByteBuf buf = Unpooled.buffer().writeInt(1);
    buf.retain(); // refCnt = 2 so a stray second decrement would be observable (drop to 0)
    assertThat(buf.refCnt()).isEqualTo(2);

    ServerRequestPayload payload =
        ServerRequestPayload.create(
            readers -> Collections.emptyList(), metadata("m"), null, 1, buf);

    int threads = 8;
    CountDownLatch start = new CountDownLatch(1);
    CompletableFuture<?>[] futures = new CompletableFuture[threads];
    for (int i = 0; i < threads; i++) {
      futures[i] =
          CompletableFuture.runAsync(
              () -> {
                try {
                  start.await();
                } catch (InterruptedException e) {
                  throw new RuntimeException(e);
                }
                payload.releaseRequestData();
              });
    }
    start.countDown();
    CompletableFuture.allOf(futures).join();

    // Exactly one logical release happened despite N concurrent callers: refCnt 2 -> 1.
    assertThat(buf.refCnt()).isEqualTo(1);
    buf.release();
  }

  /**
   * Regression guard for the "no zero-copy" precondition. Eager release after the read is only safe
   * because every materialized object is independent of the request buffer's lifetime -- production
   * {@code ByteBuf} TypeAdapters copy (e.g. {@code UnpooledByteBufTypeAdapter}), and the {@code
   * Any} deserializer copies too. A copy survives the request buffer's release; a non-retaining
   * slice (a hypothetical zero-copy adapter that forgot to retain) dangles. If a future adapter
   * hands back such a slice, release-after-read would corrupt it -- this pins the contract those
   * adapters must satisfy.
   */
  @Test
  public void nettyInvariant_copySurvivesParentRelease_bareSliceDoesNot() {
    ByteBuf parent = Unpooled.buffer().writeInt(123);

    ByteBuf copy = Unpooled.copiedBuffer(parent); // what a copying TypeAdapter produces
    ByteBuf bareSlice = parent.slice(); // a zero-copy adapter that forgot to retain

    parent.release(); // the framework releases the request frame after the read

    assertThat(copy.refCnt()).isEqualTo(1);
    assertThat(copy.getInt(0)).isEqualTo(123); // independent: still valid
    copy.release();

    assertThatThrownBy(() -> bareSlice.getInt(0)) // dangling read fails fast
        .isInstanceOf(IllegalReferenceCountException.class);
  }

  // ---------------------------------------------------------------------------
  // End-to-end through real generated handlers (standard + inherited dispatch)
  // ---------------------------------------------------------------------------

  /**
   * Instrumented request payload over a real (leak-tracked) buffer. The reader function records the
   * buffer's refCnt at the moment the generated handler actually reads, so we can assert the buffer
   * is still alive during the read and released exactly once afterward.
   */
  private static final class Probe {
    final ByteBuf frame = ByteBufAllocator.DEFAULT.buffer().writeInt(1);
    volatile boolean readInvoked = false;
    volatile int refCntDuringRead = -1;

    ServerRequestPayload payload(String method, Object... args) {
      Function<List<Reader>, List<Object>> readerTransformer =
          readers -> {
            readInvoked = true;
            refCntDuringRead = frame.refCnt();
            return java.util.Arrays.asList(args);
          };
      return ServerRequestPayload.create(
          readerTransformer, metadata(method), new NettyNiftyRequestContext(null, null), 1, frame);
    }
  }

  private static PingRequest pingRequest() {
    return new PingRequest.Builder().setRequest("hi").build();
  }

  @Test
  public void standardMethod_releasesFrameAfterRead() {
    PingService.Reactive delegate = mock(PingService.Reactive.class);
    when(delegate.ping(any()))
        .thenReturn(Mono.just(new PingResponse.Builder().setResponse("ok").build()));
    RpcServerHandler handler = new PingServiceRpcServerHandler(delegate, Collections.emptyList());

    Probe probe = new Probe();
    handler.singleRequestSingleResponse(probe.payload("ping", pingRequest())).block();

    assertThat(probe.readInvoked).isTrue();
    assertThat(probe.refCntDuringRead).isGreaterThan(0); // frame alive during the read
    assertThat(probe.frame.refCnt()).isEqualTo(0); // released exactly once after read
  }

  /**
   * The regression this change is about. A request for an inherited (parent) method enters the
   * child handler, whose dispatcher does not read -- it forwards a deferred Mono to {@code super}.
   * The release must therefore happen in the parent's {@code _do<method>} (after the parent reads),
   * not in the child dispatcher. With the earlier dispatcher-level release this test fails:
   * refCntDuringRead would be 0 because the child freed the frame before the parent read it.
   */
  @Test
  public void inheritedMethod_releasesFrameAfterParentReads() {
    ExtendedPing.Reactive delegate = mock(ExtendedPing.Reactive.class);
    when(delegate.ping(any()))
        .thenReturn(Mono.just(new PingResponse.Builder().setResponse("ok").build()));
    RpcServerHandler handler = new ExtendedPingRpcServerHandler(delegate, Collections.emptyList());

    Probe probe = new Probe();
    // "ping" is defined on the parent (PingService); the child forwards it to super.
    handler.singleRequestSingleResponse(probe.payload("ping", pingRequest())).block();

    assertThat(probe.readInvoked).isTrue();
    assertThat(probe.refCntDuringRead).isGreaterThan(0); // alive when the parent reads
    assertThat(probe.frame.refCnt()).isEqualTo(0); // released exactly once after read
  }

  @Test
  public void childOwnMethod_releasesFrameAfterRead() {
    ExtendedPing.Reactive delegate = mock(ExtendedPing.Reactive.class);
    when(delegate.pingExtended(any())).thenReturn(Mono.empty());
    RpcServerHandler handler = new ExtendedPingRpcServerHandler(delegate, Collections.emptyList());

    Probe probe = new Probe();
    // "pingExtended" is handled directly by the child.
    handler.singleRequestSingleResponse(probe.payload("pingExtended", pingRequest())).block();

    assertThat(probe.readInvoked).isTrue();
    assertThat(probe.refCntDuringRead).isGreaterThan(0);
    assertThat(probe.frame.refCnt()).isEqualTo(0);
  }

  @Test
  public void unknownMethod_doesNotReadAndLeavesReleaseToBackstop() {
    // No _do runs for an unknown method, so the generated handler never reads or releases; the
    // transport backstop is responsible. Here (no transport) we release manually and confirm the
    // handler did not read or release the frame itself.
    PingService.Reactive delegate = mock(PingService.Reactive.class);
    RpcServerHandler handler = new PingServiceRpcServerHandler(delegate, Collections.emptyList());

    Probe probe = new Probe();
    handler.singleRequestSingleResponse(probe.payload("nonexistent", pingRequest())).block();

    assertThat(probe.readInvoked).isFalse();
    assertThat(probe.frame.refCnt()).isEqualTo(1); // untouched by codegen; backstop would release
    probe.frame.release();
  }
}
