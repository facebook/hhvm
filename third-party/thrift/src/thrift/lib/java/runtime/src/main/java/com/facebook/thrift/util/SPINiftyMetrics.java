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

import com.facebook.nifty.core.NiftyMetrics;
import io.airlift.stats.DecayCounter;
import io.airlift.stats.ExponentialDecay;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicLong;

/** NiftyMetrics to support backward compatibile in SPI while refactoring to Netty 4 */
@Deprecated
public class SPINiftyMetrics implements NiftyMetrics {
  private final AtomicInteger channelCount;
  private final AtomicLong bytesRead;
  private final AtomicLong bytesWritten;
  private final AtomicLong rejectedConnections;
  private final DecayCounter rejectedConnectionsOneMin;
  private final DecayCounter rejectedConnectionsOneHour;
  private final AtomicLong droppedConnections;
  private final DecayCounter droppedConnectionsOneMin;
  private final DecayCounter droppedConnectionsOneHour;
  private final AtomicLong acceptedConnections;
  private final DecayCounter acceptedConnectionsOneMin;
  private final DecayCounter acceptedConnectionsOnHour;

  public SPINiftyMetrics() {
    channelCount = new AtomicInteger();
    bytesRead = new AtomicLong();
    bytesWritten = new AtomicLong();
    rejectedConnections = new AtomicLong();
    rejectedConnectionsOneMin = new DecayCounter(ExponentialDecay.oneMinute());
    rejectedConnectionsOneHour = new DecayCounter(ExponentialDecay.seconds(3600));
    droppedConnections = new AtomicLong();
    droppedConnectionsOneMin = new DecayCounter(ExponentialDecay.oneMinute());
    droppedConnectionsOneHour = new DecayCounter(ExponentialDecay.seconds(3600));
    acceptedConnections = new AtomicLong();
    acceptedConnectionsOneMin = new DecayCounter(ExponentialDecay.oneMinute());
    acceptedConnectionsOnHour = new DecayCounter(ExponentialDecay.oneMinute());
  }

  @Override
  public int getChannelCount() {
    return channelCount.get();
  }

  public void incrementChannelCount() {
    channelCount.incrementAndGet();
  }

  public void decrementChannelCount() {
    channelCount.decrementAndGet();
  }

  @Override
  public long getBytesRead() {
    return bytesRead.get();
  }

  public void incrementBytesRead(long bytes) {
    bytesRead.addAndGet(bytes);
  }

  @Override
  public long getBytesWritten() {
    return bytesWritten.get();
  }

  public void incrementBytesWrite(long bytes) {
    bytesWritten.getAndAdd(bytes);
  }

  @Override
  public long getRejectedConnections() {
    return rejectedConnections.get();
  }

  public void incrementRejectedConnections() {
    rejectedConnections.incrementAndGet();
    rejectedConnectionsOneMin.add(1L);
    rejectedConnectionsOneHour.add(1L);
  }

  @Override
  public long getRejectedConnectionsOneMin() {
    return Math.round(rejectedConnectionsOneMin.getCount());
  }

  @Override
  public long getRejectedConnectionsOneHour() {
    return Math.round(rejectedConnectionsOneHour.getCount());
  }

  @Override
  public long getDroppedConnections() {
    return droppedConnections.get();
  }

  public void incrementDroppedConnections() {
    droppedConnections.incrementAndGet();
    droppedConnectionsOneMin.add(1L);
    droppedConnectionsOneHour.add(1L);
  }

  @Override
  public long getDroppedConnectionsOneMin() {
    return Math.round(droppedConnectionsOneMin.getCount());
  }

  @Override
  public long getDroppedConnectionsOneHour() {
    return Math.round(droppedConnectionsOneHour.getCount());
  }

  public void incrementAcceptedConnections() {
    acceptedConnections.incrementAndGet();
    acceptedConnectionsOneMin.add(1L);
    acceptedConnectionsOnHour.add(1L);
  }

  @Override
  public long getAcceptedConnections() {
    return acceptedConnections.get();
  }

  @Override
  public long getAcceptedConnectionsOneMin() {
    return Math.round(acceptedConnectionsOneMin.getCount());
  }

  @Override
  public long getAcceptedConnectionsOneHour() {
    return Math.round(acceptedConnectionsOnHour.getCount());
  }
}
