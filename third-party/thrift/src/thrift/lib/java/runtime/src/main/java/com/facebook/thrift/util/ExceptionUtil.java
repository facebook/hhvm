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

import org.apache.thrift.TException;

public final class ExceptionUtil {
  private static final Class REACTIVE_EXCEPTION;

  static {
    try {
      REACTIVE_EXCEPTION = Class.forName("reactor.core.Exceptions$ReactiveException");
    } catch (Throwable t) {
      throw new RuntimeException(t);
    }
  }

  /**
   * Wraps given exception into TException if it is not an instance of TException
   *
   * @param t The exception to wrap
   * @return TException
   */
  public static TException wrap(Throwable t) {
    if (t instanceof TException) {
      return (TException) t;
    }
    if (t.getClass() == REACTIVE_EXCEPTION) {
      return new TException(t.getCause());
    }
    return new TException(t);
  }
}
