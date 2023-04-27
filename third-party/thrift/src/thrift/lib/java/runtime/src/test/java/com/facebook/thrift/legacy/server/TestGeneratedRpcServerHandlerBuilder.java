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

package com.facebook.thrift.legacy.server;

import com.facebook.thrift.example.ping.ExtendedPing;
import com.facebook.thrift.example.ping.PingA;
import com.facebook.thrift.example.ping.PingB;
import com.facebook.thrift.example.ping.PingRequest;
import com.facebook.thrift.example.ping.PingResponse;
import com.facebook.thrift.example.ping.PingService;
import com.facebook.thrift.legacy.server.testservices.BlockingPingService;
import java.util.Collections;
import java.util.Set;
import org.apache.thrift.TException;
import org.junit.Assert;
import org.junit.Test;
import org.mockito.Mockito;

public class TestGeneratedRpcServerHandlerBuilder {
  @Test
  public void testFindThriftInterface() {
    PingService mock = Mockito.mock(PingService.class);
    Set<Class<?>> thriftInterfaces =
        GeneratedRpcServerHandlerBuilder.getAllThriftInterfaces(mock.getClass());
    Assert.assertEquals(1, thriftInterfaces.size());
    Assert.assertTrue(thriftInterfaces.contains(PingService.class));
  }

  @Test
  public void testFindNoThriftInterface() {
    Object mock = Mockito.mock(Object.class);
    Set<Class<?>> thriftInterfaces =
        GeneratedRpcServerHandlerBuilder.getAllThriftInterfaces(mock.getClass());
    Assert.assertTrue(thriftInterfaces.isEmpty());
  }

  @Test
  public void testFindThriftInterfaceWithInheritance() {
    // Note: ExtendedPing extends PingService
    PingService mock = Mockito.mock(ExtendedPing.class);
    Set<Class<?>> thriftInterfaces =
        GeneratedRpcServerHandlerBuilder.getAllThriftInterfaces(mock.getClass());
    Assert.assertEquals(1, thriftInterfaces.size());
    Assert.assertTrue(thriftInterfaces.contains(ExtendedPing.class));
  }

  @Test
  public void testFindMultipleThriftInterface() {
    // Note: PingAB implements both PingA and PingB
    Set<Class<?>> thriftInterfaces =
        GeneratedRpcServerHandlerBuilder.getAllThriftInterfaces(PingAB.class);
    Assert.assertEquals(2, thriftInterfaces.size());
    Assert.assertTrue(thriftInterfaces.contains(PingA.class));
    Assert.assertTrue(thriftInterfaces.contains(PingB.class));
  }

  @Test
  public void testFindCorrectServerHandlerBuilder() {
    GeneratedRpcServerHandlerBuilder.getServerHandlerBuilder(
        Mockito.mock(PingService.class), PingService.class);
    GeneratedRpcServerHandlerBuilder.getServerHandlerBuilder(
        Mockito.mock(PingService.Async.class), PingService.Async.class);
    GeneratedRpcServerHandlerBuilder.getServerHandlerBuilder(
        Mockito.mock(PingService.Reactive.class), PingService.Reactive.class);
  }

  @Test(expected = ClassCastException.class)
  public void testCannotFindServerHandlerBuilder() {
    GeneratedRpcServerHandlerBuilder.getServerHandlerBuilder(
        Mockito.mock(PingService.class), PingService.Async.class);
  }

  @Test
  public void testGenerateRpcServerHandler() {
    GeneratedRpcServerHandlerBuilder.generatedRpcServerHandler(
        Collections.singletonList(new BlockingPingService()), Collections.emptyList());
    GeneratedRpcServerHandlerBuilder.generatedRpcServerHandler(
        Collections.singletonList(Mockito.mock(PingService.class)), Collections.emptyList());
  }

  private static class PingAB implements PingA, PingB {
    @Override
    public void close() {}

    @Override
    public PingResponse pingA(PingRequest pingRequest) throws TException {
      return null;
    }

    @Override
    public PingResponse pingB(PingRequest pingRequest) throws TException {
      return null;
    }
  }
}
