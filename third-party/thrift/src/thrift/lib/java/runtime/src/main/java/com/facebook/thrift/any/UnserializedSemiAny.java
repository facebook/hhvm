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
import com.facebook.thrift.type_swift.ProtocolUnion;
import com.facebook.thrift.type_swift.SemiAnyStruct;
import com.facebook.thrift.type_swift.TypeStruct;
import java.util.Objects;

/**
 * Internal class to represent unserialized thrift {@link SemiAny}. It will create an instance of
 * {@link SemiAnyStruct} with serialized data when the getAny method is called. Created from {@link
 * SemiAny}.
 *
 * @param <T>
 */
class UnserializedSemiAny<T> extends SemiAny<T> {

  private SemiAny.Builder builder;
  private transient SemiAnyStruct any;

  public UnserializedSemiAny(SemiAny.Builder<T> builder) {
    this.builder = builder;
  }

  @Override
  public T get() {
    return (T) builder.value;
  }

  @Override
  public SemiAnyStruct getAny() {
    if (any == null
        || any.getData() == null
        || any.getProtocol() == null
        || any.getType() == null) {
      synchronized (this) {
        if (any == null) {
          any = buildAny();
        }
      }
    }

    return any;
  }

  private SemiAnyStruct buildAny() {

    ThriftAnySerializer serializer = new ThriftAnySerializer(builder);
    serializer.serialize(builder.value);

    return new SemiAnyStruct.Builder()
        .setType(serializer.getTypeStruct())
        .setProtocol(serializer.getProtocolUnion())
        .setData(serializer.getBuffer())
        .build();
  }

  private Any.Builder<T> createAnyBuilder() {
    Any.Builder<T> anyBuilder = new Any.Builder<>((T) builder.value);
    anyBuilder.standardProtocol = builder.standardProtocol;
    anyBuilder.useHashPrefix = builder.useHashPrefix;
    anyBuilder.useUri = builder.useUri;
    anyBuilder.customProtocolId = builder.customProtocolId;
    anyBuilder.customProtocolUri = builder.customProtocolUri;
    anyBuilder.elementTypeList = builder.elementTypeList;
    anyBuilder.numberOfBytes = builder.numberOfBytes;
    anyBuilder.typeStruct = builder.typeStruct;

    return anyBuilder;
  }

  @Override
  public Any<T> promote() {
    return createAnyBuilder().build();
  }

  @Override
  public Any<T> promote(StandardProtocol protocol) {
    protocol = Objects.requireNonNull(protocol, "Protocol can not be null");
    Any.Builder<T> anyBuilder = createAnyBuilder();
    anyBuilder.standardProtocol = protocol;

    return anyBuilder.build();
  }

  private void setProtocol(Any.Builder<T> anyBuilder, ProtocolUnion protocolUnion) {
    if (protocolUnion.isSetStandard()) {
      anyBuilder.standardProtocol = protocolUnion.getStandard();
    } else if (protocolUnion.isSetCustom()) {
      anyBuilder.customProtocolUri = protocolUnion.getCustom();
    } else if (protocolUnion.isSetId()) {
      anyBuilder.customProtocolId = protocolUnion.getId();
    }
  }

  @Override
  public Any<T> promote(ProtocolUnion protocolUnion) {
    protocolUnion = Objects.requireNonNull(protocolUnion, "ProtocolUnion can not be null");
    Any.Builder<T> anyBuilder = createAnyBuilder();
    setProtocol(anyBuilder, protocolUnion);

    return anyBuilder.build();
  }

  @Override
  public Any<T> promote(TypeStruct typeStruct, ProtocolUnion protocolUnion) {
    protocolUnion = Objects.requireNonNull(protocolUnion, "ProtocolUnion can not be null");
    typeStruct = Objects.requireNonNull(typeStruct, "TypeStruct can not be null");
    Any.Builder<T> anyBuilder = createAnyBuilder();
    setProtocol(anyBuilder, protocolUnion);
    anyBuilder.typeStruct = typeStruct;

    return anyBuilder.build();
  }
}
