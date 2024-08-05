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

import java.lang.ref.WeakReference;
import java.util.concurrent.CancellationException;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 * Abstract distribution contains logic for scheduling the interval samples for all histograms that
 * implement this class. When an instance of this class is created, including subclasses, the
 * instance is added to the global list of distributions that need to be updated on a periodic
 * basis. The scheduled executor performs a periodic job to trigger an interval sample every 10
 * seconds. Each distribution is then responsible for taking an interval sample and applying it to
 * its history
 */
public abstract class AbstractDistribution {
  private static final Logger logger = LoggerFactory.getLogger(AbstractDistribution.class);

  /**
   * When a child class of this is constructed register interval sample job every 10 seconds with a
   * weak reference to the underlying histogram to ensure it can be garbage collected if the
   * executor holds the last reference to the distribution.
   */
  public AbstractDistribution(ScheduledExecutorService executorService) {
    WeakReference<AbstractDistribution> weakRef = new WeakReference<>(this);
    executorService.scheduleAtFixedRate(
        () -> performIntervalSample(weakRef), 10_000, 10_000, TimeUnit.MILLISECONDS);
  }

  /**
   * Call the child class interval sample implementation if the weak reference to the distribution
   * exists, if however the object has been garbage collected, throw an exception to the executor to
   * ensure that future sampling jobs get canclled and cleaned up
   */
  private static void performIntervalSample(WeakReference<AbstractDistribution> distribution) {
    if (distribution.get() == null) {
      throw new CancellationException(
          "Distribution has been garbage collected, sampling canceled for this distribution");
    }

    try {
      distribution.get().performIntervalSampleImpl();
    } catch (Throwable t) {
      logger.error("Error while performing sample interval.", t);
    }
  }

  /** Instructs implementing class to perform interval sample */
  abstract void performIntervalSampleImpl();
}
