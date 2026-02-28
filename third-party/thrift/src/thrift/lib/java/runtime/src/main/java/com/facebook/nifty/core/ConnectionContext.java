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
import java.net.SocketAddress;
import java.util.Iterator;
import java.util.Map;

public interface ConnectionContext {
  /**
   * Gets the remote address of the client that made the request
   *
   * @return The client's remote address as a {@link SocketAddress}
   */
  SocketAddress getRemoteAddress();

  /**
   * Gets the value of an additional attribute specific to the connection
   *
   * @param attributeName Name of attribute
   * @return Attribute value, or {@code null} if not present
   */
  Object getAttribute(String attributeName);

  /**
   * Sets the value of an additional attribute specific to the connection
   *
   * @param attributeName Name of attribute
   * @param value New value of attribute. Must not be {@code null}
   * @return Old attribute value, or {@code null} if not present
   */
  Object setAttribute(String attributeName, Object value);

  /**
   * Removes an additional attribute specific to the connection
   *
   * @param attributeName Name of attribute
   * @return Old attribute value, or {@code null} if attribute was not present
   */
  Object removeAttribute(String attributeName);

  /**
   * Returns a read-only iterator over the additional attributes
   *
   * @return Iterator
   */
  Iterator<Map.Entry<String, Object>> attributeIterator();

  SslSession getSslSession();
}
