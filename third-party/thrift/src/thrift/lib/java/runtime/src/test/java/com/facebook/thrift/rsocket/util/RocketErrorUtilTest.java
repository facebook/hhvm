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

package com.facebook.thrift.rsocket.util;

import static com.facebook.thrift.rsocket.util.RocketErrorUtil.decodeRocketError;
import static com.facebook.thrift.rsocket.util.RocketErrorUtil.isRocketError;
import static com.facebook.thrift.rsocket.util.RocketErrorUtil.taskExpired;
import static com.facebook.thrift.rsocket.util.RocketErrorUtil.toTransportExceptionType;
import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertTrue;

import io.rsocket.RSocketErrorException;
import io.rsocket.exceptions.RejectedException;
import io.rsocket.frame.ErrorFrameCodec;
import java.nio.charset.StandardCharsets;
import org.apache.thrift.ResponseRpcError;
import org.apache.thrift.ResponseRpcErrorCategory;
import org.apache.thrift.ResponseRpcErrorCode;
import org.apache.thrift.transport.TTransportException;
import org.junit.jupiter.api.Test;

public class RocketErrorUtilTest {

  @Test
  public void testTaskExpiredUsesTheCanceledErrorCode() {
    RSocketErrorException exception = taskExpired("someMethod", 5000L);
    assertEquals(ErrorFrameCodec.CANCELED, exception.errorCode());
    assertTrue(isRocketError(exception));
  }

  @Test
  public void testIsRocketErrorRejectsOtherThrowables() {
    assertFalse(isRocketError(new IllegalStateException("boom")));
  }

  @Test
  public void testIsRocketErrorAcceptsTheOtherServerErrorCodes() {
    assertTrue(isRocketError(new RejectedException("rejected")));
  }

  /** Pins the map that this diff moves, unchanged. The next diff in the stack corrects it. */
  @Test
  public void testTaskExpiredMapsToTimedOut() {
    assertEquals(
        TTransportException.TIMED_OUT, toTransportExceptionType(ResponseRpcErrorCode.TASK_EXPIRED));
  }

  @Test
  public void testEveryOtherCodeMapsToUnknown() {
    for (ResponseRpcErrorCode code :
        new ResponseRpcErrorCode[] {
          ResponseRpcErrorCode.QUEUE_TIMEOUT,
          ResponseRpcErrorCode.OVERLOAD,
          ResponseRpcErrorCode.QUEUE_OVERLOADED,
          ResponseRpcErrorCode.SHUTDOWN,
          ResponseRpcErrorCode.INJECTED_FAILURE,
          ResponseRpcErrorCode.REQUEST_PARSING_FAILURE,
          ResponseRpcErrorCode.RESPONSE_TOO_BIG,
          ResponseRpcErrorCode.WRONG_RPC_KIND,
          ResponseRpcErrorCode.UNKNOWN_METHOD,
          ResponseRpcErrorCode.CHECKSUM_MISMATCH,
          ResponseRpcErrorCode.INTERRUPTION,
          ResponseRpcErrorCode.APP_OVERLOAD,
          ResponseRpcErrorCode.UNIMPLEMENTED_METHOD,
          ResponseRpcErrorCode.TENANT_QUOTA_EXCEEDED,
          ResponseRpcErrorCode.INTERACTION_LOADSHEDDED,
          ResponseRpcErrorCode.UNKNOWN,
        }) {
      assertEquals(TTransportException.UNKNOWN, toTransportExceptionType(code), code.toString());
    }
  }

  @Test
  public void testTaskExpiredSurvivesTheClientDecode() {
    RSocketErrorException exception = taskExpired("someMethod", 5000L);

    ResponseRpcError error = decodeRocketError(exception);
    assertEquals(ResponseRpcErrorCode.TASK_EXPIRED, error.getCode());
    assertEquals(ResponseRpcErrorCategory.INTERNAL_ERROR, error.getCategory());
    assertEquals("TASK_EXPIRED", error.getNameUtf8());
    assertEquals("Task expired for someMethod after 5000ms", error.getWhatUtf8());
  }

  /**
   * rsocket-java carries the ERROR frame data as a UTF-8 string, so the compact encoding only
   * survives while every byte stays below 0x80. Nothing else in this class matters if that stops
   * holding.
   */
  @Test
  public void testEncodedBytesStayAscii() {
    RSocketErrorException exception = taskExpired("someMethod", 5000L);

    byte[] bytes = exception.getMessage().getBytes(StandardCharsets.UTF_8);
    for (byte b : bytes) {
      assertTrue(b >= 0, "encoded ResponseRpcError must stay ASCII, saw byte " + b);
    }
  }

  @Test
  public void testLongMethodNameStaysDecodable() {
    StringBuilder methodName = new StringBuilder();
    for (int i = 0; i < 500; i++) {
      methodName.append('a');
    }

    RSocketErrorException exception = taskExpired(methodName.toString(), 5000L);

    ResponseRpcError error = decodeRocketError(exception);
    assertEquals(ResponseRpcErrorCode.TASK_EXPIRED, error.getCode());
    assertTrue(error.getWhatUtf8().length() <= 127);
  }

  @Test
  public void testNonAsciiMethodNameStaysDecodable() {
    // Built from char codes so the source file itself stays ASCII.
    String methodName = "m" + (char) 0x00E9 + "thode" + (char) 0x00E9 + (char) 0x4E2D;

    RSocketErrorException exception = taskExpired(methodName, 5000L);

    ResponseRpcError error = decodeRocketError(exception);
    assertEquals(ResponseRpcErrorCode.TASK_EXPIRED, error.getCode());
    assertEquals("Task expired for m?thode?? after 5000ms", error.getWhatUtf8());
  }
}
