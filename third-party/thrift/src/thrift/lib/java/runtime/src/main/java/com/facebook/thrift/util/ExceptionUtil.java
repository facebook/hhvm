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

import com.facebook.thrift.payload.ClientResponsePayload;
import java.util.Map;
import org.apache.thrift.TBaseException;
import org.apache.thrift.TException;
import org.apache.thrift.protocol.TProtocolException;
import org.apache.thrift.transport.TTransportException;
import reactor.core.Exceptions;

public final class ExceptionUtil {

  // These need to match the keys/values emitted by Cpp2Server
  private static final String EXCEPTION_HEADER_KEY = "ex";
  private static final String TRANSPORT_EXCEPTION = "3";
  private static final String PROTOCOL_EXCEPTION = "4";

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

  /**
   * Prepare an unchecked {@link RuntimeException}.<br>
   * If the errorResponse header contains transport or protocol exception, transform the exception
   * into {@link TTransportException} or {@link TProtocolException}.
   *
   * @param errorResponse
   * @return RuntimeException
   */
  public static RuntimeException propagate(ClientResponsePayload errorResponse) {
    Map<String, String> headers = errorResponse.getHeaders();
    Exception exception = errorResponse.getException();

    if (headers != null) {
      if (exception instanceof TBaseException) {
        ((TBaseException) exception).setHeaders(headers);
      }

      String value = headers.get(EXCEPTION_HEADER_KEY);
      if (TRANSPORT_EXCEPTION.equals(value)) {
        TTransportException ex =
            new TTransportException(
                exception.getMessage() != null
                    ? exception.getMessage()
                    : "Proxy hit a transport exception");
        ex.setHeaders(headers);
        return ex;
      }
      if (PROTOCOL_EXCEPTION.equals(value)) {
        TProtocolException ex = new TProtocolException("Proxy hit a protocol exception");
        ex.setHeaders(headers);
        return ex;
      }
    }

    return Exceptions.propagate(exception);
  }
}
