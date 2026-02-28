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

package com.facebook.thrift.metrics.distribution.history;

import static com.facebook.thrift.metrics.distribution.Quantile.AVG;
import static com.facebook.thrift.metrics.distribution.Quantile.MAX;
import static com.facebook.thrift.metrics.distribution.Quantile.MIN;
import static com.facebook.thrift.metrics.distribution.Quantile.SUM;

import com.facebook.thrift.metrics.common.Clock;
import com.facebook.thrift.metrics.distribution.DistributionConfig;
import com.facebook.thrift.metrics.distribution.Quantile;
import java.util.Arrays;
import java.util.List;
import java.util.Map;
import java.util.concurrent.locks.ReentrantLock;
import java.util.stream.Collectors;
import org.HdrHistogram.Histogram;

/**
 * Time windowed histogram that maintains a history via circular ring buffer. Each value in the
 * buffer contains a histogram representing a time window of size
 * DistributionConfig.chunkResetPeriodMs, the ring contains DistributionConfig.chunkCount buffers i
 * total The defaults are contained in DistributionConfig.ONE_MINUTE|TEN_MINUTE|ONE_HOUR. Each
 * configuration has its own window size and count. Samples can be fed in whenever and samples will
 * be added to the appropriate bucked aging out old bucks when they are expired. This is done by
 * either adding samples to history item or overwriting them when they are expired.
 */
public class TimeWindowedHistogram {
  private final HistoryItem[] history;
  private final long creationTimeMs;
  private final Clock clock;
  private final long chunkResetPeriodMs;
  private final int historySize;
  private final Histogram histogramForSnapshot;
  private final ReentrantLock lock = new ReentrantLock();

  public TimeWindowedHistogram(DistributionConfig config, Clock clock) {
    // align start time to last tick (means it could be created at a time in the past)
    this.creationTimeMs = clock.tickMilis() - (clock.tickMilis() % config.chunkResetPeriodMs());

    this.chunkResetPeriodMs = config.chunkResetPeriodMs();
    this.historySize = config.chunkCount();
    this.clock = clock;
    this.histogramForSnapshot = new Histogram(config.significantDigitCount());

    this.history = new HistoryItem[historySize];
    for (int i = 0; i < this.historySize; ++i) {
      this.history[i] =
          new HistoryItem(new Histogram(config.significantDigitCount()), Long.MIN_VALUE);
    }
  }

  public void performIntervalSample(Histogram intervalHistogram) {
    lock.lock();
    try {
      long nowMs = clock.tickMilis();
      long elapsedMs = nowMs - creationTimeMs;

      long currPhaseNum = elapsedMs / chunkResetPeriodMs;
      int historyIndex = (int) (currPhaseNum) % historySize;

      // expired, overwrite and update expire time
      if (history[historyIndex].proposedInvalidationTimeMs <= nowMs) {
        intervalHistogram.copyInto(history[historyIndex].histogram);

        history[historyIndex].proposedInvalidationTimeMs =
            creationTimeMs + (currPhaseNum + historySize) * chunkResetPeriodMs;
      } else {
        // current index is not expired, we haven't wrapped around yet, add to existing history
        history[historyIndex].histogram.add(intervalHistogram);
      }
    } finally {
      lock.unlock();
    }
  }

  public long getEstimatedBytes() {
    long size = 0;
    for (HistoryItem item : history) {
      size += item.histogram.getEstimatedFootprintInBytes();
    }

    size += histogramForSnapshot.getEstimatedFootprintInBytes();

    return size;
  }

  public Map<Quantile, Long> getQuantiles(List<Quantile> quantiles) {
    lock.lock();
    try {
      long nowMs = clock.tickMilis();
      histogramForSnapshot.reset();

      Arrays.stream(history)
          // guard against expired data
          .filter(item -> item.proposedInvalidationTimeMs > nowMs)
          .forEach(item -> histogramForSnapshot.add(item.histogram));

      return quantiles.stream()
          .collect(
              Collectors.toMap(
                  quantile -> quantile,
                  quantile -> {
                    if (quantile == AVG) {
                      return (long) histogramForSnapshot.getMean();
                    } else if (quantile == MAX) {
                      return histogramForSnapshot.getMaxValue();
                    } else if (quantile == MIN) {
                      return histogramForSnapshot.getMinValue();
                    } else if (quantile == SUM) {
                      return histogramForSnapshot.getTotalCount();
                    } else if (quantile.getQuantile() >= 0.0 && quantile.getQuantile() <= 100.0) {
                      return histogramForSnapshot.getValueAtPercentile(quantile.getQuantile());
                    } else {
                      // this shouldn't happen
                      return 0L;
                    }
                  }));
    } finally {
      lock.unlock();
    }
  }

  private static class HistoryItem {
    long proposedInvalidationTimeMs;
    final Histogram histogram;

    public HistoryItem(Histogram histogram, long proposedInvalidationTimeMs) {
      this.proposedInvalidationTimeMs = proposedInvalidationTimeMs;
      this.histogram = histogram;
    }
  }
}
