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

package com.facebook.thrift.metrics.distribution;

import com.facebook.thrift.metrics.common.Clock;
import com.facebook.thrift.metrics.distribution.history.TimeWindowedHistogram;
import java.util.Arrays;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ScheduledExecutorService;
import org.HdrHistogram.Histogram;
import org.HdrHistogram.Recorder;

/**
 * Distribution that contains a single 1-minute window. This distribution implements
 * AbstractDistribution which coordinates scheduling of sample periods. It will call this class's
 * doSample every 10 seconds. This class will delegate to its TimeWindowedHistogram which will store
 * the samples from the last 10 seconds into its respective ring buffer
 */
public class SingleWindowDistribution extends AbstractDistribution {
  private final List<Quantile> quantiles;
  private final TimeWindowedHistogram oneMinute;
  private final Recorder recorder;
  private Histogram intervalHistogram = null;

  public SingleWindowDistribution() {
    this(
        Utils.getExecutorService(),
        Utils.getClock(),
        Arrays.asList(Quantile.P50, Quantile.P75, Quantile.P95, Quantile.P99));
  }

  public SingleWindowDistribution(List<Quantile> quantiles) {
    this(Utils.getExecutorService(), Utils.getClock(), quantiles);
  }

  public SingleWindowDistribution(
      ScheduledExecutorService executorService, Clock clock, List<Quantile> quantiles) {
    super(executorService);
    this.quantiles = quantiles;
    recorder = new Recorder(DistributionConfig.ONE_MINUTE.significantDigitCount());
    oneMinute = new TimeWindowedHistogram(DistributionConfig.ONE_MINUTE, clock);
  }

  public void add(long value) {
    recorder.recordValue(value);
  }

  @Override
  void performIntervalSampleImpl() {
    intervalHistogram = recorder.getIntervalHistogram(intervalHistogram);

    oneMinute.performIntervalSample(intervalHistogram);
  }

  public Map<Quantile, Long> getOneMinuteQuantiles() {
    return oneMinute.getQuantiles(quantiles);
  }

  public long getEstimatedBytes() {
    // recorder has a histogram that is swapped with interval histogram
    long intervalSize =
        intervalHistogram == null ? 0 : intervalHistogram.getEstimatedFootprintInBytes();
    return intervalSize * 2L + oneMinute.getEstimatedBytes();
  }
}
