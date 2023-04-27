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

package com.facebook.thrift.util;

import com.facebook.nifty.core.ConnectionContext;
import com.facebook.nifty.core.RequestContext;
import io.netty.util.internal.PlatformDependent;
import java.util.Collections;
import java.util.Iterator;
import java.util.Map;
import java.util.Objects;

@Deprecated
public class NettyNiftyRequestContext implements RequestContext {
  private final Map<String, Object> data;
  private final Map<String, String> requestHeaders;
  private final Map<String, String> responseHeaders;
  private final ConnectionContext connectionContext;

  public NettyNiftyRequestContext(
      Map<String, String> requestHeaders, ConnectionContext connectionContext) {
    this.data = PlatformDependent.newConcurrentHashMap();
    this.requestHeaders = requestHeaders;
    this.responseHeaders = PlatformDependent.newConcurrentHashMap();
    this.connectionContext = connectionContext;
  }

  @Override
  public ConnectionContext getConnectionContext() {
    return connectionContext;
  }

  @Override
  public void setContextData(String key, Object val) {
    Objects.requireNonNull(key, "context data key is null");
    data.put(key, val);
  }

  @Override
  public Object getContextData(String key) {
    Objects.requireNonNull(key, "context data key is null");
    return data.get(key);
  }

  @Override
  public void clearContextData(String key) {
    Objects.requireNonNull(key, "context data key is null");
    data.remove(key);
  }

  @Override
  public Iterator<Map.Entry<String, Object>> contextDataIterator() {
    return Collections.unmodifiableSet(data.entrySet()).iterator();
  }

  @Override
  public Map<String, String> getRequestHeader() {
    return requestHeaders;
  }

  @Override
  public void setResponseHeader(String key, String value) {
    responseHeaders.put(key, value);
  }

  public Map<String, String> getResponseHeaders() {
    return responseHeaders;
  }
}
