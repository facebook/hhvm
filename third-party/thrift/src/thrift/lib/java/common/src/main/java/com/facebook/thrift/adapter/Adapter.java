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

/**
 * Base Adapter interface for {@link TypeAdapter TypeAdapter} and FieldAdapter
 *
 * @param <T>
 */
public interface Adapter<T> {

  /**
   * Compares given types.
   *
   * @param t1 Type
   * @param t2 Type
   * @return true if both types are equal.
   */
  default boolean equals(T t1, T t2) {
    return Objects.equals(t1, t2);
  }
}
