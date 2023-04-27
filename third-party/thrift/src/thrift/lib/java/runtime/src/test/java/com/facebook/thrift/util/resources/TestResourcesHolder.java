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

package com.facebook.thrift.util.resources;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotSame;
import static org.junit.Assert.assertSame;
import static org.junit.Assert.assertTrue;

import io.netty.channel.EventLoopGroup;
import io.netty.util.HashedWheelTimer;
import io.netty.util.ResourceLeakDetector;
import java.util.concurrent.CountDownLatch;
import org.junit.Assert;
import org.junit.Test;
import reactor.core.publisher.Flux;

public class TestResourcesHolder {
  @Test
  public void testLifeCycle() {
    ResourcesHolder holder = new ResourcesHolder();
    EventLoopGroup g1 = holder.getEventLoopGroup();
    EventLoopGroup g2 = holder.getEventLoopGroup();
    assertSame(g1, g2);
    holder.dispose();

    holder = new ResourcesHolder();
    EventLoopGroup g3 = holder.getEventLoopGroup();
    assertNotSame(g1, g3);
    holder.dispose();
  }

  @Test(expected = IllegalStateException.class)
  public void testHashedWheelTimerSupplierShouldStopTimerAfterClose() {
    ResourcesHolder holder = new ResourcesHolder();
    HashedWheelTimer h1 = holder.getTimer();
    holder.dispose();
    h1.start();
  }

  @Test
  public void testResourceLeakDisabled() {
    new ResourcesHolder();
    assertEquals(ResourceLeakDetector.Level.DISABLED, ResourceLeakDetector.getLevel());
  }

  @Test
  public void testShouldThrowExceptionWhenTryingToBlockEventLoop() throws Exception {
    ResourcesHolder holder = new ResourcesHolder();

    EventLoopGroup group = holder.getEventLoopGroup();
    CountDownLatch latch = new CountDownLatch(1);
    group.submit(
        () -> {
          try {
            Flux.just("foo").blockLast();
            Assert.fail();
          } catch (Throwable e) {
            assertTrue(e instanceof IllegalStateException);
          }

          latch.countDown();
        });
    holder.dispose();
    latch.await();
  }
}
