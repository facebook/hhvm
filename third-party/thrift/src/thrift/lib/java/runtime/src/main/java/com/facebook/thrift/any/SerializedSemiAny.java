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

import com.facebook.thrift.standard_type.StandardProtocol;
import com.facebook.thrift.type_swift.AnyStruct;
import com.facebook.thrift.type_swift.ProtocolUnion;
import com.facebook.thrift.type_swift.SemiAnyStruct;
import com.facebook.thrift.type_swift.TypeStruct;
import java.util.Objects;

/**
 * Internal class to represent serialized thrift SemiAny. It will create the object in the payload
 * of {@link SemiAnyStruct} when the get method is called. Created from {@link SemiAny}.
 *
 * @param <T>
 */
class SerializedSemiAny<T> extends SemiAny<T> {

  private final SemiAnyStruct any;

  private transient Object lazyValue;

  SerializedSemiAny(SemiAnyStruct any) {
    super();
    this.any = Objects.requireNonNull(any);
  }

  /** @return SemiAny payload. Data, protocol and type must be provided to get the value. */
  public T get() {
    if (lazyValue != null) {
      return (T) lazyValue;
    }

    return getValue();
  }

  private <T> T getValue() {
    if (any.getData() == null || any.getProtocol() == null || any.getType() == null) {
      throw new IllegalStateException("Type, protocol and data must be provided to get the value");
    }
    synchronized (this) {
      if (lazyValue == null) {
        ThriftAnyDeserializer deserializer =
            new ThriftAnyDeserializer(any.getData(), any.getProtocol(), any.getType());
        lazyValue = deserializer.deserialize();
      }

      return (T) lazyValue;
    }
  }

  /** @return SemiAnyStruct */
  @Override
  public SemiAnyStruct getAny() {
    return any;
  }

  @Override
  public Any<T> promote() {
    SemiAnyStruct any = getAny();
    if (any.getType() == null || any.getProtocol() == null || any.getData() == null) {
      throw new IllegalStateException(
          "Can't promote to Any. Type, protocol and data must be provided");
    }
    return Any.wrap(
        new AnyStruct.Builder()
            .setType(any.getType())
            .setProtocol(any.getProtocol())
            .setData(any.getData())
            .build());
  }

  @Override
  public Any<T> promote(StandardProtocol protocol) {
    protocol = Objects.requireNonNull(protocol, "Protocol can not be null");

    SemiAnyStruct any = getAny();
    if (any.getType() == null || any.getData() == null) {
      throw new IllegalStateException("Can't promote to Any. Type and data must be provided");
    }
    return Any.wrap(
        new AnyStruct.Builder()
            .setType(any.getType())
            .setProtocol(ProtocolUnion.fromStandard(protocol))
            .setData(any.getData())
            .build());
  }

  @Override
  public Any<T> promote(ProtocolUnion protocolUnion) {
    protocolUnion = Objects.requireNonNull(protocolUnion, "ProtocolUnion can not be null");
    SemiAnyStruct any = getAny();
    if (any.getType() == null || any.getData() == null) {
      throw new IllegalStateException("Can't promote to Any. Type and data must be provided");
    }
    return Any.wrap(
        new AnyStruct.Builder()
            .setType(any.getType())
            .setProtocol(protocolUnion)
            .setData(any.getData())
            .build());
  }

  @Override
  public Any<T> promote(TypeStruct typeStruct, ProtocolUnion protocolUnion) {
    protocolUnion = Objects.requireNonNull(protocolUnion, "ProtocolUnion can not be null");
    typeStruct = Objects.requireNonNull(typeStruct, "TypeStruct can not be null");

    SemiAnyStruct any = getAny();
    if (any.getData() == null) {
      throw new IllegalStateException("Can't promote to Any. Data must be provided");
    }
    return Any.wrap(
        new AnyStruct.Builder()
            .setType(typeStruct)
            .setProtocol(protocolUnion)
            .setData(any.getData())
            .build());
  }
}
