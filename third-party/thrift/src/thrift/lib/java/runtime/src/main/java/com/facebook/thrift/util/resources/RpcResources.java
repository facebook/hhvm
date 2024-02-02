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

package com.facebook.thrift.util.resources;

import com.facebook.swift.service.ThriftMeterRegistry;
import io.netty.buffer.ByteBufAllocator;
import io.netty.buffer.UnpooledByteBufAllocator;
import io.netty.channel.EventLoopGroup;
import io.netty.util.HashedWheelTimer;
import java.util.Map;
import java.util.concurrent.atomic.AtomicReferenceFieldUpdater;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import reactor.core.publisher.Mono;
import reactor.core.scheduler.Scheduler;

/**
 * Facade for accessing resources used by Thrift Java. Call shutdown to cleanup the resources being
 * used
 */
public final class RpcResources {
  private static final Logger LOGGER = LoggerFactory.getLogger(RpcResources.class);
  private static final AtomicReferenceFieldUpdater<RpcResources, State> stateUpdater =
      AtomicReferenceFieldUpdater.newUpdater(RpcResources.class, State.class, "state");

  private static final RpcResources INSTANCE = new RpcResources();

  private volatile State state = State.CLOSED;
  private volatile ResourcesHolder holder;

  private enum State {
    RUNNING,
    CLOSED,
  }

  private RpcResources() {}

  // TODO: remove when rolled out
  public static boolean enableOperatorFusion() {
    return ResourceConfiguration.enableOperatorFusion;
  }

  public static HashedWheelTimer getHashedWheelTimer() {
    return INSTANCE.doGet().getTimer();
  }

  public static EventLoopGroup getEventLoopGroup() {
    return INSTANCE.doGet().getEventLoopGroup();
  }

  public static int getEventLoopGroupPendingTasks() {
    return INSTANCE.doGet().pendingTasksForEventLoop();
  }

  public static int getNumEventLoopThreads() {
    return INSTANCE.doGet().getNumThreadsForEventLoop();
  }

  public static Scheduler getOffLoopScheduler() {
    return INSTANCE.doGet().getOffLoopScheduler();
  }

  public static Scheduler getClientOffLoopScheduler() {
    return INSTANCE.doGet().getClientOffLoopScheduler();
  }

  public static Map<String, Long> getOffLoopSchedulerStats() {
    return INSTANCE.doGet().stats();
  }

  public static ByteBufAllocator getByteBufAllocator() {
    return ByteBufAllocator.DEFAULT;
  }

  public static UnpooledByteBufAllocator getUnpooledByteBufAllocator() {
    return UnpooledByteBufAllocator.DEFAULT;
  }

  public static boolean isForceExecutionOffEventLoop() {
    return INSTANCE.doGet().isForceExecutionOffEventLoop();
  }

  public static boolean isForceClientExecutionOffEventLoop() {
    return INSTANCE.doGet().isForceClientExecutionOffEventLoop();
  }

  public static ThriftMeterRegistry getThriftMeterRegistry() {
    return ThriftMeterRegistry.getInstance();
  }

  public static Mono<Void> onShutdown() {
    return INSTANCE.holder.onClose();
  }

  public static void shutdown() {
    INSTANCE.doShutdown();
  }

  public ResourcesHolder doGet() {
    ResourcesHolder h = null;
    do {
      if (state == State.CLOSED) {
        if (stateUpdater.compareAndSet(this, State.CLOSED, State.RUNNING)) {
          holder = new ResourcesHolder();
        } else {
          continue;
        }
      }

      h = holder;
    } while (h == null);

    return h;
  }

  public void doShutdown() {
    if (state == State.RUNNING && stateUpdater.compareAndSet(this, State.RUNNING, State.CLOSED)) {
      try {
        LOGGER.info("attempt to shut down RpcResources.");
        holder.dispose();
      } catch (Throwable t) {
        LOGGER.error("Error shutting down RpcResources", t);
      }
    }
  }
}
