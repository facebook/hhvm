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

import java.util.Map;
import org.junit.Assert;
import org.junit.Test;

public class TestThriftClientStats {

  private ThriftClientStats thriftClientStats = new ThriftClientStats();

  @Test
  public void testCall() throws Exception {
    thriftClientStats.call("foo");
    Map<String, Long> actual = thriftClientStats.getCounters();
    Assert.assertEquals(1L, (long) actual.get("thrift_client.foo.num_calls.sum"));
    Assert.assertEquals(1L, (long) actual.get("thrift_client.foo.num_calls.sum.60"));
    Assert.assertEquals(1L, (long) actual.get("thrift_client.foo.num_calls.sum.3600"));
  }

  @Test
  public void testPublishWrite() throws Exception {
    thriftClientStats.publishWrite("foo", 10L);
    thriftClientStats.publishWrite("foo", 20L);
    thriftClientStats.publishWrite("foo", 30L);
    thriftClientStats.publishWrite("foo", 40L);
    thriftClientStats.publishWrite("foo", 50L);

    Map<String, Long> actual = thriftClientStats.getCounters();
    Assert.assertEquals(5L, (long) actual.get("thrift_client.foo.num_writes.sum"));
    Assert.assertEquals(5L, (long) actual.get("thrift_client.foo.num_writes.sum.60"));
    Assert.assertEquals(5L, (long) actual.get("thrift_client.foo.num_writes.sum.3600"));

    Assert.assertEquals(30L, (long) actual.get("thrift_client.foo.time_write_us.avg"));
    Assert.assertEquals(30L, (long) actual.get("thrift_client.foo.time_write_us.avg.60"));
    Assert.assertEquals(30L, (long) actual.get("thrift_client.foo.time_write_us.avg.3600"));

    Assert.assertEquals(50L, (long) actual.get("thrift_client.foo.time_write_us.p90"));
    Assert.assertEquals(50L, (long) actual.get("thrift_client.foo.time_write_us.p90.60"));
    Assert.assertEquals(50L, (long) actual.get("thrift_client.foo.time_write_us.p90.3600"));

    Assert.assertEquals(50L, (long) actual.get("thrift_client.foo.time_write_us.p99"));
    Assert.assertEquals(50L, (long) actual.get("thrift_client.foo.time_write_us.p99.60"));
    Assert.assertEquals(50L, (long) actual.get("thrift_client.foo.time_write_us.p99.3600"));
  }

  @Test
  public void testPublishRead() throws Exception {
    thriftClientStats.publishRead("foo", 10L);
    thriftClientStats.publishRead("foo", 30L);
    thriftClientStats.publishRead("foo", 40L);
    thriftClientStats.publishRead("foo", 40L);
    thriftClientStats.publishRead("foo", 70L);

    Map<String, Long> actual = thriftClientStats.getCounters();
    Assert.assertEquals(5L, (long) actual.get("thrift_client.foo.num_reads.sum"));
    Assert.assertEquals(5L, (long) actual.get("thrift_client.foo.num_reads.sum.60"));
    Assert.assertEquals(5L, (long) actual.get("thrift_client.foo.num_reads.sum.3600"));

    Assert.assertEquals(38L, (long) actual.get("thrift_client.foo.time_read_us.avg"));
    Assert.assertEquals(38L, (long) actual.get("thrift_client.foo.time_read_us.avg.60"));
    Assert.assertEquals(38L, (long) actual.get("thrift_client.foo.time_read_us.avg.3600"));

    Assert.assertEquals(70L, (long) actual.get("thrift_client.foo.time_read_us.p90"));
    Assert.assertEquals(70L, (long) actual.get("thrift_client.foo.time_read_us.p90.60"));
    Assert.assertEquals(70L, (long) actual.get("thrift_client.foo.time_read_us.p90.3600"));

    Assert.assertEquals(70L, (long) actual.get("thrift_client.foo.time_read_us.p99"));
    Assert.assertEquals(70L, (long) actual.get("thrift_client.foo.time_read_us.p99.60"));
    Assert.assertEquals(70L, (long) actual.get("thrift_client.foo.time_read_us.p99.3600"));
  }

  @Test
  public void testComplete() throws Exception {
    thriftClientStats.complete("foo", 20L);
    thriftClientStats.complete("foo", 20L);
    thriftClientStats.complete("foo", 30L);
    thriftClientStats.complete("foo", 40L);
    thriftClientStats.complete("foo", 60L);

    Map<String, Long> actual = thriftClientStats.getCounters();
    Assert.assertEquals(34L, (long) actual.get("thrift_client.foo.time_process_us.avg"));
    Assert.assertEquals(34L, (long) actual.get("thrift_client.foo.time_process_us.avg.60"));
    Assert.assertEquals(34L, (long) actual.get("thrift_client.foo.time_process_us.avg.3600"));

    Assert.assertEquals(60L, (long) actual.get("thrift_client.foo.time_process_us.p90"));
    Assert.assertEquals(60L, (long) actual.get("thrift_client.foo.time_process_us.p90.60"));
    Assert.assertEquals(60L, (long) actual.get("thrift_client.foo.time_process_us.p90.3600"));

    Assert.assertEquals(60L, (long) actual.get("thrift_client.foo.time_process_us.p99"));
    Assert.assertEquals(60L, (long) actual.get("thrift_client.foo.time_process_us.p99.60"));
    Assert.assertEquals(60L, (long) actual.get("thrift_client.foo.time_process_us.p99.3600"));
  }

  @Test
  public void testError() throws Exception {
    thriftClientStats.error("foo");
    Map<String, Long> actual = thriftClientStats.getCounters();
    Assert.assertEquals(1L, (long) actual.get("thrift_client.foo.num_exceptions.sum"));
    Assert.assertEquals(1L, (long) actual.get("thrift_client.foo.num_exceptions.sum.60"));
    Assert.assertEquals(1L, (long) actual.get("thrift_client.foo.num_exceptions.sum.3600"));
  }
}
