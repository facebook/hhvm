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

/**
 * Internal class to represent unserialized thrift Any. It will create an instance of {@link
 * AnyStruct} with serialized data when the getAny method is called. Created from {@link Any}.
 *
 * @param <T>
 */
class UnserializedAny<T> extends Any<T> {

  private Any.Builder builder;
  private transient AnyStruct any;

  public UnserializedAny(Any.Builder<T> builder) {
    this.builder = builder;
  }

  @Override
  public T get() {
    return (T) builder.value;
  }

  @Override
  public AnyStruct getAny() {
    if (any == null) {
      synchronized (this) {
        if (any == null) {
          any = buildAny();
        }
      }
    }

    return any;
  }

  private AnyStruct buildAny() {

    ThriftAnySerializer serializer = new ThriftAnySerializer(builder);
    serializer.serialize(builder.value);

    return new AnyStruct.Builder()
        .setType(serializer.getTypeStruct())
        .setProtocol(serializer.getProtocolUnion())
        .setData(serializer.getBuffer())
        .build();
  }
}
