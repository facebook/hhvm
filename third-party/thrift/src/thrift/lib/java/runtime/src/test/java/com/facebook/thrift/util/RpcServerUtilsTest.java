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
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;
import static org.junit.Assume.assumeFalse;
import static org.junit.Assume.assumeTrue;
import static org.mockito.Mockito.mockStatic;

import com.facebook.nifty.core.RequestContext;
import com.facebook.nifty.ssl.SslSession;
import com.facebook.swift.service.SwiftConstants;
import com.facebook.swift.service.ThriftServerConfig;
import com.facebook.thrift.legacy.server.ThriftOptionalSslHandler;
import com.facebook.thrift.util.resources.RpcResources;
import com.google.common.collect.ImmutableMap;
import io.netty.channel.epoll.EpollServerDomainSocketChannel;
import io.netty.channel.epoll.EpollServerSocketChannel;
import io.netty.channel.kqueue.KQueueServerDomainSocketChannel;
import io.netty.channel.kqueue.KQueueServerSocketChannel;
import io.netty.channel.socket.nio.NioServerSocketChannel;
import io.netty.channel.unix.DomainSocketAddress;
import io.netty.channel.uring.IoUringServerDomainSocketChannel;
import io.netty.channel.uring.IoUringServerSocketChannel;
import io.netty.handler.ssl.SslContext;
import io.netty.util.AttributeKey;
import io.netty.util.internal.PlatformDependent;
import java.net.InetSocketAddress;
import java.net.SocketAddress;
import org.junit.Assert;
import org.junit.Test;
import org.mockito.MockedStatic;
import org.mockito.Mockito;
import reactor.core.publisher.Flux;
import reactor.core.publisher.Mono;
import reactor.test.StepVerifier;

public class RpcServerUtilsTest {
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
      Class<?> channelClass = RpcServerUtils.getChannelClass(socketAddress);
      assertEquals(IoUringServerDomainSocketChannel.class, channelClass);
    }
  }

  @Test
  public void testDomainSocketChannelLinux() {
    assumeFalse("Not Macos", isMacos());

    try (MockedStatic<NettyUtil> mockNettyUtil = mockStatic(NettyUtil.class)) {
      mockNettyUtil.when(NettyUtil::getTransportType).thenReturn(EPOLL);

      SocketAddress socketAddress = new DomainSocketAddress("/foo");
      Class<?> channelClass = RpcServerUtils.getChannelClass(socketAddress);
      assertEquals(EpollServerDomainSocketChannel.class, channelClass);
    }
  }

  @Test
  public void testEpollSocketChannel() {
    assumeFalse(isMacos());

    try (MockedStatic<NettyUtil> mockNettyUtil = mockStatic(NettyUtil.class)) {
      mockNettyUtil.when(NettyUtil::getTransportType).thenReturn(EPOLL);

      SocketAddress socketAddress = new InetSocketAddress(0);
      Class<?> channelClass = RpcServerUtils.getChannelClass(socketAddress);
      assertEquals(EpollServerSocketChannel.class, channelClass);
    }
  }

  @Test
  public void testIoUringSocketChannel() {
    assumeFalse(isMacos());
    assumeTrue("Java 9+", PlatformDependent.javaVersion() >= 9);

    try (MockedStatic<NettyUtil> mockNettyUtil = mockStatic(NettyUtil.class)) {
      mockNettyUtil.when(NettyUtil::getTransportType).thenReturn(IO_URING);

      SocketAddress socketAddress = new InetSocketAddress(0);
      Class<?> channelClass = RpcServerUtils.getChannelClass(socketAddress);
      assertEquals(IoUringServerSocketChannel.class, channelClass);
    }
  }

  @Test
  public void testDomainSocketChannelMacos() {
    if (!isMacos()) {
      return;
    }

    assumeTrue(isMacos());
    SocketAddress socketAddress = new DomainSocketAddress("/foo");
    Class<?> channelClass = RpcServerUtils.getChannelClass(socketAddress);
    assertEquals(KQueueServerDomainSocketChannel.class, channelClass);
  }

  @Test
  public void testKQueueSocketChannel() {
    if (!isMacos()) {
      return;
    }

    assumeTrue(isMacos());
    SocketAddress socketAddress = new InetSocketAddress(0);
    Class<?> channelClass = RpcServerUtils.getChannelClass(socketAddress);
    assertEquals(KQueueServerSocketChannel.class, channelClass);
  }

  @Test
  public void testNioSocketChannel() {
    try (MockedStatic<NettyUtil> mockNettyUtil = mockStatic(NettyUtil.class)) {
      mockNettyUtil.when(NettyUtil::getTransportType).thenReturn(NIO);

      SocketAddress socketAddress = new InetSocketAddress(0);
      Class<?> channelClass = RpcServerUtils.getChannelClass(socketAddress);
      assertEquals(NioServerSocketChannel.class, channelClass);
    }
  }

  @Test(expected = UnsupportedOperationException.class)
  public void testInvalidSocketGroupCombination() {
    assumeTrue(PlatformDependent.javaVersion() < 16);

    try (MockedStatic<NettyUtil> mockNettyUtil = mockStatic(NettyUtil.class)) {
      mockNettyUtil.when(NettyUtil::getTransportType).thenReturn(NIO);

      SocketAddress socketAddress = new DomainSocketAddress("/foo");
      RpcServerUtils.getChannelClass(socketAddress);
    }
  }

  // TODO(yuhanhao) need NettyTcNativeLoader
  @Test
  public void tesSslContext() {
    SslContext context = RpcServerUtils.getSslContext(new ThriftServerConfig());
    assertTrue(context.isServer());
  }

  @Test
  public void tesJdkSslContext() {
    SslContext context =
        RpcServerUtils.getSslContext(new ThriftServerConfig().setEnableJdkSsl(true));
    assertTrue(context.isServer());
  }

  @Test
  public void testSslAttribute() {
    SslContext context =
        RpcServerUtils.getSslContext(new ThriftServerConfig().setEnableJdkSsl(true));
    // This will create sslSession
    ThriftOptionalSslHandler optionalSslHandler = new ThriftOptionalSslHandler(context);
    // This should re-use sslSession
    AttributeKey<SslSession> sslSessionAttributeKey = SwiftConstants.THRIFT_SSL_SESSION_KEY;
  }

  @Test
  public void testDecorateMonoWithRequestContext() {
    RequestContext mock = Mockito.mock(RequestContext.class);
    Mockito.when(mock.getRequestHeader()).thenReturn(ImmutableMap.of("hello", "world"));
    Mono<Long> count =
        Flux.range(0, 100)
            .count()
            .handle(
                (aLong, longSynchronousSink) -> {
                  RequestContext context =
                      RequestContext.fromContextView(longSynchronousSink.contextView());
                  Assert.assertEquals("world", context.getRequestHeader().get("hello"));
                  longSynchronousSink.next(aLong);
                });
    Mono<Long> longMono = RpcServerUtils.decorateWithRequestContext(mock, count);
    StepVerifier.create(longMono).expectNextCount(1).verifyComplete();
  }

  @Test
  public void testDecorateFluxWithRequestContext() {
    RequestContext mock = Mockito.mock(RequestContext.class);
    Mockito.when(mock.getRequestHeader()).thenReturn(ImmutableMap.of("hello", "world"));
    Flux<Object> handle =
        Flux.range(0, 100)
            .handle(
                (aLong, longSynchronousSink) -> {
                  RequestContext context =
                      RequestContext.fromContextView(longSynchronousSink.contextView());
                  assertEquals("world", context.getRequestHeader().get("hello"));
                  longSynchronousSink.next(aLong);
                });

    Flux<Object> objectFlux = RpcServerUtils.decorateWithRequestContext(mock, handle);
    StepVerifier.create(objectFlux.ignoreElements()).verifyComplete();
  }

  private static boolean isMacos() {
    return System.getProperty("os.name").startsWith("Mac");
  }
}
