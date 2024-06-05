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

import static io.netty.util.CharsetUtil.UTF_8;

import io.netty.buffer.ByteBuf;
import io.netty.buffer.ByteBufUtil;
import io.netty.util.concurrent.FastThreadLocal;
import io.netty.util.internal.PlatformDependent;
import io.netty.util.internal.StringUtil;
import java.lang.invoke.MethodHandle;
import java.lang.invoke.MethodHandles;
import java.lang.reflect.Method;
import java.nio.charset.MalformedInputException;
import java.nio.charset.StandardCharsets;
import org.apache.thrift.protocol.TProtocolException;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class Utf8Util {

  private static final Logger logger = LoggerFactory.getLogger(Utf8Util.class);

  private static final MethodHandle CREATE_STRING;
  private static boolean useNewMethod = false;

  static {
    MethodHandle createStr = null;
    try {
      MethodHandles.Lookup lookup = MethodHandles.lookup();

      Method m =
          String.class.getDeclaredMethod("newStringUTF8NoRepl", byte[].class, int.class, int.class);
      m.setAccessible(true);
      createStr = lookup.unreflect(m);
    } catch (Throwable t) {
      // Ignore, validation will be done before creating the String
    }

    // Java 21+ String has a new method signature, we must use the new signature here by passing
    // in true for noShare which is equivalent to the old API
    if (createStr == null) {
      try {
        MethodHandles.Lookup lookup = MethodHandles.lookup();

        Method m =
            String.class.getDeclaredMethod(
                "newStringUTF8NoRepl", byte[].class, int.class, int.class, boolean.class);
        m.setAccessible(true);
        createStr = lookup.unreflect(m);
        useNewMethod = true;
      } catch (Throwable t) {
        // Ignore, validation will be done before creating the String
      }
    }

    if (createStr == null) {
      logger.warn(
          "Add JVM option for faster UTF-8 validation. --add-opens"
              + " java.base/java.lang=ALL-UNNAMED");
    }

    CREATE_STRING = createStr;
  }

  private static void throwException(ByteBuf buf) {
    buf.resetReaderIndex();
    throw new TProtocolException(
        TProtocolException.INVALID_DATA, "Malformed UTF8 string: " + ByteBufUtil.hexDump(buf));
  }

  static final int MAX_TL_ARRAY_LEN = 1024;
  private static final FastThreadLocal<byte[]> BYTE_ARRAYS =
      new FastThreadLocal<byte[]>() {
        @Override
        protected byte[] initialValue() throws Exception {
          return PlatformDependent.allocateUninitializedArray(MAX_TL_ARRAY_LEN);
        }
      };

  private static byte[] threadLocalTempArray(int minLength) {
    return minLength <= MAX_TL_ARRAY_LEN
        ? BYTE_ARRAYS.get()
        : PlatformDependent.allocateUninitializedArray(minLength);
  }

  /**
   * Read string from given ByteBuf. All bytes are read. If the bytes are not valid UTF-8 encoded
   * data, an exception is raised.
   *
   * @param src ByteBuf wrapping a byte array
   * @return UTF-8 encoded string
   * @throws TProtocolException If the UTF-8 bytes are malformed.
   */
  public static String readStringReportIfInvalid(ByteBuf src) throws TProtocolException {
    if (CREATE_STRING == null) {
      if (!ByteBufUtil.isText(src, UTF_8)) {
        throwException(src);
      }
      return src.toString(StandardCharsets.UTF_8);
    }

    final int length = src.readableBytes();
    if (length == 0) {
      return StringUtil.EMPTY_STRING;
    }
    final byte[] array;
    final int offset;

    if (src.hasArray()) {
      array = src.array();
      offset = src.arrayOffset();
    } else {
      array = threadLocalTempArray(length);
      offset = 0;
      src.getBytes(0, array, 0, length);
    }

    try {
      if (useNewMethod) {
        return (String) CREATE_STRING.invoke(array, offset, length, true);
      } else {
        return (String) CREATE_STRING.invoke(array, offset, length);
      }
    } catch (Throwable t) {
      if (t.getCause() instanceof MalformedInputException) {
        throwException(src);
      }
      throw new RuntimeException(t);
    }
  }

  /**
   * Read string from given ByteBuf. If the bytes are not valid UTF-8 encoded data, they are
   * replaced with replacement char.
   *
   * @param src
   * @return String
   */
  public static String readString(ByteBuf src) throws TProtocolException {
    final int length = src.readableBytes();
    if (length == 0) {
      return StringUtil.EMPTY_STRING;
    }
    final byte[] array;
    final int offset;

    if (src.hasArray()) {
      array = src.array();
      offset = src.arrayOffset();
    } else {
      array = threadLocalTempArray(length);
      offset = 0;
      src.getBytes(0, array, 0, length);
    }

    return new String(array, offset, length);
  }
}
