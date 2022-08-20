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

import com.facebook.thrift.type.Type;
import io.netty.buffer.ByteBuf;
import java.util.Objects;
import java.util.function.Function;
import org.apache.thrift.conformance.Any;
import org.apache.thrift.conformance.StandardProtocol;

/**
 * This a {@link LazyAny} that contains an unserialized struct. It will create an instance of Any
 * with serialized data when the getAny method is called.
 */
public class UnserializedLazyAny<T> extends LazyAny<T> {
  private final Object value;

  private final String customProtocol;

  private final StandardProtocol standardProtocol;

  private final Type type;

  private final Function<Object, ByteBuf> serializer;

  private final boolean useHashPrefix;

  private final boolean useUri;

  private final int numberOfBytes;

  private transient Any any;

  UnserializedLazyAny(
      Object value,
      String customProtocol,
      StandardProtocol standardProtocol,
      Type type,
      Function<Object, ByteBuf> serializer,
      boolean useHashPrefix,
      boolean useUri,
      int numberOfBytes) {
    this.value = Objects.requireNonNull(value);
    this.customProtocol = customProtocol;
    this.standardProtocol = Objects.requireNonNull(standardProtocol);
    this.type = Objects.requireNonNull(type);
    this.serializer = serializer;
    this.useHashPrefix = useHashPrefix;
    this.useUri = useUri;
    this.numberOfBytes = numberOfBytes;
  }

  @Override
  @SuppressWarnings("unchecked")
  public <T> T get() {
    return (T) value;
  }

  @Override
  public Any getAny() {
    if (any == null) {
      synchronized (this) {
        if (any == null) {
          any = buildAny();
        }
      }
    }

    return any;
  }

  private Any buildAny() {
    ByteBuf data = serializer.apply(value);

    Any.Builder builder = new Any.Builder().setProtocol(standardProtocol).setData(data);

    if (standardProtocol == StandardProtocol.CUSTOM) {
      builder.setCustomProtocol(customProtocol);
    }

    if (useUri) {
      builder.setType(type.getUniversalName().getUri());
    }

    if (useHashPrefix) {
      builder.setTypeHashPrefixSha2256(type.getUniversalName().getHashPrefixBytes(numberOfBytes));
    }

    return builder.build();
  }
}
