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
import org.junit.jupiter.api.Assertions;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;
import org.mockito.*;

public class TestThriftServerStats {

  private ServerStats thriftServerStats = new ServerStats();

  @Mock ScheduledExecutorService executorService;

  @Captor ArgumentCaptor<Runnable> runnableCaptor;

  @BeforeEach
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
    Assertions.assertEquals(1L, (long) actual.get("thrift.received_requests.count"));
    Assertions.assertEquals(1L, (long) actual.get("thrift.received_requests.count.60"));
    Assertions.assertEquals(1L, (long) actual.get("thrift.received_requests.count.3600"));

    Assertions.assertEquals(1L, (long) actual.get("thrift.foo.num_calls.sum"));
    Assertions.assertEquals(1L, (long) actual.get("thrift.foo.num_calls.sum.60"));
    Assertions.assertEquals(1L, (long) actual.get("thrift.foo.num_calls.sum.3600"));
  }

  @Test
  public void testReplySent() throws Exception {
    thriftServerStats.replySent(20L, "foo");
    Map<String, Long> actual = thriftServerStats.getCounters();
    Assertions.assertEquals(1L, (long) actual.get("thrift.sent_replies.count"));
    Assertions.assertEquals(1L, (long) actual.get("thrift.sent_replies.count.60"));
    Assertions.assertEquals(1L, (long) actual.get("thrift.sent_replies.count.3600"));
  }

  @Test
  public void testProcessTime() throws Exception {
    thriftServerStats.replySent(10L, "foo");

    performIntervalSampleOnDistributions();

    Map<String, Long> actual = thriftServerStats.getCounters();

    Assertions.assertEquals(10L, (long) actual.get("thrift.process_time.avg"));
    Assertions.assertEquals(10L, (long) actual.get("thrift.process_time.avg.60"));
    Assertions.assertEquals(10L, (long) actual.get("thrift.process_time.avg.3600"));

    Assertions.assertEquals(10L, (long) actual.get("thrift.process_time.p99"));
    Assertions.assertEquals(10L, (long) actual.get("thrift.process_time.p99.60"));
    Assertions.assertEquals(10L, (long) actual.get("thrift.process_time.p99.3600"));

    Assertions.assertEquals(1L, (long) actual.get("thrift.foo.num_processed.sum"));
    Assertions.assertEquals(1L, (long) actual.get("thrift.foo.num_processed.sum.60"));
    Assertions.assertEquals(1L, (long) actual.get("thrift.foo.num_processed.sum.3600"));

    Assertions.assertEquals(10L, (long) actual.get("thrift.foo.time_process_us.avg"));
    Assertions.assertEquals(10L, (long) actual.get("thrift.foo.time_process_us.avg.60"));
    Assertions.assertEquals(10L, (long) actual.get("thrift.foo.time_process_us.avg.3600"));
  }

  @Test
  public void testError() throws Exception {
    thriftServerStats.error("foo");
    Map<String, Long> actual = thriftServerStats.getCounters();
    Assertions.assertEquals(1L, (long) actual.get("thrift.foo.num_exceptions.sum"));
    Assertions.assertEquals(1L, (long) actual.get("thrift.foo.num_exceptions.sum.60"));
    Assertions.assertEquals(1L, (long) actual.get("thrift.foo.num_exceptions.sum.3600"));
  }

  @Test
  public void testWriteTime() throws Exception {
    thriftServerStats.publishWriteTime(10L);

    performIntervalSampleOnDistributions();

    Map<String, Long> actual = thriftServerStats.getCounters();

    Assertions.assertEquals(10L, (long) actual.get("thrift.write_time.p99"));
    Assertions.assertEquals(10L, (long) actual.get("thrift.write_time.p99.60"));
    Assertions.assertEquals(10L, (long) actual.get("thrift.write_time.p99.3600"));
  }
}
