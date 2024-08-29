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

package com.facebook.swift.service;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyLong;
import static org.mockito.Mockito.atLeastOnce;
import static org.mockito.Mockito.verify;
import static org.mockito.MockitoAnnotations.initMocks;

import com.facebook.swift.service.stats.ServerStats;
import com.facebook.thrift.metrics.distribution.Utils;
import java.util.Map;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.mockito.*;

public class TestThriftServerStats {

  private ServerStats thriftServerStats = new ServerStats();

  @Mock ScheduledExecutorService executorService;

  @Captor ArgumentCaptor<Runnable> runnableCaptor;

  @Before
  public void setup() {
    initMocks(this);
    Utils.setExecutorService(executorService);
  }

  // Ensure at least one interval sample is done on histogram so that data is collected
  private void performIntervalSampleOnDistributions() {
    verify(executorService, atLeastOnce())
        .scheduleAtFixedRate(runnableCaptor.capture(), anyLong(), anyLong(), any(TimeUnit.class));
    runnableCaptor.getAllValues().forEach(Runnable::run);
  }

  @Test
  public void testRequestReceived() throws Exception {
    thriftServerStats.requestReceived(10L, "foo");
    Map<String, Long> actual = thriftServerStats.getCounters();
    Assert.assertEquals(1L, (long) actual.get("thrift.received_requests.count"));
    Assert.assertEquals(1L, (long) actual.get("thrift.received_requests.count.60"));
    Assert.assertEquals(1L, (long) actual.get("thrift.received_requests.count.3600"));

    Assert.assertEquals(1L, (long) actual.get("thrift.foo.num_calls.sum"));
    Assert.assertEquals(1L, (long) actual.get("thrift.foo.num_calls.sum.60"));
    Assert.assertEquals(1L, (long) actual.get("thrift.foo.num_calls.sum.3600"));
  }

  @Test
  public void testReplySent() throws Exception {
    thriftServerStats.replySent(20L, "foo");
    Map<String, Long> actual = thriftServerStats.getCounters();
    Assert.assertEquals(1L, (long) actual.get("thrift.sent_replies.count"));
    Assert.assertEquals(1L, (long) actual.get("thrift.sent_replies.count.60"));
    Assert.assertEquals(1L, (long) actual.get("thrift.sent_replies.count.3600"));
  }

  @Test
  public void testProcessTime() throws Exception {
    thriftServerStats.replySent(10L, "foo");

    performIntervalSampleOnDistributions();

    Map<String, Long> actual = thriftServerStats.getCounters();

    Assert.assertEquals(10L, (long) actual.get("thrift.process_time.avg"));
    Assert.assertEquals(10L, (long) actual.get("thrift.process_time.avg.60"));
    Assert.assertEquals(10L, (long) actual.get("thrift.process_time.avg.3600"));

    Assert.assertEquals(10L, (long) actual.get("thrift.process_time.p99"));
    Assert.assertEquals(10L, (long) actual.get("thrift.process_time.p99.60"));
    Assert.assertEquals(10L, (long) actual.get("thrift.process_time.p99.3600"));

    Assert.assertEquals(1L, (long) actual.get("thrift.foo.num_processed.sum"));
    Assert.assertEquals(1L, (long) actual.get("thrift.foo.num_processed.sum.60"));
    Assert.assertEquals(1L, (long) actual.get("thrift.foo.num_processed.sum.3600"));

    Assert.assertEquals(10L, (long) actual.get("thrift.foo.time_process_us.avg"));
    Assert.assertEquals(10L, (long) actual.get("thrift.foo.time_process_us.avg.60"));
    Assert.assertEquals(10L, (long) actual.get("thrift.foo.time_process_us.avg.3600"));
  }

  @Test
  public void testError() throws Exception {
    thriftServerStats.error("foo");
    Map<String, Long> actual = thriftServerStats.getCounters();
    Assert.assertEquals(1L, (long) actual.get("thrift.foo.num_exceptions.sum"));
    Assert.assertEquals(1L, (long) actual.get("thrift.foo.num_exceptions.sum.60"));
    Assert.assertEquals(1L, (long) actual.get("thrift.foo.num_exceptions.sum.3600"));
  }

  @Test
  public void testWriteTime() throws Exception {
    thriftServerStats.publishWriteTime(10L);

    performIntervalSampleOnDistributions();

    Map<String, Long> actual = thriftServerStats.getCounters();

    Assert.assertEquals(10L, (long) actual.get("thrift.write_time.p99"));
    Assert.assertEquals(10L, (long) actual.get("thrift.write_time.p99.60"));
    Assert.assertEquals(10L, (long) actual.get("thrift.write_time.p99.3600"));
  }
}
