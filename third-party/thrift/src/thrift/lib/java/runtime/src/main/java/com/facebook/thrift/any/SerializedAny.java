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

package com.facebook.thrift.any;

import com.facebook.thrift.type_swift.AnyStruct;
import java.util.Objects;

/**
 * Internal class to represent serialized thrift Any. It will create the object in the payload of
 * {@link AnyStruct} when the get method is called. Created from {@link Any}.
 *
 * @param <T>
 */
class SerializedAny<T> extends Any<T> {

  private final AnyStruct any;

  private transient Object lazyValue;

  SerializedAny(AnyStruct any) {
    this.any = Objects.requireNonNull(any);
  }

  /**
   * @return Any payload
   */
  public T get() {
    if (lazyValue != null) {
      return (T) lazyValue;
    }

    return getValue();
  }

  private <T> T getValue() {
    synchronized (this) {
      if (lazyValue == null) {
        ThriftAnyDeserializer deserializer =
            new ThriftAnyDeserializer(any.getData(), any.getProtocol(), any.getType());
        lazyValue = deserializer.deserialize();
      }

      return (T) lazyValue;
    }
  }

  /**
   * @return AnyStruct
   */
  @Override
  public AnyStruct getAny() {
    return any;
  }
}
