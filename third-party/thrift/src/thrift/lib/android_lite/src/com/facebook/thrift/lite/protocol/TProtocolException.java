/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

package com.facebook.thrift.lite.protocol;

import java.io.IOException;

/** Protocol exceptions. */
public class TProtocolException extends IOException {

  public static final int UNKNOWN = 0;
  public static final int INVALID_DATA = 1;
  public static final int NEGATIVE_SIZE = 2;
  public static final int SIZE_LIMIT = 3;
  public static final int BAD_VERSION = 4;
  public static final int NOT_IMPLEMENTED = 5;
  public static final int MISSING_REQUIRED_FIELD = 6;

  private static final long serialVersionUID = -2623086986841580219L;

  protected int type = UNKNOWN;

  public TProtocolException() {
    this(UNKNOWN);
  }

  public TProtocolException(int t) {
    super();
    type = t;
  }

  public TProtocolException(int t, String message) {
    super("Thrift Protocol Exception (" + Integer.toString(t) + "): " + message);
    type = t;
  }
}
