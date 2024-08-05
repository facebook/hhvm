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

import com.facebook.thrift.metrics.distribution.DistributionConfig;
import com.facebook.thrift.metrics.distribution.Quantile;
import java.util.List;
import java.util.Map;
import java.util.concurrent.locks.ReentrantLock;
import java.util.stream.Collectors;
import org.HdrHistogram.Histogram;

public class AllTimeHistogram {

  private final Histogram allTimeHistogram;
  private final ReentrantLock lock = new ReentrantLock();

  public AllTimeHistogram(DistributionConfig config) {
    this.allTimeHistogram = new Histogram(config.significantDigitCount());
  }

  public Map<Quantile, Long> getQuantiles(List<Quantile> quantiles) {
    lock.lock();
    try {
      return quantiles.stream()
          .collect(
              Collectors.toMap(
                  quantile -> quantile,
                  quantile -> {
                    if (quantile == AVG) {
                      return (long) allTimeHistogram.getMean();
                    } else if (quantile == MAX) {
                      return allTimeHistogram.getMaxValue();
                    } else if (quantile == MIN) {
                      return allTimeHistogram.getMinValue();
                    } else if (quantile == SUM) {
                      return allTimeHistogram.getTotalCount();
                    } else if (quantile.getQuantile() >= 0.0 && quantile.getQuantile() <= 100.0) {
                      return allTimeHistogram.getValueAtPercentile(quantile.getQuantile());
                    } else {
                      // this shouldn't happen
                      return 0L;
                    }
                  }));
    } finally {
      lock.unlock();
    }
  }

  public void performIntervalSample(Histogram intervalHistogram) {
    lock.lock();
    try {
      allTimeHistogram.add(intervalHistogram);
    } finally {
      lock.unlock();
    }
  }

  public long getEstimatedBytes() {
    return allTimeHistogram.getEstimatedFootprintInBytes();
  }
}
