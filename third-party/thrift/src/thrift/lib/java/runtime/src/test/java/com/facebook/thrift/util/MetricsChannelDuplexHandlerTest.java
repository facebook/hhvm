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

import static org.junit.jupiter.api.Assertions.assertEquals;

import io.netty.channel.embedded.EmbeddedChannel;
import org.junit.jupiter.api.Test;

/** Unit tests for {@link MetricsChannelDuplexHandler}. */
public class MetricsChannelDuplexHandlerTest {

  /**
   * Standard active+inactive sequence: count goes up on active and back down on inactive.
   * Accepted/dropped counters both increment by 1.
   */
  @Test
  public void incrementsAndDecrementsOnMatchingActiveInactive() {
    SPINiftyMetrics metrics = new SPINiftyMetrics();
    EmbeddedChannel channel = new EmbeddedChannel(new MetricsChannelDuplexHandler(metrics));

    // EmbeddedChannel fires channelActive on construction, then channelInactive on close().
    assertEquals(1, metrics.getChannelCount());
    assertEquals(1, metrics.getAcceptedConnections());

    channel.close();

    assertEquals(0, metrics.getChannelCount());
    assertEquals(1, metrics.getDroppedConnections());
  }

  /**
   * Regression: an unmatched channelInactive (no preceding channelActive) must not drive the
   * channel-count gauge negative. Previously the handler unconditionally decremented on
   * channelInactive, which would underflow the gauge when an upstream defer-active handler
   * (reactor-netty's SslReadHandler or our DeferChannelActiveHandler) had suppressed the original
   * channelActive.
   */
  @Test
  public void doesNotDecrementWhenChannelActiveWasNeverObserved() throws Exception {
    SPINiftyMetrics metrics = new SPINiftyMetrics();
    EmbeddedChannel channel = new EmbeddedChannel(false);
    channel.pipeline().addLast(new MetricsChannelDuplexHandler(metrics));
    // Note: register=false above means EmbeddedChannel does NOT auto-fire channelActive.

    assertEquals(0, metrics.getChannelCount());
    assertEquals(0, metrics.getAcceptedConnections());

    // Simulate the channel becoming inactive without a preceding channelActive (e.g. a defer-
    // active handler upstream swallowed it because the connection never reached the protocol-
    // decision point).
    channel.pipeline().fireChannelInactive();

    assertEquals(0, metrics.getChannelCount(), "gauge must not go negative on unmatched inactive");
    assertEquals(
        0,
        metrics.getDroppedConnections(),
        "dropped counter should not bump on unmatched inactive");
  }

  /** Repeated unmatched inactives are also safe. */
  @Test
  public void repeatedUnmatchedInactivesAreSafe() {
    SPINiftyMetrics metrics = new SPINiftyMetrics();
    EmbeddedChannel channel = new EmbeddedChannel(false);
    channel.pipeline().addLast(new MetricsChannelDuplexHandler(metrics));

    channel.pipeline().fireChannelInactive();
    channel.pipeline().fireChannelInactive();
    channel.pipeline().fireChannelInactive();

    assertEquals(0, metrics.getChannelCount());
    assertEquals(0, metrics.getDroppedConnections());
  }
}
