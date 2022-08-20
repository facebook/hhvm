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

package com.facebook.thrift.adapter.common;

import com.facebook.thrift.adapter.TypeAdapter;
import com.facebook.thrift.protocol.ByteBufTProtocol;
import org.apache.thrift.protocol.TProtocol;

/**
 * A Thrift Type Adapter exposes {@link ByteBufTProtocol protocol} directly
 *
 * @param <T>
 */
public interface ByteBufTProtocolTypeAdapter<T> extends TypeAdapter<T> {
  @Override
  default void toThrift(T t, TProtocol protocol) {
    toThrift(t, (ByteBufTProtocol) protocol);
  }

  void toThrift(T t, ByteBufTProtocol protocol);

  @Override
  default T fromThrift(TProtocol protocol) {
    return fromThrift((ByteBufTProtocol) protocol);
  }

  T fromThrift(ByteBufTProtocol protocol);
}
