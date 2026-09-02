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

import com.facebook.thrift.protocol.ProtocolUtil;
import com.facebook.thrift.util.resources.RpcResources;
import io.netty.buffer.ByteBuf;
import io.netty.buffer.Unpooled;
import io.netty.util.ReferenceCountUtil;
import io.rsocket.RSocketErrorException;
import io.rsocket.exceptions.CanceledException;
import io.rsocket.frame.ErrorFrameCodec;
import java.nio.charset.StandardCharsets;
import java.util.Collections;
import java.util.EnumMap;
import java.util.Map;
import org.apache.thrift.ResponseRpcError;
import org.apache.thrift.ResponseRpcErrorCategory;
import org.apache.thrift.ResponseRpcErrorCode;
import org.apache.thrift.ResponseRpcMetadata;
import org.apache.thrift.TApplicationException;

/**
 * Encodes and decodes the errors that Rocket carries out of band, in an RSocket ERROR frame, rather
 * than as a normal response payload.
 *
 * <p>A {@link TApplicationException} sent as an ordinary reply cannot express these: both the C++
 * and the Java clients rebuild such an exception from the response metadata alone and discard the
 * serialized body, so its type is always lost. The protocol therefore carries them as an ERROR
 * frame whose data is a compact-serialized {@link ResponseRpcError}, which is what the C++ server
 * sends and what both clients decode.
 *
 * <p>Rocket draws its exception boundary by layer, not by severity. Anything the peer reported in
 * an ERROR frame is application level and becomes a {@link TApplicationException}, even a retryable
 * condition such as load shedding. Anything about the connection or the local request lifecycle
 * stays a {@code TTransportException}. This class covers the first of those two.
 *
 * <p>rsocket-java exposes the frame data as the exception's message, so the encoding only survives
 * while every byte stays below 0x80. That holds for the fixed fields, and {@link #taskExpired}
 * keeps the variable part ASCII and short enough for its length to fit in a single-byte varint.
 */
public final class RocketErrorUtil {

  /** Response header that carries the error code string, as {@code kHeaderEx} does in C++. */
  public static final String HEADER_EX = "ex";

  private static final String TASK_EXPIRED_NAME = "TASK_EXPIRED";

  /** Keeps the {@code what_utf8} length varint to a single byte. See the class javadoc. */
  private static final int MAX_MESSAGE_LENGTH = 127;

  private static final String UNKNOWN_ERROR_CODE = "0";
  private static final String OVERLOADED_ERROR_CODE = "1";
  private static final String TASK_EXPIRED_ERROR_CODE = "2";
  private static final String QUEUE_OVERLOADED_ERROR_CODE = "5";
  private static final String INJECTED_FAILURE_ERROR_CODE = "14";
  private static final String SERVER_QUEUE_TIMEOUT_ERROR_CODE = "15";
  private static final String RESPONSE_TOO_BIG_ERROR_CODE = "17";
  private static final String WRONG_RPC_KIND_ERROR_CODE = "21";
  private static final String APP_OVERLOADED_ERROR_CODE = "22";
  private static final String METHOD_UNKNOWN_ERROR_CODE = "25";
  private static final String INTERACTION_ID_UNKNOWN_ERROR_CODE = "26";
  private static final String INTERACTION_CONSTRUCTOR_ERROR_ERROR_CODE = "27";
  private static final String REQUEST_PARSING_ERROR_CODE = "28";
  private static final String UNIMPLEMENTED_METHOD_ERROR_CODE = "31";
  private static final String TENANT_QUOTA_EXCEEDED_ERROR_CODE = "32";
  private static final String INTERACTION_LOADSHEDDED_ERROR_CODE = "34";
  private static final String INTERACTION_LOADSHEDDED_QUEUE_TIMEOUT_ERROR_CODE = "35";
  private static final String INTERACTION_LOADSHEDDED_OVERLOAD_ERROR_CODE = "36";
  private static final String INTERACTION_LOADSHEDDED_APP_OVERLOAD_ERROR_CODE = "37";

  private static final Mapping UNKNOWN_MAPPING =
      new Mapping(TApplicationException.UNKNOWN, UNKNOWN_ERROR_CODE);

  private static final Map<ResponseRpcErrorCode, Mapping> MAPPINGS = buildMappings();

  private RocketErrorUtil() {}

  /** The exception type and {@code ex} header code that one error code maps onto. */
  private static final class Mapping {
    private final int exceptionType;
    private final String errorCode;

    Mapping(int exceptionType, String errorCode) {
      this.exceptionType = exceptionType;
      this.errorCode = errorCode;
    }
  }

  /**
   * Mirrors the table in the C++ client's {@code decodeResponseError}. Two rows look wrong and are
   * deliberate: {@code CHECKSUM_MISMATCH} reports the unknown error code rather than its own, and
   * {@code INTERRUPTION} reports no code at all. Both match C++, and the C++ fast_thrift client
   * reproduces them, so they are bug-compatible on purpose.
   */
  private static Map<ResponseRpcErrorCode, Mapping> buildMappings() {
    Map<ResponseRpcErrorCode, Mapping> mappings = new EnumMap<>(ResponseRpcErrorCode.class);

    mappings.put(
        ResponseRpcErrorCode.OVERLOAD,
        new Mapping(TApplicationException.LOADSHEDDING, OVERLOADED_ERROR_CODE));
    mappings.put(
        ResponseRpcErrorCode.QUEUE_OVERLOADED,
        new Mapping(TApplicationException.LOADSHEDDING, QUEUE_OVERLOADED_ERROR_CODE));
    mappings.put(
        ResponseRpcErrorCode.SHUTDOWN,
        new Mapping(TApplicationException.LOADSHEDDING, QUEUE_OVERLOADED_ERROR_CODE));
    mappings.put(
        ResponseRpcErrorCode.APP_OVERLOAD,
        new Mapping(TApplicationException.LOADSHEDDING, APP_OVERLOADED_ERROR_CODE));
    mappings.put(
        ResponseRpcErrorCode.TENANT_QUOTA_EXCEEDED,
        new Mapping(TApplicationException.LOADSHEDDING, TENANT_QUOTA_EXCEEDED_ERROR_CODE));
    mappings.put(
        ResponseRpcErrorCode.INTERACTION_LOADSHEDDED,
        new Mapping(TApplicationException.LOADSHEDDING, INTERACTION_LOADSHEDDED_ERROR_CODE));
    mappings.put(
        ResponseRpcErrorCode.INTERACTION_LOADSHEDDED_OVERLOAD,
        new Mapping(
            TApplicationException.LOADSHEDDING, INTERACTION_LOADSHEDDED_OVERLOAD_ERROR_CODE));
    mappings.put(
        ResponseRpcErrorCode.INTERACTION_LOADSHEDDED_APP_OVERLOAD,
        new Mapping(
            TApplicationException.LOADSHEDDING, INTERACTION_LOADSHEDDED_APP_OVERLOAD_ERROR_CODE));
    mappings.put(
        ResponseRpcErrorCode.INTERACTION_LOADSHEDDED_QUEUE_TIMEOUT,
        new Mapping(
            TApplicationException.LOADSHEDDING, INTERACTION_LOADSHEDDED_QUEUE_TIMEOUT_ERROR_CODE));

    mappings.put(
        ResponseRpcErrorCode.TASK_EXPIRED,
        new Mapping(TApplicationException.TIMEOUT, TASK_EXPIRED_ERROR_CODE));
    mappings.put(
        ResponseRpcErrorCode.QUEUE_TIMEOUT,
        new Mapping(TApplicationException.TIMEOUT, SERVER_QUEUE_TIMEOUT_ERROR_CODE));

    mappings.put(
        ResponseRpcErrorCode.UNKNOWN_METHOD,
        new Mapping(TApplicationException.UNKNOWN_METHOD, METHOD_UNKNOWN_ERROR_CODE));
    mappings.put(
        ResponseRpcErrorCode.WRONG_RPC_KIND,
        new Mapping(TApplicationException.UNKNOWN_METHOD, WRONG_RPC_KIND_ERROR_CODE));
    mappings.put(
        ResponseRpcErrorCode.UNIMPLEMENTED_METHOD,
        new Mapping(TApplicationException.UNKNOWN_METHOD, UNIMPLEMENTED_METHOD_ERROR_CODE));

    mappings.put(
        ResponseRpcErrorCode.INJECTED_FAILURE,
        new Mapping(TApplicationException.INJECTED_FAILURE, INJECTED_FAILURE_ERROR_CODE));
    mappings.put(
        ResponseRpcErrorCode.REQUEST_PARSING_FAILURE,
        new Mapping(TApplicationException.UNSUPPORTED_CLIENT_TYPE, REQUEST_PARSING_ERROR_CODE));
    mappings.put(
        ResponseRpcErrorCode.RESPONSE_TOO_BIG,
        new Mapping(TApplicationException.INTERNAL_ERROR, RESPONSE_TOO_BIG_ERROR_CODE));
    mappings.put(
        ResponseRpcErrorCode.CHECKSUM_MISMATCH,
        new Mapping(TApplicationException.CHECKSUM_MISMATCH, UNKNOWN_ERROR_CODE));
    mappings.put(
        ResponseRpcErrorCode.INTERRUPTION, new Mapping(TApplicationException.INTERRUPTION, null));

    mappings.put(
        ResponseRpcErrorCode.UNKNOWN_INTERACTION_ID,
        new Mapping(TApplicationException.UNKNOWN, INTERACTION_ID_UNKNOWN_ERROR_CODE));
    mappings.put(
        ResponseRpcErrorCode.INTERACTION_CONSTRUCTOR_ERROR,
        new Mapping(TApplicationException.UNKNOWN, INTERACTION_CONSTRUCTOR_ERROR_ERROR_CODE));

    return Collections.unmodifiableMap(mappings);
  }

  private static Mapping mappingFor(ResponseRpcErrorCode code) {
    Mapping mapping = code == null ? null : MAPPINGS.get(code);
    return mapping == null ? UNKNOWN_MAPPING : mapping;
  }

  /**
   * Returns the error a server sends when a request outlives its task timeout. Mirrors the C++
   * server, which reports {@code TASK_EXPIRED} in an ERROR frame with the {@code CANCELED} code.
   */
  public static RSocketErrorException taskExpired(String methodName, long timeoutMillis) {
    String message = String.format("Task expired for %s after %dms", methodName, timeoutMillis);
    ResponseRpcError error =
        new ResponseRpcError.Builder()
            .setNameUtf8(TASK_EXPIRED_NAME)
            .setWhatUtf8(toSafeMessage(message))
            .setCategory(ResponseRpcErrorCategory.INTERNAL_ERROR)
            .setCode(ResponseRpcErrorCode.TASK_EXPIRED)
            .build();
    return new CanceledException(encode(error));
  }

  /**
   * Returns whether the throwable is an ERROR frame carrying a {@link ResponseRpcError}. The Rocket
   * protocol defines the INVALID, CANCELED and REJECTED codes as server errors that do so.
   */
  public static boolean isRocketError(Throwable t) {
    if (!(t instanceof RSocketErrorException)) {
      return false;
    }
    int code = ((RSocketErrorException) t).errorCode();
    return code >= ErrorFrameCodec.REJECTED && code <= ErrorFrameCodec.INVALID;
  }

  /**
   * Reads the {@link ResponseRpcError} out of an error for which {@link #isRocketError} holds.
   * rsocket-java surfaces the frame data as the exception message. Returns null when the frame does
   * not hold a readable struct.
   *
   * <p>The ASCII rule in the class javadoc governs what {@link #taskExpired} writes; it says
   * nothing about what a peer sends. A C++ server puts its raw exception text in {@code what_utf8}
   * with no length limit, and text of 128 bytes or more produces a length varint that is not valid
   * UTF-8, which rsocket-java has already replaced by the time the message reaches this method. A
   * peer that sends CANCELED, REJECTED or INVALID with anything other than a {@code
   * ResponseRpcError} fails here too. C++ guards the same read in {@code decodeResponseError} and
   * reports a parse failure rather than letting it escape.
   */
  public static ResponseRpcError decodeRocketError(Throwable t) {
    try {
      ByteBuf buffer = Unpooled.wrappedBuffer(t.getMessage().getBytes(StandardCharsets.UTF_8));
      return ProtocolUtil.readCompact(ResponseRpcError::read0, buffer);
    } catch (Throwable parseFailure) {
      return null;
    }
  }

  /**
   * Builds the exception a caller should see for an ERROR frame, including one this runtime cannot
   * read. Mirrors the two guards C++ applies in {@code decodeResponseError}.
   */
  public static TApplicationException toApplicationException(Throwable t) {
    ResponseRpcError error = decodeRocketError(t);
    if (error == null) {
      return new TApplicationException(
          TApplicationException.UNKNOWN, "Error parsing error frame: " + t);
    }
    return toApplicationException(error);
  }

  /** Returns the {@link TApplicationException} type that an error code maps onto. */
  public static int toApplicationExceptionType(ResponseRpcErrorCode code) {
    return mappingFor(code).exceptionType;
  }

  /** Returns the {@link #HEADER_EX} value for an error code, or null when C++ sends none. */
  public static String toErrorCodeHeader(ResponseRpcErrorCode code) {
    return mappingFor(code).errorCode;
  }

  /** Builds the exception a caller should see for a decoded {@link ResponseRpcError}. */
  public static TApplicationException toApplicationException(ResponseRpcError error) {
    String what = error.getWhatUtf8() == null ? "" : error.getWhatUtf8();
    return new TApplicationException(toApplicationExceptionType(error.getCode()), what);
  }

  /**
   * Builds the response metadata that travels with a decoded {@link ResponseRpcError}: the {@link
   * #HEADER_EX} header, and the server load when the error reports it.
   */
  public static ResponseRpcMetadata toResponseMetadata(ResponseRpcError error) {
    ResponseRpcMetadata.Builder builder = new ResponseRpcMetadata.Builder();
    String errorCode = toErrorCodeHeader(error.getCode());
    if (errorCode != null) {
      builder.setOtherMetadata(Collections.singletonMap(HEADER_EX, errorCode));
    }
    if (error.getLoad() != null) {
      builder.setLoad(error.getLoad());
    }
    return builder.build();
  }

  /** Reduces a message to plain ASCII within {@link #MAX_MESSAGE_LENGTH} characters. */
  private static String toSafeMessage(String message) {
    int length = Math.min(message.length(), MAX_MESSAGE_LENGTH);
    StringBuilder builder = new StringBuilder(length);
    for (int i = 0; i < length; i++) {
      char c = message.charAt(i);
      builder.append(c < 0x80 ? c : '?');
    }
    return builder.toString();
  }

  private static String encode(ResponseRpcError error) {
    ByteBuf buffer = RpcResources.getByteBufAllocator().buffer();
    try {
      ProtocolUtil.writeCompact(error::write0, buffer);
      return buffer.toString(StandardCharsets.UTF_8);
    } finally {
      ReferenceCountUtil.safeRelease(buffer);
    }
  }
}
