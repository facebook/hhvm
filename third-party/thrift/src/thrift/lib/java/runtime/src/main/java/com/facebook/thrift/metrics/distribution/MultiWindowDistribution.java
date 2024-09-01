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
import com.facebook.thrift.metrics.distribution.history.AllTimeHistogram;
import com.facebook.thrift.metrics.distribution.history.TimeWindowedHistogram;
import java.util.Arrays;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ScheduledExecutorService;
import org.HdrHistogram.Histogram;
import org.HdrHistogram.Recorder;

/**
 * Distribution that contains multiple windows to comply with fb303 stats export, 1 minute, 10
 * minute, 1 hour and all time. This distribution implements AbstractDistribution which coordinates
 * scheduling of sample periods. It will call this class's doSample every 10 seconds. This class
 * will delegate to each Multi window distribution which will store the samples from the last 10
 * seconds into its respective ring buffer
 */
public class MultiWindowDistribution extends AbstractDistribution {

  private final List<Quantile> quantiles;

  private final TimeWindowedHistogram oneMinute;

  private final TimeWindowedHistogram tenMinute;

  private final TimeWindowedHistogram oneHour;

  private final AllTimeHistogram allTime;

  private final Recorder recorder;
  private Histogram intervalHistogram = null;

  public MultiWindowDistribution() {
    this(
        Utils.getExecutorService(),
        Utils.getClock(),
        Arrays.asList(Quantile.P50, Quantile.P75, Quantile.P95, Quantile.P99));
  }

  public MultiWindowDistribution(ScheduledExecutorService executorService, Clock clock) {
    this(
        executorService,
        clock,
        Arrays.asList(Quantile.P50, Quantile.P75, Quantile.P95, Quantile.P99));
  }

  public MultiWindowDistribution(
      ScheduledExecutorService executorService, Clock clock, List<Quantile> quantiles) {
    super(executorService);
    this.quantiles = quantiles;
    this.recorder = new Recorder(DistributionConfig.ONE_MINUTE.significantDigitCount());

    this.oneMinute = new TimeWindowedHistogram(DistributionConfig.ONE_MINUTE, clock);
    this.tenMinute = new TimeWindowedHistogram(DistributionConfig.TEN_MINUTE, clock);
    this.oneHour = new TimeWindowedHistogram(DistributionConfig.ONE_HOUR, clock);
    this.allTime = new AllTimeHistogram(DistributionConfig.ALL_TIME);
  }

  public void add(long value) {
    recorder.recordValue(value);
  }

  @Override
  void performIntervalSampleImpl() {
    intervalHistogram = recorder.getIntervalHistogram(intervalHistogram);

    oneMinute.performIntervalSample(intervalHistogram);
    tenMinute.performIntervalSample(intervalHistogram);
    oneHour.performIntervalSample(intervalHistogram);
    allTime.performIntervalSample(intervalHistogram);
  }

  public Map<Quantile, Long> getOneMinuteQuantiles() {
    return oneMinute.getQuantiles(quantiles);
  }

  public Map<Quantile, Long> getTenMinuteQuantiles() {
    return tenMinute.getQuantiles(quantiles);
  }

  public Map<Quantile, Long> getOneHourQuantiles() {
    return oneHour.getQuantiles(quantiles);
  }

  public Map<Quantile, Long> getAllTimeQuantiles() {
    return allTime.getQuantiles(quantiles);
  }

  public long getEstimatedBytes() {
    // recorder has a histogram that is swapped with interval histogram
    long intervalBytes =
        intervalHistogram == null ? 0 : intervalHistogram.getEstimatedFootprintInBytes();
    return intervalBytes * 2L
        + oneMinute.getEstimatedBytes()
        + tenMinute.getEstimatedBytes()
        + oneHour.getEstimatedBytes()
        + allTime.getEstimatedBytes();
  }
}
