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

public interface Wrapper<T, P, R> extends Adapter<P> {

  /**
   * Converts given type to the wrapped type.
   *
   * @param t Thrift type or an adapted type.
   * @param fieldContext field context including field id and root object
   * @return Wrapped type
   */
  P fromThrift(T t, FieldContext<R> fieldContext);

  /**
   * Converts wrapped type to the original type.
   *
   * @param p Wrapped type
   * @return Thrift type or an adapted type
   */
  T toThrift(P p);
}
