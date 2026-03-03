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

package com.facebook.thrift.util;

import static com.facebook.thrift.util.NettyUtil.TransportType.EPOLL;
import static com.facebook.thrift.util.NettyUtil.TransportType.IO_URING;
import static com.facebook.thrift.util.NettyUtil.TransportType.NIO;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;
import static org.junit.Assume.assumeFalse;
import static org.junit.Assume.assumeTrue;
import static org.mockito.Mockito.mockStatic;

import com.facebook.thrift.client.ThriftClientConfig;
import com.facebook.thrift.example.ping.CustomException;
import com.facebook.thrift.example.ping.PingResponse;
import com.facebook.thrift.model.StreamResponse;
import com.facebook.thrift.payload.ClientRequestPayload;
import com.facebook.thrift.payload.ClientResponsePayload;
import com.facebook.thrift.payload.Reader;
import com.facebook.thrift.protocol.ByteBufTProtocol;
import com.facebook.thrift.protocol.TProtocolType;
import com.facebook.thrift.util.resources.RpcResources;
import com.google.common.collect.ImmutableMap;
import io.netty.buffer.Unpooled;
import io.netty.channel.epoll.EpollDomainSocketChannel;
import io.netty.channel.epoll.EpollSocketChannel;
import io.netty.channel.kqueue.KQueueDomainSocketChannel;
import io.netty.channel.kqueue.KQueueSocketChannel;
import io.netty.channel.socket.nio.NioSocketChannel;
import io.netty.channel.unix.DomainSocketAddress;
import io.netty.channel.uring.IoUringDomainSocketChannel;
import io.netty.channel.uring.IoUringSocketChannel;
import io.netty.handler.ssl.SslContext;
import io.netty.util.internal.PlatformDependent;
import io.rsocket.util.DefaultPayload;
import java.net.InetSocketAddress;
import java.net.SocketAddress;
import java.util.Collections;
import java.util.Map;
import java.util.Optional;
import org.apache.thrift.PayloadAppUnknownExceptionMetdata;
import org.apache.thrift.PayloadDeclaredExceptionMetadata;
import org.apache.thrift.PayloadExceptionMetadata;
import org.apache.thrift.PayloadExceptionMetadataBase;
import org.apache.thrift.PayloadMetadata;
import org.apache.thrift.PayloadProxyExceptionMetadata;
import org.apache.thrift.ProtocolId;
import org.apache.thrift.RequestRpcMetadata;
import org.apache.thrift.ResponseRpcMetadata;
import org.apache.thrift.StreamPayloadMetadata;
import org.apache.thrift.TApplicationException;
import org.apache.thrift.TException;
import org.apache.thrift.protocol.TField;
import org.apache.thrift.protocol.TStruct;
import org.apache.thrift.protocol.TType;
import org.apache.thrift.transport.TTransportException;
import org.junit.Test;
import org.mockito.MockedStatic;
import reactor.core.Exceptions;

public class RpcClientUtilsTest {

  private final Reader<PingResponse> reader =
      (oprot) -> {
        try {
          return PingResponse.read0(oprot);
        } catch (Throwable ex) {
          throw Exceptions.propagate(ex);
        }
      };

  private final Reader<CustomException> exReader =
      (oprot) -> {
        try {
          return CustomException.read0(oprot);
        } catch (Throwable ex) {
          throw Exceptions.propagate(ex);
        }
      };

  private final Map<Short, Reader> exceptionMap = ImmutableMap.of((short) 1, exReader);
  private final ClientRequestPayload<PingResponse> requestPayload =
      ClientRequestPayload.create(
          "test",
          null,
          reader,
          reader,
          exceptionMap,
          exceptionMap,
          new RequestRpcMetadata.Builder().setProtocol(ProtocolId.BINARY).build(),
          Collections.emptyMap());

  @Test
  public void testGetEventLoopGroup() {
    assertNotNull("EventLoopGroup should not be null", RpcResources.getEventLoopGroup());
  }

  @Test
  public void testDomainSocketChannelLinuxJava9() {
    assumeFalse("Not Macos", isMacos());
    assumeTrue("Java 9+", PlatformDependent.javaVersion() >= 9);

    try (MockedStatic<NettyUtil> mockNettyUtil = mockStatic(NettyUtil.class)) {
      mockNettyUtil.when(NettyUtil::getTransportType).thenReturn(IO_URING);

      SocketAddress socketAddress = new DomainSocketAddress("/foo");
      Class<?> channelClass = RpcClientUtils.getChannelClass(socketAddress);
      assertEquals(IoUringDomainSocketChannel.class, channelClass);
    }
  }

  @Test
  public void testDomainSocketChannelLinux() {
    assumeFalse("Not Macos", isMacos());

    try (MockedStatic<NettyUtil> mockNettyUtil = mockStatic(NettyUtil.class)) {
      mockNettyUtil.when(NettyUtil::getTransportType).thenReturn(EPOLL);

      SocketAddress socketAddress = new DomainSocketAddress("/foo");
      Class<?> channelClass = RpcClientUtils.getChannelClass(socketAddress);
      assertEquals(EpollDomainSocketChannel.class, channelClass);
    }
  }

  @Test
  public void testEpollSocketChannel() {
    assumeFalse(isMacos());

    try (MockedStatic<NettyUtil> mockNettyUtil = mockStatic(NettyUtil.class)) {
      mockNettyUtil.when(NettyUtil::getTransportType).thenReturn(EPOLL);

      SocketAddress socketAddress = new InetSocketAddress(0);
      Class<?> channelClass = RpcClientUtils.getChannelClass(socketAddress);
      assertEquals(EpollSocketChannel.class, channelClass);
    }
  }

  @Test
  public void testIoUringSocketChannel() {
    assumeFalse(isMacos());
    assumeTrue("Java 9+", PlatformDependent.javaVersion() >= 9);

    try (MockedStatic<NettyUtil> mockNettyUtil = mockStatic(NettyUtil.class)) {
      mockNettyUtil.when(NettyUtil::getTransportType).thenReturn(IO_URING);

      SocketAddress socketAddress = new InetSocketAddress(0);
      Class<?> channelClass = RpcClientUtils.getChannelClass(socketAddress);
      assertEquals(IoUringSocketChannel.class, channelClass);
    }
  }

  @Test
  public void testDomainSocketChannelMacos() {
    if (isMacos()) {
      SocketAddress socketAddress = new DomainSocketAddress("/foo");
      Class<?> channelClass = RpcClientUtils.getChannelClass(socketAddress);
      assertEquals(KQueueDomainSocketChannel.class, channelClass);
    }
  }

  @Test
  public void testKQueueSocketChannel() {
    if (isMacos()) {
      SocketAddress socketAddress = new InetSocketAddress(0);
      Class<?> channelClass = RpcClientUtils.getChannelClass(socketAddress);
      assertEquals(KQueueSocketChannel.class, channelClass);
    }
  }

  @Test
  public void testNioSocketChannel() {
    try (MockedStatic<NettyUtil> mockNettyUtil = mockStatic(NettyUtil.class)) {
      mockNettyUtil.when(NettyUtil::getTransportType).thenReturn(NIO);

      SocketAddress socketAddress = new InetSocketAddress(0);
      Class<?> channelClass = RpcClientUtils.getChannelClass(socketAddress);
      assertEquals(NioSocketChannel.class, channelClass);
    }
  }

  @Test(expected = UnsupportedOperationException.class)
  public void testInvalidSocketGroupCombination() {
    assumeTrue(PlatformDependent.javaVersion() < 16);

    try (MockedStatic<NettyUtil> mockNettyUtil = mockStatic(NettyUtil.class)) {
      mockNettyUtil.when(NettyUtil::getTransportType).thenReturn(NIO);

      SocketAddress socketAddress = new DomainSocketAddress("/foo");
      RpcClientUtils.getChannelClass(socketAddress);
    }
  }

  @Test
  public void tesGetSslContextDomain() {
    SslContext context =
        RpcClientUtils.getSslContext(new ThriftClientConfig(), new DomainSocketAddress("/foo"));
    assertNull(context);
  }

  @Test
  public void tesDisabledSsl() {
    SslContext context =
        RpcClientUtils.getSslContext(
            new ThriftClientConfig().setDisableSSL(true), new InetSocketAddress(0));
    assertNull(context);
  }

  @Test
  public void tesSslContext() {
    SslContext context =
        RpcClientUtils.getSslContext(new ThriftClientConfig(), new InetSocketAddress(0));
    assertTrue(context.isClient());
  }

  @Test
  public void tesJdkSslContext() {
    SslContext context =
        RpcClientUtils.getSslContext(
            new ThriftClientConfig().setEnableJdkSsl(true), new InetSocketAddress(0));
    assertTrue(context.isClient());
  }

  @Test
  public void tesDecodeStreamPayload() {
    ByteBufTProtocol protocol = TProtocolType.TBinary.apply(Unpooled.buffer());
    protocol.writeStructBegin(new TStruct());
    protocol.writeFieldBegin(new TField("pingResponse", TType.STRUCT, (short) 0));
    PingResponse pingResponse = new PingResponse.Builder().setResponse("foo").build();
    pingResponse.write0(protocol);
    protocol.writeFieldEnd();
    protocol.writeStructEnd();

    StreamResponse<PingResponse, PingResponse> actual =
        (StreamResponse)
            RpcClientUtils.decodeStreamPayload(
                    requestPayload,
                    DefaultPayload.create(protocol.getByteBuf()),
                    StreamPayloadMetadata.defaultInstance(),
                    ResponseRpcMetadata.defaultInstance(),
                    false)
                .getData();
    assertEquals("foo", actual.getData().getResponse());
  }

  @Test
  public void tesDecodeFirstStreamPayload() {
    ByteBufTProtocol protocol = TProtocolType.TBinary.apply(Unpooled.buffer());
    protocol.writeStructBegin(new TStruct());
    protocol.writeFieldBegin(new TField("pingResponse", TType.STRUCT, (short) 0));
    PingResponse pingResponse = new PingResponse.Builder().setResponse("foo").build();
    pingResponse.write0(protocol);
    protocol.writeFieldEnd();
    protocol.writeStructEnd();

    StreamResponse<PingResponse, PingResponse> actual =
        (StreamResponse)
            RpcClientUtils.decodeStreamPayload(
                    requestPayload,
                    DefaultPayload.create(protocol.getByteBuf()),
                    StreamPayloadMetadata.defaultInstance(),
                    ResponseRpcMetadata.defaultInstance(),
                    true)
                .getData();
    assertEquals("foo", actual.getFirstResponse().getResponse());
  }

  @Test
  public void testVoidFirstResponse() {
    ByteBufTProtocol protocol = TProtocolType.TBinary.apply(Unpooled.buffer());
    protocol.writeStructBegin(new TStruct());
    protocol.writeFieldBegin(new TField("pingResponse", TType.STOP, (short) 0));
    PingResponse pingResponse = new PingResponse.Builder().setResponse("foo").build();
    pingResponse.write0(protocol);
    protocol.writeFieldEnd();
    protocol.writeStructEnd();

    StreamResponse actual =
        (StreamResponse)
            RpcClientUtils.decodeStreamPayload(
                    requestPayload,
                    DefaultPayload.create(protocol.getByteBuf()),
                    StreamPayloadMetadata.defaultInstance(),
                    ResponseRpcMetadata.defaultInstance(),
                    true)
                .getData();
    assertTrue(actual.isSetFirstResponse());
  }

  @Test
  public void testExceptionResponse() {

    ByteBufTProtocol protocol = TProtocolType.TBinary.apply(Unpooled.buffer());
    protocol.writeStructBegin(new TStruct());
    protocol.writeFieldBegin(new TField("ex", TType.STRUCT, (short) 1));
    CustomException exception = new CustomException.Builder().setMessage("foo").build();
    exception.write0(protocol);
    protocol.writeFieldEnd();
    protocol.writeStructEnd();

    ClientResponsePayload actual =
        RpcClientUtils.decodeStreamPayload(
            requestPayload,
            DefaultPayload.create(protocol.getByteBuf()),
            StreamPayloadMetadata.defaultInstance(),
            ResponseRpcMetadata.defaultInstance(),
            false);
    assertNotNull(actual.getException());
  }

  @Test
  public void testUndefinedExceptioStringResponse() {
    ClientRequestPayload<PingResponse> requestPayload =
        ClientRequestPayload.create(
            "test",
            null,
            reader,
            reader,
            Collections.emptyMap(),
            Collections.emptyMap(),
            new RequestRpcMetadata.Builder().setProtocol(ProtocolId.BINARY).build(),
            Collections.emptyMap());

    ByteBufTProtocol protocol = TProtocolType.TBinary.apply(Unpooled.buffer());
    protocol.writeStructBegin(new TStruct());
    protocol.writeFieldBegin(new TField("ex", TType.STRING, (short) 1));
    protocol.writeString("internal_error");
    protocol.writeFieldEnd();
    protocol.writeStructEnd();

    ClientResponsePayload actual =
        RpcClientUtils.decodeStreamPayload(
            requestPayload,
            DefaultPayload.create(protocol.getByteBuf()),
            StreamPayloadMetadata.defaultInstance(),
            ResponseRpcMetadata.defaultInstance(),
            false);
    assertEquals("internal_error", actual.getException().getMessage());
  }

  @Test
  public void testUndefinedExceptionResponse() {
    ClientRequestPayload<PingResponse> requestPayload =
        ClientRequestPayload.create(
            "test",
            null,
            reader,
            reader,
            Collections.emptyMap(),
            Collections.emptyMap(),
            new RequestRpcMetadata.Builder().setProtocol(ProtocolId.BINARY).build(),
            Collections.emptyMap());

    ClientResponsePayload actual =
        RpcClientUtils.decodeStreamPayload(
            requestPayload,
            DefaultPayload.create("Undefined exception"),
            StreamPayloadMetadata.defaultInstance(),
            ResponseRpcMetadata.defaultInstance(),
            false);
    assertEquals("Undefined exception", actual.getException().getMessage());
  }

  @Test
  public void tesDecodeSinglePayload() {
    ByteBufTProtocol protocol = TProtocolType.TBinary.apply(Unpooled.buffer());
    protocol.writeStructBegin(new TStruct());
    protocol.writeFieldBegin(new TField("pingResponse", TType.STRUCT, (short) 0));
    PingResponse pingResponse = new PingResponse.Builder().setResponse("foo").build();
    pingResponse.write0(protocol);
    protocol.writeFieldEnd();
    protocol.writeStructEnd();

    PingResponse actual =
        RpcClientUtils.decodeRSocketPayload(
                reader, Collections.emptyMap(), protocol, new ResponseRpcMetadata.Builder().build())
            .getData();
    assertEquals("foo", actual.getResponse());
  }

  @Test
  public void testHasUndeclaredException() {
    ResponseRpcMetadata rpcMetadata =
        new ResponseRpcMetadata.Builder()
            .setPayloadMetadata(
                PayloadMetadata.fromExceptionMetadata(
                    new PayloadExceptionMetadataBase.Builder()
                        .setMetadata(
                            PayloadExceptionMetadata.fromAppUnknownException(
                                PayloadAppUnknownExceptionMetdata.defaultInstance()))
                        .build()))
            .build();
    Optional<? extends TException> undeclaredException =
        RpcClientUtils.getUndeclaredException(rpcMetadata);
    assertTrue(undeclaredException.isPresent());
    assertFalse(undeclaredException.get() instanceof TTransportException);
  }

  @Test
  public void testUndeclaredExceptionStackTrace1() {
    ResponseRpcMetadata rpcMetadata =
        new ResponseRpcMetadata.Builder()
            .setPayloadMetadata(
                PayloadMetadata.fromExceptionMetadata(
                    new PayloadExceptionMetadataBase.Builder()
                        .setWhatUtf8("Foo")
                        .setMetadata(
                            PayloadExceptionMetadata.fromAppUnknownException(
                                PayloadAppUnknownExceptionMetdata.defaultInstance()))
                        .build()))
            .build();
    TException undeclaredException = RpcClientUtils.getUndeclaredException(rpcMetadata).get();
    assertNotNull(undeclaredException);
    assertTrue(undeclaredException instanceof TApplicationException);
    assertEquals("Foo", undeclaredException.getMessage());
    assertEquals(
        "org.apache.thrift.TApplicationException",
        undeclaredException.getStackTrace()[0].getClassName());
  }

  @Test
  public void testUndeclaredExceptionStackTrace2() {
    ResponseRpcMetadata rpcMetadata =
        new ResponseRpcMetadata.Builder()
            .setPayloadMetadata(
                PayloadMetadata.fromExceptionMetadata(
                    new PayloadExceptionMetadataBase.Builder()
                        .setWhatUtf8("Foo")
                        .setMetadata(
                            PayloadExceptionMetadata.fromDEPRECATEDProxyException(
                                PayloadProxyExceptionMetadata.defaultInstance()))
                        .build()))
            .build();
    TException undeclaredException = RpcClientUtils.getUndeclaredException(rpcMetadata).get();
    assertNotNull(undeclaredException);
    assertTrue(undeclaredException instanceof TTransportException);
    assertEquals("Foo", undeclaredException.getMessage());
    assertEquals(
        "com.facebook.thrift.util.RpcClientUtils",
        undeclaredException.getStackTrace()[0].getClassName());
  }

  @Test
  public void testHasDeclaredException() {
    ResponseRpcMetadata rpcMetadata =
        new ResponseRpcMetadata.Builder()
            .setPayloadMetadata(
                PayloadMetadata.fromExceptionMetadata(
                    new PayloadExceptionMetadataBase.Builder()
                        .setMetadata(
                            PayloadExceptionMetadata.fromDeclaredException(
                                PayloadDeclaredExceptionMetadata.defaultInstance()))
                        .build()))
            .build();
    Optional<? extends TException> undeclaredException =
        RpcClientUtils.getUndeclaredException(rpcMetadata);
    assertFalse(undeclaredException.isPresent());
  }

  private static boolean isMacos() {
    return System.getProperty("os.name").startsWith("Mac");
  }
}
