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

import io.netty.util.internal.PlatformDependent;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import org.reactivestreams.Subscriber;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import reactor.core.Disposable;
import reactor.core.Exceptions;
import reactor.core.publisher.FluxProcessor;
import reactor.core.publisher.UnicastProcessor;

@SuppressWarnings({"unchecked", "rawtypes"})
public class DefaultHeaderEventBus implements HeaderEventBus {
  private final Logger LOGGER = LoggerFactory.getLogger(DefaultHeaderEventBus.class);
  private final FluxProcessor<HeaderEvent, HeaderEvent> processor;
  private final Disposable disposable;
  private final Map<Long, Subscriber> subscribers;

  public DefaultHeaderEventBus() {
    this.processor =
        UnicastProcessor.<HeaderEvent>create(PlatformDependent.newMpscQueue()).serialize();
    this.disposable = init();
    this.subscribers = new ConcurrentHashMap<>();
  }

  private Disposable init() {
    return processor.doOnNext(this::dispatchHeaderEvent).retry().subscribe();
  }

  @Override
  public void dispose() {
    processor.dispose();
    disposable.dispose();
  }

  @Override
  public boolean isDisposed() {
    return processor.isDisposed();
  }

  private void dispatchHeaderEvent(HeaderEvent headerEvent) {
    switch (headerEvent.getType()) {
      case EMIT:
        emit((Emit) headerEvent);
        break;
      case ADD_TO_MAP:
        addToMap((AddToMap) headerEvent);
        break;
      case REMOVE_FROM_MAP:
        removeFromMap((RemoveFromMap) headerEvent);
        break;
    }
  }

  private void addToMap(AddToMap addToMap) {
    subscribers.put(addToMap.getStreamId(), addToMap.getSubscriber());
  }

  private void emit(Emit emit) {
    try {
      Subscriber subscriber = subscribers.get(emit.getStreamId());
      if (subscriber != null) {
        subscriber.onNext(emit.getT());
      }
    } catch (Throwable t) {
      LOGGER.error("error occurred ", t);
    }
  }

  private void removeFromMap(RemoveFromMap removeFromMap) {
    subscribers.remove(removeFromMap.getStreamId());
  }

  @Override
  public void send(HeaderEvent headerEvent) {
    try {
      processor.onNext(headerEvent);
    } catch (Throwable t) {
      LOGGER.error("error sending header event", t);
      throw Exceptions.propagate(t);
    }
  }
}
