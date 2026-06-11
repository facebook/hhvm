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

package com.facebook.thrift.rsocket.server;

import static org.junit.jupiter.api.Assertions.assertEquals;

import com.facebook.swift.service.ThriftEventHandler;
import com.facebook.thrift.compression.CompressionManager;
import com.facebook.thrift.protocol.TProtocolType;
import com.facebook.thrift.rsocket.util.PayloadUtil;
import com.facebook.thrift.server.RpcServerHandler;
import com.facebook.thrift.test.rocket.TestRequest;
import com.facebook.thrift.test.rocket.TestResponse;
import com.facebook.thrift.test.rocket.TestService;
import com.facebook.thrift.util.resources.RpcResources;
import io.netty.buffer.ByteBuf;
import io.netty.buffer.ByteBufAllocator;
import io.netty.buffer.Unpooled;
import io.netty.util.ResourceLeakDetector;
import io.rsocket.Payload;
import java.util.Collections;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.stream.Stream;
import org.apache.thrift.CompressionAlgorithm;
import org.apache.thrift.ProtocolId;
import org.apache.thrift.RequestRpcMetadata;
import org.apache.thrift.RpcKind;
import org.apache.thrift.TApplicationException;
import org.apache.thrift.protocol.TField;
import org.apache.thrift.protocol.TProtocol;
import org.apache.thrift.protocol.TStruct;
import org.apache.thrift.protocol.TType;
import org.junit.jupiter.api.AfterAll;
import org.junit.jupiter.api.BeforeAll;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.params.ParameterizedTest;
import org.junit.jupiter.params.provider.Arguments;
import org.junit.jupiter.params.provider.MethodSource;
import reactor.test.StepVerifier;

/**
 * Locks down the eager request-buffer release on the RSocket transport ({@link
 * ThriftServerRSocket}), the analog of the header-transport contract covered by {@code
 * ServerRequestPayloadReleaseTest}.
 *
 * <p>The transport releases the RSocket {@link Payload} immediately after the decoded request
 * {@code ByteBuf} is extracted, and hands ONLY that decoded buffer to {@link
 * com.facebook.thrift.payload.ServerRequestPayload} as its owned reference. The server handler
 * releases it via {@code releaseRequestData()} right after the synchronous read (the generated
 * {@code _do<method>} for request-response/oneway, {@code StreamResponseHandlerTemplate} for
 * streams), and the transport's {@code doFinally} calls the same idempotent method as a backstop.
 * So the request buffer is freed as soon as the arguments are decoded instead of being pinned until
 * the response (or the whole stream) completes.
 *
 * <p>Request buffers are allocated with {@link Unpooled} so their {@code refCnt()} is a stable
 * release oracle: unlike pooled buffers and the recyclable {@code ByteBufPayload}, an unpooled
 * buffer's object is not recycled, so {@code refCnt() == 0} after completion proves it was freed.
 * Netty leak detection is forced to PARANOID for the whole class, so any missed release (leak) or
 * extra release (double-free throws {@code IllegalReferenceCountException}) in the exercised paths
 * is caught.
 */
public class ThriftServerRSocketReleaseTest {

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

  private final ByteBufAllocator alloc = RpcResources.getByteBufAllocator();

  private final RpcServerHandler handler =
      TestService.Reactive.serverHandlerBuilder(new TestServiceHandler()).build();

  private final ThriftServerRSocket rocket = new ThriftServerRSocket(handler, alloc);

  static Stream<Arguments> protocols() {
    return Stream.of(Arguments.of(ProtocolId.COMPACT), Arguments.of(ProtocolId.BINARY));
  }

  @ParameterizedTest
  @MethodSource("protocols")
  public void requestResponse_freesRequestBuffersAfterDecode(ProtocolId protocolId) {
    RequestRpcMetadata metadata =
        metadata(protocolId, "requestResponse", RpcKind.SINGLE_REQUEST_SINGLE_RESPONSE);
    ByteBuf data = createData(metadata, request(5, "foo"));
    ByteBuf meta = createMetadata(metadata);
    Payload payload = PayloadUtil.createPayload(data, meta);

    StepVerifier.create(rocket.requestResponse(payload))
        .assertNext(
            response -> {
              TestResponse result = readTestResponse(protocolId, response);
              assertEquals(5, result.getIntField());
              assertEquals("foo", result.getStrField());
              response.release();
            })
        .verifyComplete();

    assertEquals(0, data.refCnt(), "request data must be freed after decode");
    assertEquals(0, meta.refCnt(), "request metadata must be freed after decode");
  }

  @Test
  public void requestStream_freesRequestBufferEagerlyBeforeStreamCompletes() {
    RequestRpcMetadata metadata =
        metadata(ProtocolId.COMPACT, "streamResponse", RpcKind.SINGLE_REQUEST_STREAMING_RESPONSE);
    ByteBuf data = createData(metadata, request(0, "foo"));
    ByteBuf meta = createMetadata(metadata);
    Payload payload = PayloadUtil.createPayload(data, meta);

    StepVerifier.create(rocket.requestStream(payload))
        // The args are read (and the request buffer released) when the stream is subscribed, before
        // the first element is emitted -- the buffer is freed up front, not pinned for the whole
        // stream (10 data elements still follow).
        .assertNext(
            response -> {
              assertEquals(
                  0, data.refCnt(), "request data must be freed eagerly when the stream starts");
              response.release();
            })
        .thenConsumeWhile(p -> true, Payload::release)
        .verifyComplete();

    assertEquals(0, data.refCnt());
    assertEquals(0, meta.refCnt());
  }

  @Test
  public void requestResponse_compressedRequest_freesCompressedAndDecodedBuffers() {
    // Compressed path: the request frame on the wire is the ZLIB-compressed payload of the
    // serialized struct. ThriftServerRSocket.maybeDecompressRequestData decompresses it -- the
    // compressed slice is consumed inside CompressionManager.decompress and the returned
    // decompressed buffer is what ServerRequestPayload owns. The RSocket Payload itself is released
    // immediately after the decompress step. Both the compressed wire buffer (carried inside the
    // Payload) and the request metadata must reach refCnt() == 0 once the request completes.
    // (ZLIB is pure-Java via java.util.zip, no native FFI -- runs on Mac without libzstd/libz.)
    RequestRpcMetadata metadata =
        new RequestRpcMetadata.Builder()
            .setProtocol(ProtocolId.COMPACT)
            .setName("requestResponse")
            .setKind(RpcKind.SINGLE_REQUEST_SINGLE_RESPONSE)
            .setCompression(CompressionAlgorithm.ZLIB)
            .build();
    ByteBuf uncompressedData = createData(metadata, request(7, "compressed"));
    // Build the compressed wire buffer ourselves so we can hold a ref to it for the assertion.
    ByteBuf compressedData =
        CompressionManager.compress(CompressionAlgorithm.ZLIB, alloc, uncompressedData);
    ByteBuf meta = createMetadata(metadata);
    Payload payload = PayloadUtil.createPayload(compressedData, meta);

    StepVerifier.create(rocket.requestResponse(payload))
        .assertNext(
            response -> {
              TestResponse result = readTestResponse(ProtocolId.COMPACT, response);
              assertEquals(7, result.getIntField());
              assertEquals("compressed", result.getStrField());
              response.release();
            })
        .verifyComplete();

    assertEquals(0, compressedData.refCnt(), "compressed request data must be freed after decode");
    assertEquals(0, meta.refCnt(), "request metadata must be freed after decode");
  }

  // ---------------------------------------------------------------------------
  // Preprocess shed paths: the request buffer MUST be freed when a ThriftEventHandler.preprocess
  // throws (admission-control rejection) even though the generated dispatcher returns BEFORE
  // subscribing the inner Mono.defer. The transport's outer doFinally is the safety net.
  // ---------------------------------------------------------------------------

  @Test
  public void preprocessThrowsTApplicationException_freesBufferImmediately() {
    AtomicInteger handlerInvocations = new AtomicInteger();
    ThriftEventHandler sheddingHandler =
        new ThriftEventHandler() {
          @Override
          public void preprocess(Object context, String methodName) {
            handlerInvocations.incrementAndGet();
            throw new TApplicationException(TApplicationException.LOADSHEDDING, "shed");
          }
        };
    RpcServerHandler sheddingRpcHandler =
        TestService.Reactive.serverHandlerBuilder(new TestServiceHandler())
            .setEventHandlers(Collections.singletonList(sheddingHandler))
            .build();
    ThriftServerRSocket sheddingRocket = new ThriftServerRSocket(sheddingRpcHandler, alloc);

    RequestRpcMetadata metadata =
        metadata(ProtocolId.COMPACT, "requestResponse", RpcKind.SINGLE_REQUEST_SINGLE_RESPONSE);
    ByteBuf data = createData(metadata, request(5, "foo"));
    ByteBuf meta = createMetadata(metadata);
    Payload payload = PayloadUtil.createPayload(data, meta);

    // Sanity check: client receives a proper Thrift LOADSHEDDING error response, not a connection
    // drop, when the dispatcher returns Mono.just(errorResponse) from the preprocess catch.
    StepVerifier.create(sheddingRocket.requestResponse(payload))
        .assertNext(Payload::release)
        .verifyComplete();

    assertEquals(1, handlerInvocations.get(), "preprocess must fire exactly once");
    // The core invariant: even though preprocess sheds BEFORE the Mono.defer is ever subscribed,
    // the transport's outer doFinally still fires and ServerRequestPayload.releaseRequestData()
    // frees the decoded request buffer. The RSocket payload was already released at decode time.
    assertEquals(0, data.refCnt(), "request data must be freed on preprocess shed");
    assertEquals(0, meta.refCnt(), "request metadata must be freed on preprocess shed");
  }

  @Test
  public void preprocessThrowsRuntimeException_freesBufferOnErrorPath() {
    ThriftEventHandler crashingHandler =
        new ThriftEventHandler() {
          @Override
          public void preprocess(Object context, String methodName) {
            throw new RuntimeException("unexpected handler crash");
          }
        };
    RpcServerHandler crashingRpcHandler =
        TestService.Reactive.serverHandlerBuilder(new TestServiceHandler())
            .setEventHandlers(Collections.singletonList(crashingHandler))
            .build();
    ThriftServerRSocket crashingRocket = new ThriftServerRSocket(crashingRpcHandler, alloc);

    RequestRpcMetadata metadata =
        metadata(ProtocolId.COMPACT, "requestResponse", RpcKind.SINGLE_REQUEST_SINGLE_RESPONSE);
    ByteBuf data = createData(metadata, request(5, "foo"));
    ByteBuf meta = createMetadata(metadata);
    Payload payload = PayloadUtil.createPayload(data, meta);

    // Non-TApplicationException throws are wrapped via RpcPayloadUtil.internalErrorResponse so the
    // client receives a structured Thrift INTERNAL_ERROR instead of a generic RSocket
    // APPLICATION_ERROR. The buffer must still be freed via the transport's doFinally.
    StepVerifier.create(crashingRocket.requestResponse(payload))
        .assertNext(Payload::release)
        .verifyComplete();

    assertEquals(0, data.refCnt(), "request data must be freed on preprocess error");
    assertEquals(0, meta.refCnt(), "request metadata must be freed on preprocess error");
  }

  @Test
  public void preprocessShedOnStreaming_freesBufferImmediately() {
    ThriftEventHandler sheddingHandler =
        new ThriftEventHandler() {
          @Override
          public void preprocess(Object context, String methodName) {
            throw new TApplicationException(TApplicationException.LOADSHEDDING, "shed");
          }
        };
    RpcServerHandler sheddingRpcHandler =
        TestService.Reactive.serverHandlerBuilder(new TestServiceHandler())
            .setEventHandlers(Collections.singletonList(sheddingHandler))
            .build();
    ThriftServerRSocket sheddingRocket = new ThriftServerRSocket(sheddingRpcHandler, alloc);

    RequestRpcMetadata metadata =
        metadata(ProtocolId.COMPACT, "streamResponse", RpcKind.SINGLE_REQUEST_STREAMING_RESPONSE);
    ByteBuf data = createData(metadata, request(0, "foo"));
    ByteBuf meta = createMetadata(metadata);
    Payload payload = PayloadUtil.createPayload(data, meta);

    // Streaming shed: dispatcher returns Flux.just(errorResponse). The stream emits the error
    // payload and completes; the transport's doFinally releases the buffer immediately -- not
    // pinned for any stream lifetime.
    StepVerifier.create(sheddingRocket.requestStream(payload))
        .assertNext(Payload::release)
        .verifyComplete();

    assertEquals(0, data.refCnt(), "request data must be freed on streaming preprocess shed");
    assertEquals(0, meta.refCnt(), "request metadata must be freed on streaming preprocess shed");
  }

  // ---------------------------------------------------------------------------
  // helpers
  // ---------------------------------------------------------------------------

  private static RequestRpcMetadata metadata(ProtocolId protocolId, String name, RpcKind kind) {
    return new RequestRpcMetadata.Builder()
        .setProtocol(protocolId)
        .setName(name)
        .setKind(kind)
        .build();
  }

  private static TestRequest request(int intField, String strField) {
    return new TestRequest.Builder().setIntField(intField).setStrField(strField).build();
  }

  private ByteBuf createMetadata(RequestRpcMetadata metadata) {
    // Request metadata is always read with the compact protocol (see decodeRequestRpcMetadata).
    ByteBuf buf = Unpooled.buffer();
    TProtocol protocol = TProtocolType.fromProtocolId(ProtocolId.COMPACT).apply(buf);
    metadata.write0(protocol);
    return buf;
  }

  private ByteBuf createData(RequestRpcMetadata metadata, TestRequest request) {
    ByteBuf buf = Unpooled.buffer();
    TProtocol protocol = TProtocolType.fromProtocolId(metadata.getProtocol()).apply(buf);
    protocol.writeStructBegin(new TStruct());
    protocol.writeFieldBegin(new TField("struct", (byte) TType.STRUCT, (short) 1));
    request.write0(protocol);
    protocol.writeFieldEnd();
    protocol.writeFieldStop();
    protocol.writeStructEnd();
    return buf;
  }

  private TestResponse readTestResponse(ProtocolId protocolId, Payload response) {
    TProtocol protocol = TProtocolType.fromProtocolId(protocolId).apply(response.data());
    protocol.readFieldBegin();
    protocol.readStructBegin();
    TestResponse testResponse = TestResponse.read0(protocol);
    protocol.readStructEnd();
    return testResponse;
  }
}
