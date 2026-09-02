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
import org.apache.thrift.ResponseRpcError;
import org.apache.thrift.ResponseRpcErrorCategory;
import org.apache.thrift.ResponseRpcErrorCode;
import org.apache.thrift.transport.TTransportException;

/**
 * Encodes and decodes the errors that Rocket carries out of band, in an RSocket ERROR frame, rather
 * than as a normal response payload.
 *
 * <p>A {@link org.apache.thrift.TApplicationException} sent as an ordinary reply cannot express
 * these: both the C++ and the Java clients rebuild such an exception from the response metadata
 * alone and discard the serialized body, so its type is always lost. The protocol therefore carries
 * them as an ERROR frame whose data is a compact-serialized {@link ResponseRpcError}, which is what
 * the C++ server sends and what both clients decode.
 *
 * <p>rsocket-java exposes that frame data as the exception's message, so the encoding only survives
 * while every byte stays below 0x80. That holds for the fixed fields, and {@link #taskExpired}
 * keeps the variable part ASCII and short enough for its length to fit in a single-byte varint.
 */
public final class RocketErrorUtil {

  private static final String TASK_EXPIRED_NAME = "TASK_EXPIRED";

  /** Keeps the {@code what_utf8} length varint to a single byte. See the class javadoc. */
  private static final int MAX_MESSAGE_LENGTH = 127;

  private RocketErrorUtil() {}

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
   * rsocket-java surfaces the frame data as the exception message.
   */
  public static ResponseRpcError decodeRocketError(Throwable t) {
    ByteBuf buffer = Unpooled.wrappedBuffer(t.getMessage().getBytes(StandardCharsets.UTF_8));
    return ProtocolUtil.readCompact(ResponseRpcError::read0, buffer);
  }

  /**
   * Maps a decoded error code onto the {@link TTransportException} type a caller should see.
   *
   * <p>This is a verbatim move of the client's former private {@code getType}, so the map is the
   * one that shipped before: only {@code TASK_EXPIRED} reaches TIMED_OUT, and every other code,
   * including {@code QUEUE_TIMEOUT}, degrades to UNKNOWN.
   *
   * <p>It cannot be completed in place. The C++ client maps these codes onto {@link
   * org.apache.thrift.TApplicationException}, which has a type for nearly every code: LOADSHEDDING
   * for the overload, shutdown, quota and interaction-loadshed codes, plus UNKNOWN_METHOD,
   * CHECKSUM_MISMATCH, INJECTED_FAILURE, INTERRUPTION, UNSUPPORTED_CLIENT_TYPE and INTERNAL_ERROR.
   * {@link TTransportException} declares only UNKNOWN, NOT_OPEN, ALREADY_OPEN, TIMED_OUT and
   * END_OF_FILE, so TIMED_OUT is the one type it shares with that set. The next diff in the stack
   * moves the client to {@code TApplicationException} and ports the full table.
   */
  public static int toTransportExceptionType(ResponseRpcErrorCode code) {
    switch (code) {
      case TASK_EXPIRED:
        return TTransportException.TIMED_OUT;
      default:
        return TTransportException.UNKNOWN;
    }
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
