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

package com.facebook.thrift.utils;

import java.io.IOException;
import java.io.InputStream;
import java.nio.ByteBuffer;

public final class ByteBufferUtils {
  public static final byte[] EMPTY_BYTES = new byte[0];
  public static final ByteBuffer EMPTY_BUFFER = ByteBuffer.allocate(0);

  private ByteBufferUtils() {}

  /**
   * Convert the ByteBuffer content into a byte[] without consuming the ByteBuffer (i.e. without
   * moving the position).
   *
   * <p>If the ByteBuffer is backed by a byte[] and the position and limit are at the limit of the
   * buffer (resp. 0 and capacity), the underlying byte[] is returned without any copy.
   *
   * @param buf the ByteBuffer
   * @return the byte array containing the same data as the ByteBuffer.
   */
  public static byte[] toBytes(ByteBuffer buffer) {
    if (buffer == null) {
      return EMPTY_BYTES;
    }

    if (buffer.hasArray()
        && buffer.arrayOffset() == 0
        && buffer.position() == 0
        && buffer.limit() == buffer.capacity()) {
      return buffer.array();
    } else {
      ByteBuffer shallowCopy = buffer.duplicate();
      byte[] copy = new byte[shallowCopy.remaining()];
      shallowCopy.get(copy);
      return copy;
    }
  }

  /**
   * Return a String representing the content of a ByteBuffer in hexadecimal form without consuming
   * the ByteBuffer (i.e. without moving the position marker).
   */
  public static String toHexString(ByteBuffer buffer) {
    if (buffer == null) {
      return "null";
    }

    StringBuilder s = new StringBuilder();
    for (int i = buffer.position(); i < buffer.limit(); i++) {
      if (i > buffer.position()) {
        s.append(' ');
      }
      s.append(String.format("%02x", buffer.get(i)));
    }
    return s.toString();
  }

  /**
   * Create a new ByteBuffer with a deep copy of the bytes represented by the input. This method
   * does not consume the ByteBuffer (i.e. doesn't move its position marker).
   */
  public static ByteBuffer deepCopy(ByteBuffer src) {
    if (src == null) {
      return null;
    }

    if (src.hasArray()) {
      byte[] bytes = new byte[src.remaining()];
      System.arraycopy(src.array(), src.arrayOffset() + src.position(), bytes, 0, bytes.length);
      return ByteBuffer.wrap(bytes);
    } else {
      ByteBuffer shallowCopy = src.duplicate();
      ByteBuffer copy = ByteBuffer.allocate(shallowCopy.remaining());
      copy.put(shallowCopy);
      copy.flip();
      return copy;
    }
  }

  /**
   * Create an instance of an InputStream which streams the data contained in a ByteBuffer. Note:
   * This stream consumes the data in the ByteBuffer (i.e. move the position marker).
   */
  public static InputStream toInputStream(final ByteBuffer buffer) {
    if (buffer == null) {
      return new InputStream() {
        public int read() throws IOException {
          return -1;
        }

        public int read(byte[] bytes, int off, int len) throws IOException {
          return -1;
        }
      };
    }

    return new InputStream() {
      private ByteBuffer buf = buffer;

      public int read() throws IOException {
        if (!buf.hasRemaining()) {
          return -1;
        }
        return buf.get() & 0xFF;
      }

      public int read(byte[] bytes, int off, int len) throws IOException {
        if (!buf.hasRemaining()) {
          return -1;
        }

        int n = Math.min(len, buf.remaining());
        buf.get(bytes, off, n);
        return n;
      }
    };
  }
}
