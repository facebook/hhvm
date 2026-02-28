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

package com.facebook.thrift.loadbalancing;

import static org.junit.Assert.*;
import static org.mockito.MockitoAnnotations.openMocks;

import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;
import org.junit.After;
import org.junit.Before;
import org.junit.Test;

public class OutstandingRequestLoadSupplierTest {
  AutoCloseable closeable;
  OutstandingRequestLoadSupplier supplier;

  @Before
  public void setUp() throws Exception {
    closeable = openMocks(this);
    supplier = new OutstandingRequestLoadSupplier();
  }

  @After
  public void tearDown() throws Exception {
    closeable.close();
  }

  @Test
  public void testMultipleThreadsIncrement() throws Exception {
    ExecutorService exec = Executors.newFixedThreadPool(10);
    for (int i = 0; i < 10_000; i++) {
      exec.execute(() -> supplier.onRequest());
    }

    exec.shutdown();
    exec.awaitTermination(10, TimeUnit.SECONDS);

    assertEquals(supplier.getLoad(), 10_000);
  }

  @Test
  public void testMultipleThreadsIncrementAndDecrement() throws Exception {
    ExecutorService exec = Executors.newFixedThreadPool(10);
    for (int i = 0; i < 10_000; i++) {
      exec.execute(() -> supplier.onRequest());
    }

    for (int i = 0; i < 10_000; i++) {
      exec.execute(() -> supplier.doFinally());
    }

    exec.shutdown();
    exec.awaitTermination(10, TimeUnit.SECONDS);

    assertEquals(supplier.getLoad(), 0);
  }
}
