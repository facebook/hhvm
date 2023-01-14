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

import com.facebook.thrift.type_swift.TypeStruct;
import io.netty.buffer.ByteBuf;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.function.BiFunction;
import java.util.function.Function;

/**
 * Base class for Any and SemiAny.
 *
 * @param <T> The type which Any and SemiAny holds
 * @param <A> AnyStruct or SemiAnyStruct
 */
abstract class AbstractAny<T, A> {

  /** Map for custom serializer */
  static final Map<String, Function<Object, ByteBuf>> serializerUri = new ConcurrentHashMap<>();

  /** Map for custom deserializer */
  static final Map<String, BiFunction<TypeStruct, ByteBuf, Object>> deserializerUri =
      new ConcurrentHashMap<>();

  /** Map for custom serializer */
  static final Map<Long, Function<Object, ByteBuf>> serializerId = new ConcurrentHashMap<>();

  /** Map for custom deserializer */
  static final Map<Long, BiFunction<TypeStruct, ByteBuf, Object>> deserializerId =
      new ConcurrentHashMap<>();

  /**
   * Registers a serializer for a custom protocol with uri. Before using a customer protocol, a
   * serializer must be registered.
   *
   * @param uri Uri of the custom protocol
   * @param serializer Serializer as a {@link Function} which serialize a given Object to a {@link
   *     ByteBuf}
   */
  public static void registerSerializer(String uri, Function<Object, ByteBuf> serializer) {
    serializerUri.put(uri, serializer);
  }

  /**
   * Registers a serializer for a custom protocol with external id. Before using a customer
   * protocol, a serializer must be registered.
   *
   * @param id External id of the custom protocol
   * @param serializer Serializer as a {@link Function} which serialize a given Object to a {@link
   *     ByteBuf}
   */
  public static void registerSerializer(long id, Function<Object, ByteBuf> serializer) {
    serializerId.put(id, serializer);
  }

  /**
   * Registers a deserializer for a custom protocol with uri. Before using a customer protocol, a
   * deserializer must be registered.
   *
   * @param uri Uri of the custom protocol
   * @param deserializer Deserializer as a {@link BiFunction} which creates an Object from a given
   *     Class name and serialized {@link ByteBuf}.
   */
  public static void registerDeserializer(
      String uri, BiFunction<TypeStruct, ByteBuf, Object> deserializer) {
    deserializerUri.put(uri, deserializer);
  }

  /**
   * Registers a deserializer for a custom protocol with external id. Before using a customer
   * protocol, a deserializer must be registered.
   *
   * @param id External id of the custom protocol
   * @param deserializer Deserializer as a {@link BiFunction} which creates an Object from a given
   *     Class name and serialized {@link ByteBuf}.
   */
  public static void registerDeserializer(
      long id, BiFunction<TypeStruct, ByteBuf, Object> deserializer) {
    deserializerId.put(id, deserializer);
  }

  /** @return Returns payload of Any and SemiAny */
  public abstract T get();

  /** @return Returns AnyStruct or SemiAnyStruct */
  public abstract A getAny();

  @Override
  public final int hashCode() {
    return getAny().hashCode();
  }

  @Override
  public final boolean equals(Object obj) {
    if (obj == this) {
      return true;
    }

    if (!(obj instanceof AbstractAny)) {
      return false;
    }

    return getAny().equals(((AbstractAny) obj).getAny());
  }
}
