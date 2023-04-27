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

package org.apache.thrift;

import java.util.Collections;
import java.util.Map;

/** Generic exception class for Thrift. */
public abstract class TBaseException extends RuntimeException {
  protected Map<String, String> headers = Collections.emptyMap();

  private static final long serialVersionUID = 1L;

  public TBaseException() {
    super();
  }

  public TBaseException(String message) {
    super(message);
  }

  public TBaseException(Throwable cause) {
    super(cause);
  }

  public TBaseException(String message, Throwable cause) {
    super(message, cause);
  }

  public void setHeaders(Map<String, String> headers) {
    this.headers = headers;
  }

  public Map<String, String> headers() {
    return headers;
  }
}
