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

abstract class HeaderEvent {
  enum Type {
    ADD_TO_MAP,
    REMOVE_FROM_MAP,
    EMIT
  }

  private final Type type;
  private final long streamId;
  private final long timestamp;

  public HeaderEvent(Type type, long streamId) {
    this.type = type;
    this.streamId = streamId;
    this.timestamp = System.currentTimeMillis();
  }

  public Type getType() {
    return type;
  }

  public long getStreamId() {
    return streamId;
  }

  public long getTimestamp() {
    return timestamp;
  }
}
