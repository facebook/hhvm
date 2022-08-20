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

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;
import static org.junit.Assume.assumeFalse;
import static org.junit.Assume.assumeTrue;

import com.facebook.nifty.ssl.SslSession;
import com.facebook.swift.service.SwiftConstants;
import com.facebook.swift.service.ThriftServerConfig;
import com.facebook.thrift.legacy.server.ThriftOptionalSslHandler;
import com.facebook.thrift.util.resources.RpcResources;
import com.google.common.util.concurrent.ThreadFactoryBuilder;
import io.netty.channel.EventLoopGroup;
import io.netty.channel.epoll.EpollServerDomainSocketChannel;
import io.netty.channel.epoll.EpollServerSocketChannel;
import io.netty.channel.kqueue.KQueueServerDomainSocketChannel;
import io.netty.channel.kqueue.KQueueServerSocketChannel;
import io.netty.channel.nio.NioEventLoopGroup;
import io.netty.channel.socket.nio.NioServerSocketChannel;
import io.netty.channel.unix.DomainSocketAddress;
import io.netty.handler.ssl.SslContext;
import io.netty.util.AttributeKey;
import java.net.InetSocketAddress;
import java.net.SocketAddress;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;

public class RpcServerUtilsTest {

  @Rule public ExpectedException expectedException = ExpectedException.none();
  private EventLoopGroup group = RpcResources.getEventLoopGroup();

  @Test
  public void testDomainSocketChannelLinux() {
    assumeFalse(isMacos());
    SocketAddress socketAddress = new DomainSocketAddress("/foo");
    Class<?> channelClass = RpcServerUtils.getChannelClass(group, socketAddress);
    assertEquals(channelClass, EpollServerDomainSocketChannel.class);
  }

  @Test
  public void testEpollSocketChannel() {
    assumeFalse(isMacos());
    SocketAddress socketAddress = new InetSocketAddress(0);
    Class<?> channelClass = RpcServerUtils.getChannelClass(group, socketAddress);
    assertEquals(channelClass, EpollServerSocketChannel.class);
  }

  @Test
  public void testDomainSocketChannelMacos() {
    if (!isMacos()) {
      return;
    }

    assumeTrue(isMacos());
    SocketAddress socketAddress = new DomainSocketAddress("/foo");
    Class<?> channelClass = RpcServerUtils.getChannelClass(group, socketAddress);
    assertEquals(channelClass, KQueueServerDomainSocketChannel.class);
  }

  @Test
  public void testKQueueSocketChannel() {
    if (!isMacos()) {
      return;
    }

    assumeTrue(isMacos());
    SocketAddress socketAddress = new InetSocketAddress(0);
    Class<?> channelClass = RpcServerUtils.getChannelClass(group, socketAddress);
    assertEquals(channelClass, KQueueServerSocketChannel.class);
  }

  @Test
  public void testNioSocketChannel() {
    SocketAddress socketAddress = new InetSocketAddress(0);
    Class<?> channelClass =
        RpcServerUtils.getChannelClass(
            new NioEventLoopGroup(0, new ThreadFactoryBuilder().build()), socketAddress);
    assertEquals(channelClass, NioServerSocketChannel.class);
  }

  @Test
  public void testInvalidSocketGroupCombination() {
    expectedException.expect(UnsupportedOperationException.class);
    SocketAddress socketAddress = new DomainSocketAddress("/foo");
    RpcServerUtils.getChannelClass(
        new NioEventLoopGroup(0, new ThreadFactoryBuilder().build()), socketAddress);
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

  private static boolean isMacos() {
    return System.getProperty("os.name").startsWith("Mac");
  }
}
