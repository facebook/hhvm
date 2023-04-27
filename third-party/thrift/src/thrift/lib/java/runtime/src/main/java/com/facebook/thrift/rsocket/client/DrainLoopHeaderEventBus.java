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

package com.facebook.thrift.rsocket.client;

import io.netty.util.collection.LongObjectHashMap;
import java.util.Comparator;
import java.util.concurrent.ConcurrentSkipListSet;
import java.util.concurrent.atomic.AtomicIntegerFieldUpdater;
import org.reactivestreams.Subscriber;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

@SuppressWarnings({"unchecked", "rawtypes"})
public class DrainLoopHeaderEventBus implements HeaderEventBus {
  private static final Logger LOGGER = LoggerFactory.getLogger(DrainLoopHeaderEventBus.class);

  private static final AtomicIntegerFieldUpdater<DrainLoopHeaderEventBus> WIP =
      AtomicIntegerFieldUpdater.newUpdater(DrainLoopHeaderEventBus.class, "wip");

  private static final AtomicIntegerFieldUpdater<DrainLoopHeaderEventBus> DISPOSED =
      AtomicIntegerFieldUpdater.newUpdater(DrainLoopHeaderEventBus.class, "disposed");

  private final ConcurrentSkipListSet<HeaderEvent> events;
  private final LongObjectHashMap<Subscriber> subscribers;

  private volatile int disposed = 0;
  private volatile int wip;

  public DrainLoopHeaderEventBus() {
    this.events = new ConcurrentSkipListSet<>(Comparator.comparing(HeaderEvent::getTimestamp));
    this.subscribers = new LongObjectHashMap<>();
  }

  @Override
  public void send(HeaderEvent headerEvent) {
    if (isDisposed()) {
      throw new IllegalStateException("can not submit to the EventBus when it is disposed");
    }

    events.add(headerEvent);
    tryDrain();
  }

  private void tryDrain() {
    if (WIP.getAndIncrement(this) == 0) {
      drain();
    }
  }

  private void drain() {
    int missed = 1;
    do {
      if (isDisposed()) {
        return;
      }

      HeaderEvent event = events.first();
      if (event != null) {
        dispatchHeaderEvent(event);
      }

      missed = WIP.getAndAdd(this, -missed);
    } while (missed != 0);
  }

  private void dispatchHeaderEvent(HeaderEvent event) {
    switch (event.getType()) {
      case EMIT:
        if (subscribers.containsKey(event.getStreamId())) {
          emit((Emit) event);
        }
        break;
      case REMOVE_FROM_MAP:
        removeFromMap((RemoveFromMap) event);
        break;
      case ADD_TO_MAP:
        addToMap((AddToMap) event);
        break;
    }
  }

  private void addToMap(AddToMap addToMap) {
    events.pollFirst();
    subscribers.put(addToMap.getStreamId(), addToMap.getSubscriber());
  }

  private void emit(Emit emit) {
    try {
      Subscriber subscriber = subscribers.get(emit.getStreamId());
      if (subscriber != null) {
        events.pollFirst();
        subscriber.onNext(emit.getT());
      }
    } catch (Throwable t) {
      LOGGER.error("error occurred ", t);
    }
  }

  private void removeFromMap(RemoveFromMap removeFromMap) {
    events.pollFirst();
    events.removeIf(headerEvent -> headerEvent.getStreamId() == removeFromMap.getStreamId());
    subscribers.remove(removeFromMap.getStreamId());
  }

  @Override
  public void dispose() {
    DISPOSED.set(this, 1);
  }

  @Override
  public boolean isDisposed() {
    return disposed == 1;
  }
}
