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

package com.facebook.thrift.model;

import java.util.Objects;

public final class StreamResponse<T, K> {

  private static final StreamResponse EMPTY = new StreamResponse();

  private final T firstResponse;
  private final K data;
  private final boolean isFirstResponse;

  private StreamResponse(T firstResponse, K data) {
    this.firstResponse = firstResponse;
    this.data = data;
    this.isFirstResponse = firstResponse != null;
  }

  private StreamResponse() {
    this.isFirstResponse = false;
    this.firstResponse = null;
    this.data = null;
  }

  public static StreamResponse emptyResponse() {
    return EMPTY;
  }

  public static <T, K> StreamResponse<T, K> fromFirstResponse(T firstResponse) {
    return new StreamResponse<>(Objects.requireNonNull(firstResponse), null);
  }

  public static <T, K> StreamResponse<T, K> fromData(K data) {
    return new StreamResponse<>(null, Objects.requireNonNull(data));
  }

  public T getFirstResponse() {
    if (!this.isFirstResponse) {
      throw new IllegalStateException("Not a FirstResponse Element");
    }
    return firstResponse;
  }

  public boolean isSetFirstResponse() {
    return this.isFirstResponse;
  }

  public K getData() {
    if (this.isFirstResponse) {
      throw new IllegalStateException("Not a Data Element");
    }
    return data;
  }

  public boolean isSetData() {
    return !this.isFirstResponse;
  }
}
