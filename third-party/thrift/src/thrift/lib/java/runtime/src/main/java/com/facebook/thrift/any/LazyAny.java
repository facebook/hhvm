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

import com.facebook.thrift.payload.ThriftSerializable;
import com.facebook.thrift.payload.Writer;
import com.facebook.thrift.protocol.ByteBufTProtocol;
import com.facebook.thrift.type.HashAlgorithmSHA256;
import com.facebook.thrift.type.Type;
import com.facebook.thrift.type.TypeRegistry;
import com.facebook.thrift.type.UniversalName;
import com.facebook.thrift.util.SerializationProtocol;
import com.facebook.thrift.util.SerializerUtil;
import com.facebook.thrift.util.resources.RpcResources;
import com.google.common.base.Strings;
import io.netty.buffer.ByteBuf;
import java.util.Map;
import java.util.Objects;
import java.util.concurrent.ConcurrentHashMap;
import java.util.function.BiFunction;
import java.util.function.Function;
import org.apache.thrift.conformance.Any;
import org.apache.thrift.conformance.StandardProtocol;

/** LazyAny implementation that contains Any object. Any is created via Builder object. */
public abstract class LazyAny<T> {

  /** Map for custom serializer */
  protected static final Map<String, Function<Object, ByteBuf>> serializers =
      new ConcurrentHashMap<>();

  /** Map for custom deserializer */
  protected static final Map<String, BiFunction<Class, ByteBuf, Object>> deserializers =
      new ConcurrentHashMap<>();

  /**
   * Registers a serializer for a custom protocol. Before using a customer protocol, a serializer
   * must be registered.
   *
   * @param name Name of the custom protocol
   * @param serializer Serializer as a {@link Function} which serialize a given Object to a {@link
   *     ByteBuf}
   */
  public static void registerSerializer(String name, Function<Object, ByteBuf> serializer) {
    serializers.put(name, serializer);
  }

  /**
   * Registers a deserializer for a custom protocol. Before using a customer protocol, a
   * deserializer must be registered.
   *
   * @param name Name of the custom protocol
   * @param deserializer Deserializer as a {@link BiFunction} which creates an Object from a given
   *     Class name and serialized {@link ByteBuf}.
   */
  public static void registerDeserializer(
      String name, BiFunction<Class, ByteBuf, Object> deserializer) {
    deserializers.put(name, deserializer);
  }

  public static LazyAny wrap(Any any) {
    return new SerializedLazyAny(any);
  }

  public abstract <T> T get();

  public abstract Any getAny();

  @Override
  public final int hashCode() {
    return getAny().hashCode();
  }

  @Override
  public final boolean equals(Object obj) {
    if (obj == this) {
      return true;
    }

    if (!(obj instanceof LazyAny)) {
      return false;
    }

    Any any = getAny();
    LazyAny other = (LazyAny) obj;
    return any.equals(other.getAny());
  }

  public static class Builder<T> {
    private T value;

    private String customProtocol;

    private SerializationProtocol serializationProtocol = SerializationProtocol.TCompact;

    private Function<Object, ByteBuf> serializer;

    private boolean useHashPrefix;

    private boolean useUri;

    private int numberOfBytes = HashAlgorithmSHA256.INSTANCE.getMinHashBytes();

    /** LazyAny builder */
    public Builder() {}

    public Builder(T value) {
      this.value = value;
    }

    /** Payload of Any */
    public Builder<T> setValue(T value) {
      this.value = value;
      return this;
    }

    /**
     * Serialization protocol of Any payload when standard protocol is used. When set, custom
     * protocol should not be set.
     */
    public Builder<T> setProtocol(SerializationProtocol serializationProtocol) {
      this.serializationProtocol = serializationProtocol;
      return this;
    }

    /**
     * Custom serialization protocol of Any payload. When set, serialization protocol should not be
     * set.
     */
    public Builder<T> setCustomProtocol(String customProtocol) {
      this.customProtocol = customProtocol;
      return this;
    }

    /**
     * Any uses 8 bytes of hash for the object type. This is default behavior when not set. Either
     * useHash or useUri must be set.
     */
    public Builder<T> useHashPrefix() {
      this.useHashPrefix = true;
      return this;
    }

    /**
     * Any uses numberOfBytes bytes of hash for the object type. Either useHash or useUri must be
     * set.
     */
    public Builder<T> useHashPrefix(int numberOfBytes) {
      this.useHashPrefix = true;
      this.numberOfBytes = numberOfBytes;
      return this;
    }

    /** Any uses uri for the object type. Either useHash or useUri must be set. */
    public Builder<T> useUri() {
      this.useUri = true;
      return this;
    }

    public LazyAny build() {
      Objects.requireNonNull(value, "Must have a value");
      Type type = TypeRegistry.findByClass(value.getClass());
      if (type == null) {
        throw new IllegalArgumentException("No type found for value " + value.getClass());
      }

      if (useHashPrefix && useUri) {
        throw new IllegalStateException("Must set either the useHashPrefix or useUri, not both");
      }

      if (!useHashPrefix && !useUri) {
        UniversalName un = type.getUniversalName();
        if (un.preferHash(numberOfBytes)) {
          useHashPrefix = true;
        } else {
          useUri = true;
        }
      }

      if (numberOfBytes < 1) {
        throw new IllegalArgumentException("When using hash prefix must select one or more bytes");
      }

      if (!Strings.isNullOrEmpty(customProtocol) && !serializers.containsKey(customProtocol)) {
        throw new IllegalArgumentException(
            "When using a custom protocol you must provide a serializer");
      }

      final StandardProtocol standardProtocol;
      if (Strings.isNullOrEmpty(customProtocol)) {
        if (serializationProtocol == null) {
          this.serializationProtocol = SerializationProtocol.TCompact;
        }
        switch (serializationProtocol) {
          case TCompact:
            standardProtocol = StandardProtocol.COMPACT;
            break;
          case TJSON:
            standardProtocol = StandardProtocol.JSON;
            break;
          case TBinary:
            standardProtocol = StandardProtocol.BINARY;
            break;
          case TSimpleJSONBase64:
          case TSimpleJSON:
            standardProtocol = StandardProtocol.SIMPLE_JSON;
            break;
          default:
            throw new IllegalArgumentException(
                "Unknown serialization protocol " + serializationProtocol);
        }

        if (value instanceof ThriftSerializable) {
          serializer =
              generateSerializer(((ThriftSerializable) value)::write0, serializationProtocol);
        } else if (value instanceof Writer) {
          serializer = generateSerializer((Writer) value, serializationProtocol);
        } else if (serializer == null) {
          throw new IllegalArgumentException(
              "You must register a serializer if the value is not of ThriftSerializable or Writer");
        }
      } else {
        standardProtocol = StandardProtocol.CUSTOM;
        serializer = serializers.get(customProtocol);
      }

      return new UnserializedLazyAny(
          value,
          customProtocol,
          standardProtocol,
          type,
          serializer,
          useHashPrefix,
          useUri,
          numberOfBytes);
    }
  }

  private static Function<Object, ByteBuf> generateSerializer(
      Writer w, SerializationProtocol serializationProtocol) {
    return o -> {
      ByteBuf dest = RpcResources.getUnpooledByteBufAllocator().buffer(1024, 1 << 24);
      ByteBufTProtocol protocol = SerializerUtil.toByteBufProtocol(serializationProtocol, dest);
      w.write(protocol);

      return dest;
    };
  }
}
