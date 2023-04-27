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

package com.facebook.thrift.client;

import java.nio.charset.StandardCharsets;
import java.util.HashMap;
import java.util.Map;

public class DefaultResponseWrapper<T> implements ResponseWrapper {

  private T data;
  private Map<String, String> headers;
  private Map<String, byte[]> binaryHeaders;

  public DefaultResponseWrapper(T data, Map<String, String> headers) {
    this.data = data;
    this.headers = headers;
  }

  public DefaultResponseWrapper(
      T data, Map<String, String> headers, Map<String, byte[]> binaryHeaders) {
    this.data = data;
    this.headers = headers;
    this.binaryHeaders = binaryHeaders;
  }

  public T getData() {
    return data;
  }

  public Map<String, String> getHeaders() {
    return headers;
  }

  @Override
  public Map<String, byte[]> getBinaryHeaders() {
    if (binaryHeaders == null) {
      binaryHeaders = new HashMap<>();
      headers.forEach((s, s2) -> binaryHeaders.put(s, s2.getBytes(StandardCharsets.UTF_8)));
    }

    return binaryHeaders;
  }
}
