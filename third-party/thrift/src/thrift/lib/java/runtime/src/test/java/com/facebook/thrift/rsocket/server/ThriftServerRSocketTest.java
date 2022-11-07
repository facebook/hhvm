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

import static org.junit.Assert.*;

import com.facebook.thrift.protocol.TProtocolType;
import com.facebook.thrift.rsocket.util.PayloadUtil;
import com.facebook.thrift.server.RpcServerHandler;
import com.facebook.thrift.test.rocket.InitialTestResponse;
import com.facebook.thrift.test.rocket.TestException;
import com.facebook.thrift.test.rocket.TestFunctionException;
import com.facebook.thrift.test.rocket.TestRequest;
import com.facebook.thrift.test.rocket.TestRequest2;
import com.facebook.thrift.test.rocket.TestResponse;
import com.facebook.thrift.test.rocket.TestService;
import com.facebook.thrift.util.resources.RpcResources;
import io.netty.buffer.ByteBuf;
import io.netty.buffer.ByteBufAllocator;
import io.rsocket.Payload;
import java.util.Arrays;
import java.util.Collection;
import java.util.concurrent.atomic.AtomicInteger;
import org.apache.thrift.ErrorBlame;
import org.apache.thrift.PayloadExceptionMetadataBase;
import org.apache.thrift.PayloadResponseMetadata;
import org.apache.thrift.ProtocolId;
import org.apache.thrift.RequestRpcMetadata;
import org.apache.thrift.ResponseRpcMetadata;
import org.apache.thrift.RpcKind;
import org.apache.thrift.StreamPayloadMetadata;
import org.apache.thrift.protocol.TField;
import org.apache.thrift.protocol.TProtocol;
import org.apache.thrift.protocol.TStruct;
import org.apache.thrift.protocol.TType;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.Parameterized;
import reactor.test.StepVerifier;

@RunWith(Parameterized.class)
public class ThriftServerRSocketTest {

  @Parameterized.Parameters
  public static Collection<Object> data() {
    return Arrays.asList(ProtocolId.COMPACT, ProtocolId.BINARY);
  }

  private final ProtocolId protocolId;

  public ThriftServerRSocketTest(ProtocolId protocolId) {
    this.protocolId = protocolId;
  }

  ByteBufAllocator alloc = RpcResources.getByteBufAllocator();

  RpcServerHandler handler =
      TestService.Reactive.serverHandlerBuilder(new TestServiceHandler()).build();

  ThriftServerRSocket rocket = new ThriftServerRSocket(handler, alloc);

  private ByteBuf createMetadata(RequestRpcMetadata metadata) {
    ByteBuf buf = alloc.buffer();
    TProtocol protocol = TProtocolType.fromProtocolId(ProtocolId.COMPACT).apply(buf);
    metadata.write0(protocol);
    buf.retain();

    return buf;
  }

  private ByteBuf createData(RequestRpcMetadata metadata, TestRequest request) {
    ByteBuf buf = alloc.buffer();
    TProtocol protocol = TProtocolType.fromProtocolId(metadata.getProtocol()).apply(buf);
    protocol.writeStructBegin(new TStruct());
    protocol.writeFieldBegin(new TField("struct", (byte) TType.STRUCT, (short) 1));
    request.write0(protocol);
    protocol.writeFieldEnd();
    protocol.writeFieldStop();
    protocol.writeStructEnd();
    buf.retain();

    return buf;
  }

  private ByteBuf createData(
      RequestRpcMetadata metadata, TestRequest request, TestRequest2 request2) {
    ByteBuf buf = alloc.buffer();
    TProtocol protocol = TProtocolType.fromProtocolId(metadata.getProtocol()).apply(buf);
    protocol.writeStructBegin(new TStruct());

    protocol.writeFieldBegin(new TField("struct", (byte) TType.STRUCT, (short) 2));
    request.write0(protocol);
    protocol.writeFieldEnd();

    protocol.writeFieldBegin(new TField("struct", (byte) TType.STRUCT, (short) 5));
    request2.write0(protocol);
    protocol.writeFieldEnd();

    protocol.writeFieldStop();
    protocol.writeStructEnd();
    buf.retain();

    return buf;
  }

  private TestResponse getTestResponse(RequestRpcMetadata metadata, Payload response) {
    return getTestResponse(metadata.getProtocol(), response);
  }

  private TestResponse getTestResponse(ProtocolId protocolId, Payload response) {
    TProtocol protocol = TProtocolType.fromProtocolId(protocolId).apply(response.data());
    TField field = protocol.readFieldBegin();
    assertEquals(0, field.id);
    assertEquals(TType.STRUCT, field.type);
    protocol.readStructBegin();
    TestResponse testResponse = TestResponse.read0(protocol);
    protocol.readStructEnd();

    return testResponse;
  }

  private InitialTestResponse getInitialTestResponse(ProtocolId protocolId, Payload response) {
    TProtocol protocol = TProtocolType.fromProtocolId(protocolId).apply(response.data());
    TField field = protocol.readFieldBegin();
    assertEquals(0, field.id);
    assertEquals(TType.STRUCT, field.type);
    protocol.readStructBegin();
    InitialTestResponse initialResponse = InitialTestResponse.read0(protocol);
    protocol.readStructEnd();

    return initialResponse;
  }

  private TestRequest createTestRequest(int intField, String strField) {
    return new TestRequest.Builder().setIntField(intField).setStrField(strField).build();
  }

  private ResponseRpcMetadata getResponseMetadata(Payload response) {
    TProtocol protocol =
        TProtocolType.fromProtocolId(ProtocolId.COMPACT).apply(response.metadata());
    return ResponseRpcMetadata.read0(protocol);
  }

  private void assertDataIsEmpty(Payload response) {
    assertEquals(1, response.data().readableBytes());
    assertEquals(0, response.data().readByte());
  }

  private void assertEmptyResponse(Payload response) {
    assertDataIsEmpty(response);
  }

  private StreamPayloadMetadata getStreamMetadata(Payload response) {
    TProtocol protocol =
        TProtocolType.fromProtocolId(ProtocolId.COMPACT).apply(response.metadata());
    return StreamPayloadMetadata.read0(protocol);
  }

  private RequestRpcMetadata createRequestRpcMetadata(String name, RpcKind rpc) {
    return new RequestRpcMetadata.Builder()
        .setProtocol(protocolId)
        .setName(name)
        .setKind(rpc)
        .build();
  }

  private void assertDataIsTestException(ProtocolId protocolId, Payload payload, int id) {
    TProtocol protocol = TProtocolType.fromProtocolId(protocolId).apply(payload.data());
    TField field = protocol.readFieldBegin();
    assertEquals(id, field.id);
    assertEquals(TType.STRUCT, field.type);
    protocol.readStructBegin();
    TestException exc = TestException.read0(protocol);
    protocol.readStructEnd();
    assertEquals("exc", exc.getMsg());
  }

  private void assertDataIsFunctionException(ProtocolId protocolId, Payload payload, int id) {
    TProtocol protocol = TProtocolType.fromProtocolId(protocolId).apply(payload.data());
    TField field = protocol.readFieldBegin();
    assertEquals(id, field.id);
    assertEquals(TType.STRUCT, field.type);
    protocol.readStructBegin();
    TestFunctionException exc = TestFunctionException.read0(protocol);
    protocol.readStructEnd();
    assertEquals("exc", exc.getMsg());
  }

  private Payload createPayload(RequestRpcMetadata requestMetadata, int intField, String strField) {
    TestRequest input = createTestRequest(intField, strField);
    return PayloadUtil.createPayload(
        createData(requestMetadata, input), createMetadata(requestMetadata));
  }

  private void assertRpcMetadataIsDefault(Payload response) {
    ResponseRpcMetadata responseMetadata = getResponseMetadata(response);
    assertEquals(
        PayloadResponseMetadata.defaultInstance(),
        responseMetadata.getPayloadMetadata().getResponseMetadata());
  }

  private void assertStreamMetadataIsDefault(Payload response) {
    StreamPayloadMetadata streamMetadata = getStreamMetadata(response);
    assertEquals(
        PayloadResponseMetadata.defaultInstance(),
        streamMetadata.getPayloadMetadata().getResponseMetadata());
  }

  @Test
  public void testRequestResponse() {
    RequestRpcMetadata requestMetadata =
        createRequestRpcMetadata("requestResponse", RpcKind.SINGLE_REQUEST_SINGLE_RESPONSE);
    Payload request = createPayload(requestMetadata, 5, "foo");

    StepVerifier.create(rocket.requestResponse(request))
        .assertNext(
            response -> {
              assertRpcMetadataIsDefault(response);
              TestResponse result = getTestResponse(requestMetadata, response);
              assertEquals(5, result.getIntField());
              assertEquals("foo", result.getStrField());
            })
        .verifyComplete();
  }

  @Test
  public void testRequestResponseVoid() {
    RequestRpcMetadata requestMetadata =
        createRequestRpcMetadata("requestResponseVoid", RpcKind.SINGLE_REQUEST_SINGLE_RESPONSE);
    Payload request = createPayload(requestMetadata, 5, "foo");

    StepVerifier.create(rocket.requestResponse(request))
        .assertNext(
            response -> {
              assertRpcMetadataIsDefault(response);
              assertEquals(5, TestServiceHandler.inputParameter.getIntField());
              assertEquals("foo", TestServiceHandler.inputParameter.getStrField());
              assertDataIsEmpty(response);
            })
        .verifyComplete();
  }

  @Test
  public void testStreamBasic() {
    RequestRpcMetadata requestMetadata =
        createRequestRpcMetadata("streamResponse", RpcKind.SINGLE_REQUEST_STREAMING_RESPONSE);
    Payload request = createPayload(requestMetadata, 0, "foo");

    StepVerifier.Step<Payload> steps =
        StepVerifier.create(rocket.requestStream(request))
            .assertNext(
                response -> {
                  assertRpcMetadataIsDefault(response);
                  assertDataIsEmpty(response);
                });

    final AtomicInteger counter = new AtomicInteger();
    for (int i = 0; i < 10; i++) {
      steps.assertNext(
          response -> {
            assertStreamMetadataIsDefault(response);

            TestResponse result = getTestResponse(requestMetadata, response);
            assertEquals(counter.get(), result.getIntField());
            assertEquals("foo" + counter.getAndIncrement(), result.getStrField());
          });
    }
    steps.verifyComplete();
  }

  @Test
  public void testNullStream() {
    RequestRpcMetadata requestMetadata =
        createRequestRpcMetadata("streamResponseNull", RpcKind.SINGLE_REQUEST_STREAMING_RESPONSE);
    Payload request = createPayload(requestMetadata, 0, "foo");

    StepVerifier.create(rocket.requestStream(request))
        .assertNext(
            response -> {
              ResponseRpcMetadata responseMetadata = getResponseMetadata(response);
              assertEquals(
                  ErrorBlame.SERVER,
                  responseMetadata
                      .getPayloadMetadata()
                      .getExceptionMetadata()
                      .getMetadata()
                      .getAppUnknownException()
                      .getErrorClassification()
                      .getBlame());
            })
        .verifyComplete();
  }

  @Test
  public void testEmptyStream() {
    RequestRpcMetadata requestMetadata =
        createRequestRpcMetadata("streamResponseEmpty", RpcKind.SINGLE_REQUEST_STREAMING_RESPONSE);
    Payload request = createPayload(requestMetadata, 0, "foo");

    StepVerifier.create(rocket.requestStream(request))
        .assertNext(
            response -> {
              assertRpcMetadataIsDefault(response);
              assertDataIsEmpty(response);
            })
        .verifyComplete();
  }

  @Test
  public void testStreamInitialResponse() {
    RequestRpcMetadata requestMetadata =
        createRequestRpcMetadata(
            "streamInitialResponse", RpcKind.SINGLE_REQUEST_STREAMING_RESPONSE);
    Payload request = createPayload(requestMetadata, 0, "foo");

    StepVerifier.Step<Payload> steps =
        StepVerifier.create(rocket.requestStream(request))
            .assertNext(
                response -> {
                  assertRpcMetadataIsDefault(response);
                  InitialTestResponse initialResponse =
                      getInitialTestResponse(requestMetadata.getProtocol(), response);
                  assertEquals(100, initialResponse.getIntField());
                });

    final AtomicInteger counter = new AtomicInteger();
    for (int i = 0; i < 10; i++) {
      steps.assertNext(
          response -> {
            assertStreamMetadataIsDefault(response);

            TestResponse result = getTestResponse(requestMetadata, response);
            assertEquals(counter.get(), result.getIntField());
            assertEquals("foo" + counter.getAndIncrement(), result.getStrField());
          });
    }
    steps.verifyComplete();
  }

  private void checkUndeclaredExceptionNoStream(String funcName, String excName, String msg) {
    RequestRpcMetadata requestMetadata =
        createRequestRpcMetadata(funcName, RpcKind.SINGLE_REQUEST_STREAMING_RESPONSE);
    Payload request = createPayload(requestMetadata, 0, "foo");

    StepVerifier.create(rocket.requestStream(request))
        .assertNext(
            response -> {
              ResponseRpcMetadata responseMetadata = getResponseMetadata(response);
              PayloadExceptionMetadataBase expMetadata =
                  responseMetadata.getPayloadMetadata().getExceptionMetadata();
              assertEquals(
                  ErrorBlame.SERVER,
                  expMetadata
                      .getMetadata()
                      .getAppUnknownException()
                      .getErrorClassification()
                      .getBlame());
              assertEquals(excName, expMetadata.getNameUtf8());
              assertEquals(msg, expMetadata.getWhatUtf8());
            })
        .verifyComplete();
  }

  private void checkDeclaredException(String funcName) {
    RequestRpcMetadata requestMetadata =
        createRequestRpcMetadata(funcName, RpcKind.SINGLE_REQUEST_STREAMING_RESPONSE);
    Payload request = createPayload(requestMetadata, 0, "foo");

    StepVerifier.create(rocket.requestStream(request))
        .assertNext(
            response -> {
              assertRpcMetadataIsDefault(response);
              assertEmptyResponse(response);
            })
        .assertNext(
            response -> {
              assertStreamMedatadataHasDeclaredException(response);
              assertDataIsTestException(requestMetadata.getProtocol(), response, 1);
            })
        .verifyComplete();
  }

  private void checkDeclaredExceptionNoStream(String funcName) {
    RequestRpcMetadata requestMetadata =
        createRequestRpcMetadata(funcName, RpcKind.SINGLE_REQUEST_STREAMING_RESPONSE);
    Payload request = createPayload(requestMetadata, 0, "foo");

    StepVerifier.create(rocket.requestStream(request))
        .assertNext(
            response -> {
              assertRpcMedatadataHasDeclaredException(response);
              assertDataIsTestException(requestMetadata.getProtocol(), response, 2);
            })
        .verifyComplete();
  }

  @Test
  public void testStreamDeclaredException() {
    checkDeclaredException("streamDeclaredException");
  }

  @Test
  public void testStreamDeclaredException2() {
    checkDeclaredExceptionNoStream("streamDeclaredException2");
  }

  @Test
  public void testDeclaredFunctionException() {
    RequestRpcMetadata requestMetadata =
        createRequestRpcMetadata(
            "streamDeclaredAndFunctionException", RpcKind.SINGLE_REQUEST_STREAMING_RESPONSE);
    Payload request = createPayload(requestMetadata, 0, "foo");

    StepVerifier.create(rocket.requestStream(request))
        .assertNext(
            response -> {
              assertRpcMetadataIsDefault(response);
              assertEmptyResponse(response);
            })
        .assertNext(
            response -> {
              assertStreamMedatadataHasDeclaredException(response);
              assertDataIsFunctionException(requestMetadata.getProtocol(), response, 6);
            })
        .verifyComplete();
  }

  private void checkUndeclaredException(String funcName, String excName, String msg) {
    RequestRpcMetadata requestMetadata =
        createRequestRpcMetadata(funcName, RpcKind.SINGLE_REQUEST_STREAMING_RESPONSE);
    Payload request = createPayload(requestMetadata, 0, "foo");

    StepVerifier.create(rocket.requestStream(request))
        .assertNext(
            response -> {
              assertRpcMetadataIsDefault(response);
              assertEmptyResponse(response);
            })
        .assertNext(
            response -> {
              StreamPayloadMetadata streamMetadata = getStreamMetadata(response);
              PayloadExceptionMetadataBase expMetadata =
                  streamMetadata.getPayloadMetadata().getExceptionMetadata();
              assertEquals(
                  ErrorBlame.SERVER,
                  expMetadata
                      .getMetadata()
                      .getAppUnknownException()
                      .getErrorClassification()
                      .getBlame());
              assertEquals(excName, expMetadata.getNameUtf8());
              assertEquals(msg, expMetadata.getWhatUtf8());
            })
        .verifyComplete();
  }

  private void checkUndeclaredException(String funcName) {
    checkUndeclaredException(funcName, IllegalArgumentException.class.getName(), "exc");
  }

  @Test
  public void testStreamUndeclaredException() {
    checkUndeclaredException("streamUndeclaredException");
  }

  @Test
  public void testStreamUndeclaredException2() {
    checkUndeclaredExceptionNoStream(
        "streamUndeclaredException2", IllegalArgumentException.class.getName(), "exc");
  }

  @Test
  public void testUndeclaredFunctionException() {
    checkUndeclaredException("streamUndeclaredAndFunctionException");
  }

  @Test
  public void testInitialResponseDeclaredException() {
    RequestRpcMetadata requestMetadata =
        createRequestRpcMetadata(
            "streamInitialResponseDeclaredException", RpcKind.SINGLE_REQUEST_STREAMING_RESPONSE);
    Payload request = createPayload(requestMetadata, 0, "foo");

    StepVerifier.create(rocket.requestStream(request))
        .assertNext(
            response -> {
              assertRpcMedatadataHasDeclaredException(response);
            })
        .verifyComplete();
  }

  private void assertRpcMedatadataHasDeclaredException(Payload payload) {
    ResponseRpcMetadata responseMetadata = getResponseMetadata(payload);
    assertEquals(
        ErrorBlame.SERVER,
        responseMetadata
            .getPayloadMetadata()
            .getExceptionMetadata()
            .getMetadata()
            .getDeclaredException()
            .getErrorClassification()
            .getBlame());
  }

  private void assertStreamMedatadataHasDeclaredException(Payload payload) {
    StreamPayloadMetadata responseMetadata = getStreamMetadata(payload);
    assertEquals(
        ErrorBlame.SERVER,
        responseMetadata
            .getPayloadMetadata()
            .getExceptionMetadata()
            .getMetadata()
            .getDeclaredException()
            .getErrorClassification()
            .getBlame());
  }

  @Test
  public void testStreamDeclaredExceptionFluxError() {
    RequestRpcMetadata requestMetadata =
        createRequestRpcMetadata(
            "streamDeclaredExceptionFluxError", RpcKind.SINGLE_REQUEST_STREAMING_RESPONSE);
    Payload request = createPayload(requestMetadata, 0, "foo");

    StepVerifier.create(rocket.requestStream(request))
        .expectNextCount(5)
        .assertNext(
            response -> {
              assertStreamMedatadataHasDeclaredException(response);
              assertDataIsTestException(requestMetadata.getProtocol(), response, 7);
            })
        .verifyComplete();
  }

  @Test
  public void testStreamMultiInput() {
    RequestRpcMetadata requestMetadata =
        createRequestRpcMetadata(
            "streamDeclaredExceptionMultiInput", RpcKind.SINGLE_REQUEST_STREAMING_RESPONSE);
    TestRequest input = createTestRequest(3, "foo");
    TestRequest2 input2 =
        new TestRequest2.Builder().setShortField((short) 7).setStrField("bar").build();
    Payload request =
        PayloadUtil.createPayload(
            createData(requestMetadata, input, input2), createMetadata(requestMetadata));

    StepVerifier.create(rocket.requestStream(request))
        .assertNext(
            response -> {
              // initial response
              assertRpcMetadataIsDefault(response);
              assertDataIsEmpty(response);
            })
        .assertNext(
            response -> {
              // stream responses
              assertStreamMetadataIsDefault(response);

              TestResponse result = getTestResponse(requestMetadata, response);
              assertEquals(10, result.getIntField());
              assertEquals("foobar7", result.getStrField());
            })
        .verifyComplete();
  }
}
