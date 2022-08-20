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

import org.reactivestreams.Subscriber;
import reactor.core.Disposable;

interface HeaderEventBus extends Disposable {
  default void sendAddToMapEvent(long streamId, Subscriber<?> subscriber) {
    send(new AddToMap(streamId, subscriber));
  }

  default void sendRemoveFromMap(long streamId) {
    send(new RemoveFromMap(streamId));
  }

  default void sendEmit(long streamId, Object o) {
    send(new Emit(streamId, o));
  }

  void send(HeaderEvent headerEvent);
}
