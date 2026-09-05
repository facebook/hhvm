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
import static com.facebook.thrift.rsocket.util.RocketErrorUtil.toApplicationException;
import static com.facebook.thrift.rsocket.util.RocketErrorUtil.toApplicationExceptionType;
import static com.facebook.thrift.rsocket.util.RocketErrorUtil.toErrorCodeHeader;
import static com.facebook.thrift.rsocket.util.RocketErrorUtil.toResponseMetadata;
import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertNull;
import static org.junit.jupiter.api.Assertions.assertTrue;

import io.rsocket.RSocketErrorException;
import io.rsocket.exceptions.CanceledException;
import io.rsocket.exceptions.RejectedException;
import io.rsocket.frame.ErrorFrameCodec;
import java.nio.charset.StandardCharsets;
import java.util.Arrays;
import java.util.List;
import java.util.Set;
import java.util.stream.Collectors;
import java.util.stream.Stream;
import org.apache.thrift.ResponseRpcError;
import org.apache.thrift.ResponseRpcErrorCategory;
import org.apache.thrift.ResponseRpcErrorCode;
import org.apache.thrift.ResponseRpcMetadata;
import org.apache.thrift.TApplicationException;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.params.ParameterizedTest;
import org.junit.jupiter.params.provider.Arguments;
import org.junit.jupiter.params.provider.MethodSource;

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

  /**
   * The complete table from the C++ client's {@code decodeResponseError}. Each row is code ->
   * {@code TApplicationException} type -> {@code ex} header value.
   */
  static Stream<Arguments> errorCodeTable() {
    return Stream.of(
        Arguments.of(ResponseRpcErrorCode.OVERLOAD, TApplicationException.LOADSHEDDING, "1"),
        Arguments.of(ResponseRpcErrorCode.TASK_EXPIRED, TApplicationException.TIMEOUT, "2"),
        Arguments.of(
            ResponseRpcErrorCode.QUEUE_OVERLOADED, TApplicationException.LOADSHEDDING, "5"),
        Arguments.of(ResponseRpcErrorCode.SHUTDOWN, TApplicationException.LOADSHEDDING, "5"),
        Arguments.of(
            ResponseRpcErrorCode.INJECTED_FAILURE, TApplicationException.INJECTED_FAILURE, "14"),
        Arguments.of(
            ResponseRpcErrorCode.REQUEST_PARSING_FAILURE,
            TApplicationException.UNSUPPORTED_CLIENT_TYPE,
            "28"),
        Arguments.of(ResponseRpcErrorCode.QUEUE_TIMEOUT, TApplicationException.TIMEOUT, "15"),
        Arguments.of(
            ResponseRpcErrorCode.RESPONSE_TOO_BIG, TApplicationException.INTERNAL_ERROR, "17"),
        Arguments.of(
            ResponseRpcErrorCode.WRONG_RPC_KIND, TApplicationException.UNKNOWN_METHOD, "21"),
        Arguments.of(
            ResponseRpcErrorCode.UNKNOWN_METHOD, TApplicationException.UNKNOWN_METHOD, "25"),
        // C++ reports the unknown code here, not "30". Bug-compatible on purpose.
        Arguments.of(
            ResponseRpcErrorCode.CHECKSUM_MISMATCH, TApplicationException.CHECKSUM_MISMATCH, "0"),
        // C++ sends no ex header for this one.
        Arguments.of(ResponseRpcErrorCode.INTERRUPTION, TApplicationException.INTERRUPTION, null),
        Arguments.of(ResponseRpcErrorCode.APP_OVERLOAD, TApplicationException.LOADSHEDDING, "22"),
        Arguments.of(
            ResponseRpcErrorCode.UNKNOWN_INTERACTION_ID, TApplicationException.UNKNOWN, "26"),
        Arguments.of(
            ResponseRpcErrorCode.INTERACTION_CONSTRUCTOR_ERROR,
            TApplicationException.UNKNOWN,
            "27"),
        Arguments.of(
            ResponseRpcErrorCode.UNIMPLEMENTED_METHOD, TApplicationException.UNKNOWN_METHOD, "31"),
        Arguments.of(
            ResponseRpcErrorCode.TENANT_QUOTA_EXCEEDED, TApplicationException.LOADSHEDDING, "32"),
        Arguments.of(
            ResponseRpcErrorCode.INTERACTION_LOADSHEDDED, TApplicationException.LOADSHEDDING, "34"),
        Arguments.of(
            ResponseRpcErrorCode.INTERACTION_LOADSHEDDED_QUEUE_TIMEOUT,
            TApplicationException.LOADSHEDDING,
            "35"),
        Arguments.of(
            ResponseRpcErrorCode.INTERACTION_LOADSHEDDED_OVERLOAD,
            TApplicationException.LOADSHEDDING,
            "36"),
        Arguments.of(
            ResponseRpcErrorCode.INTERACTION_LOADSHEDDED_APP_OVERLOAD,
            TApplicationException.LOADSHEDDING,
            "37"),
        Arguments.of(ResponseRpcErrorCode.UNKNOWN, TApplicationException.UNKNOWN, "0"));
  }

  @ParameterizedTest
  @MethodSource("errorCodeTable")
  public void testMatchesTheCppTable(
      ResponseRpcErrorCode code, int expectedType, String expectedErrorCode) {
    assertEquals(expectedType, toApplicationExceptionType(code));
    assertEquals(expectedErrorCode, toErrorCodeHeader(code));
  }

  /**
   * The table above must name every constant the generated enum declares. A code added to {@code
   * RpcMetadata.thrift} otherwise falls through to UNKNOWN in silence, and nobody learns that the
   * C++ {@code decodeResponseError} has a new row to copy. C++ has the same silent default, so this
   * guard is stricter than C++ on purpose: it fails the build, not the request.
   *
   * <p>Only the enum key can come from the thrift source. The other two columns cannot: the {@code
   * TApplicationException} types are hand-written ints in that class, and the {@code ex} codes are
   * hand-written strings in the C++ {@code ResponseChannel.cpp}. Neither is derivable from {@code
   * BaseEnum.getValue()}, which returns the thrift i32 and agrees with the {@code ex} code on
   * TASK_EXPIRED alone, by coincidence. This guard is what keeps the hand-written columns honest.
   *
   * <p>The reverse direction needs no guard. A row for a deleted constant does not compile.
   */
  @Test
  public void testTheTableCoversEveryGeneratedEnumConstant() {
    Set<ResponseRpcErrorCode> tabled =
        errorCodeTable()
            .map(arguments -> (ResponseRpcErrorCode) arguments.get()[0])
            .collect(Collectors.toSet());

    List<ResponseRpcErrorCode> missing =
        Arrays.stream(ResponseRpcErrorCode.values())
            .filter(code -> !tabled.contains(code))
            .collect(Collectors.toList());

    assertTrue(
        missing.isEmpty(),
        "RpcMetadata.thrift declares codes this table does not map: "
            + missing
            + ". Read the new rows in the C++ decodeResponseError, then add them here and to"
            + " RocketErrorUtil.");
  }

  @Test
  public void testUnrecognizedCodeFallsBackToUnknown() {
    assertEquals(TApplicationException.UNKNOWN, toApplicationExceptionType(null));
    assertEquals("0", toErrorCodeHeader(null));
  }

  @Test
  public void testApplicationExceptionCarriesTypeAndWhatUtf8() {
    ResponseRpcError error =
        new ResponseRpcError.Builder()
            .setCode(ResponseRpcErrorCode.APP_OVERLOAD)
            .setWhatUtf8("server is busy")
            .build();

    TApplicationException exception = toApplicationException(error);
    assertEquals(TApplicationException.LOADSHEDDING, exception.getType());
    assertEquals("server is busy", exception.getMessage());
  }

  @Test
  public void testApplicationExceptionToleratesAMissingWhatUtf8() {
    ResponseRpcError error =
        new ResponseRpcError.Builder().setCode(ResponseRpcErrorCode.OVERLOAD).build();
    assertEquals("", toApplicationException(error).getMessage());
  }

  @Test
  public void testResponseMetadataCarriesTheExHeaderAndLoad() {
    ResponseRpcError error =
        new ResponseRpcError.Builder()
            .setCode(ResponseRpcErrorCode.QUEUE_TIMEOUT)
            .setLoad(42L)
            .build();

    ResponseRpcMetadata metadata = toResponseMetadata(error);
    assertEquals("15", metadata.getOtherMetadata().get(RocketErrorUtil.HEADER_EX));
    assertEquals(Long.valueOf(42L), metadata.getLoad());
  }

  @Test
  public void testResponseMetadataOmitsTheExHeaderWhenCppSendsNone() {
    ResponseRpcError error =
        new ResponseRpcError.Builder().setCode(ResponseRpcErrorCode.INTERRUPTION).build();

    ResponseRpcMetadata metadata = toResponseMetadata(error);
    assertTrue(
        metadata.getOtherMetadata() == null || metadata.getOtherMetadata().isEmpty(),
        "expected no ex header, got " + metadata.getOtherMetadata());
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

  /**
   * A C++ server puts its raw exception text in what_utf8 with no length limit. Text of 128 bytes
   * or more makes a length varint that is not valid UTF-8, and rsocket-java replaces it before this
   * runtime sees it. The read must report a parse failure, not let one escape.
   */
  @Test
  public void testAnUnreadableFrameBecomesAParseFailure() {
    RSocketErrorException notAThriftStruct = new CanceledException("connection reset by peer");

    assertNull(decodeRocketError(notAThriftStruct));

    TApplicationException exception = toApplicationException((Throwable) notAThriftStruct);
    assertEquals(TApplicationException.UNKNOWN, exception.getType());
    assertTrue(
        exception.getMessage().startsWith("Error parsing error frame: "),
        "unexpected message: " + exception.getMessage());
  }

  @Test
  public void testAReadableFrameStillDecodesThroughTheThrowableOverload() {
    RSocketErrorException exception = taskExpired("someMethod", 5000L);

    TApplicationException decoded = toApplicationException((Throwable) exception);
    assertEquals(TApplicationException.TIMEOUT, decoded.getType());
    assertEquals("Task expired for someMethod after 5000ms", decoded.getMessage());
  }
}
