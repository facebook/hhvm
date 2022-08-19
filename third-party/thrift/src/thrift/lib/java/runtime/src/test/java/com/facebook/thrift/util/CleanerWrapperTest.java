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

import java.util.concurrent.CountDownLatch;
import org.junit.Assert;
import org.junit.Test;

public class CleanerWrapperTest {
  @Test
  public void testCreate() {
    CleanerWrapper wrapper = CleanerWrapper.create();
    Assert.assertNotNull(wrapper);
  }

  @Test
  public void testCreateWithThreadFactory() {
    CleanerWrapper wrapper = CleanerWrapper.create(Thread::new);

    Assert.assertNotNull(wrapper);
  }

  @Test
  public void testRegister() throws Exception {
    if (System.getProperty("java.version").startsWith("1.8")) {
      return;
    }

    CountDownLatch latch = new CountDownLatch(1);
    CleanerWrapper.create().register(new Object(), latch::countDown);
    System.gc();
    latch.await();
  }
}
