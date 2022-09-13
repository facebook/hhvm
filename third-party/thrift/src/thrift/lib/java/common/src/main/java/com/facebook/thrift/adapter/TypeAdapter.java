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

import com.facebook.thrift.adapter.common.EnumTypeAdapter;
import com.facebook.thrift.adapter.common.ListTypeAdapter;
import com.facebook.thrift.adapter.common.MapTypeAdapter;
import com.facebook.thrift.adapter.common.SetTypeAdapter;

/**
 * Base interface for Type Adapters. TypeAdapters can be applied to typedef, struct or a struct
 * field.
 *
 * @param <T> Type, This is a thrift type when type is read from thrift IO stream during encoding or
 *     an adapted type when a Type Adapter is composed from another Adapter.
 *     <p>Note that most of the types are mapped to equivalent Boxed java types, but binary is
 *     mapped to ByteBuf. Mapping binary to ByteBuf allow zero-copy of the binary data and pass it
 *     to the adapter.
 *     <p>When adapting a thrift type, these type interfaces can be used as base. If one of the
 *     interfaces doesn’t work you can implement TypeAdapter directly.
 *     <ul>
 *       <li>{@link com.facebook.thrift.adapter.common.BooleanTypeAdapter BooleanTypeAdapter}
 *       <li>{@link com.facebook.thrift.adapter.common.ByteTypeAdapter ByteTypeAdapter}
 *       <li>{@link com.facebook.thrift.adapter.common.ShortTypeAdapter ShortTypeAdapter}
 *       <li>{@link com.facebook.thrift.adapter.common.IntegerTypeAdapter IntegerTypeAdapter}
 *       <li>{@link com.facebook.thrift.adapter.common.LongTypeAdapter LongTypeAdapter}
 *       <li>{@link com.facebook.thrift.adapter.common.FloatTypeAdapter FloatTypeAdapter}
 *       <li>{@link com.facebook.thrift.adapter.common.DoubleTypeAdapter DoubleTypeAdapter}
 *       <li>{@link com.facebook.thrift.adapter.common.StringTypeAdapter StringTypeAdapter}
 *       <li>{@link com.facebook.thrift.adapter.common.BinaryStringTypeAdapter
 *           BinaryStringTypeAdapter}
 *       <li>{@link EnumTypeAdapter EnumAdapter}
 *       <li>{@link MapTypeAdapter MapAdapter}
 *       <li>{@link ListTypeAdapter ListAdapter}
 *       <li>{@link SetTypeAdapter SetAdapter}
 *       <li>{@link com.facebook.thrift.adapter.common.StructTypeAdapter StructTypeAdapter}
 *       <li>{@link com.facebook.thrift.adapter.common.CopiedPooledByteBufTypeAdapter
 *           CopiedPooledByteBufTypeAdapter}
 *       <li>{@link com.facebook.thrift.adapter.common.UnpooledByteBufTypeAdapter
 *           UnpooledByteBufTypeAdapter}
 *       <li>{@link com.facebook.thrift.adapter.common.RetainedSlicedPooledByteBufTypeAdapter
 *           RetainedSlicedPooledByteBufTypeAdapter}
 *     </ul>
 *
 * @param <P> Adapted type, i.e java.util.Date
 */
public interface TypeAdapter<T, P> extends Adapter<P> {

  /**
   * Converts given type to the adapted type.
   *
   * @param t Thrift type or an adapted type.
   * @return Adapted type
   */
  P fromThrift(T t);

  /**
   * Converts adapted type to the original type.
   *
   * @param p Adapted type
   * @return Thrift type or an adapted type
   */
  T toThrift(P p);
}
