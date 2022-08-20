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

package com.facebook.thrift.adapter;

import java.util.Objects;
import org.apache.thrift.protocol.TProtocol;

/**
 * A Thrift Type Adapter in Java.
 *
 * @param <T>
 */
public interface TypeAdapter<T> {
  void toThrift(T t, TProtocol protocol);

  T fromThrift(TProtocol protocol);

  default boolean equals(T t1, T t2) {
    return Objects.equals(t1, t2);
  }
}
