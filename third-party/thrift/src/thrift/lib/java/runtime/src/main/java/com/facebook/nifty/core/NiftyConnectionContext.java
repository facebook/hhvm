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

package com.facebook.nifty.core;

import com.facebook.nifty.ssl.SslSession;
import com.google.common.base.Preconditions;
import java.net.SocketAddress;
import java.util.Collections;
import java.util.Iterator;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

public class NiftyConnectionContext implements ConnectionContext {
  private SocketAddress remoteAddress;
  private SslSession sslSession;
  private Map<String, Object> attributes = new ConcurrentHashMap<>();

  @Override
  public SocketAddress getRemoteAddress() {
    return remoteAddress;
  }

  public SslSession getSslSession() {
    return sslSession;
  }

  public void setSslSession(SslSession sslSession) {
    this.sslSession = sslSession;
  }

  public void setRemoteAddress(SocketAddress remoteAddress) {
    this.remoteAddress = remoteAddress;
  }

  @Override
  public Object getAttribute(String attributeName) {
    Preconditions.checkNotNull(attributeName);
    return attributes.get(attributeName);
  }

  @Override
  public Object setAttribute(String attributeName, Object value) {
    Preconditions.checkNotNull(attributeName);
    Preconditions.checkNotNull(value);
    return attributes.put(attributeName, value);
  }

  @Override
  public Object removeAttribute(String attributeName) {
    Preconditions.checkNotNull(attributeName);
    return attributes.remove(attributeName);
  }

  @Override
  public Iterator<Map.Entry<String, Object>> attributeIterator() {
    return Collections.unmodifiableSet(attributes.entrySet()).iterator();
  }
}
