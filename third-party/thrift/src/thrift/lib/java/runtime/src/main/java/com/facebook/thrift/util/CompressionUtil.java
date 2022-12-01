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

import static java.lang.invoke.MethodType.methodType;

import io.netty.buffer.ByteBuf;
import io.netty.buffer.ByteBufAllocator;
import io.netty.buffer.ByteBufUtil;
import io.netty.buffer.CompositeByteBuf;
import io.netty.util.ReferenceCountUtil;
import java.lang.invoke.MethodHandle;
import java.lang.invoke.MethodHandles;
import java.nio.ByteBuffer;
import java.util.Objects;
import java.util.zip.Inflater;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import reactor.core.Exceptions;

public class CompressionUtil {
  private static final Logger LOGGER = LoggerFactory.getLogger(CompressionUtil.class);
  private static final int BYTE_BUFFER_ZLIB_BUFFER_SIZE = 1 << 20;
  private static final int ZLIB_BUFFER_SIZE;

  private static final MethodHandle SET_INPUT_METHOD_HANDLE;
  private static final MethodHandle INFLATE_METHOD_HANDLE;

  static {
    MethodHandle setInputMethodHandle = null;
    MethodHandle inflateMethodHandle = null;

    try {
      MethodHandles.Lookup lookup = MethodHandles.lookup();
      setInputMethodHandle =
          lookup.findVirtual(Inflater.class, "setInput", methodType(void.class, ByteBuffer.class));
      inflateMethodHandle =
          lookup.findVirtual(Inflater.class, "inflate", methodType(int.class, ByteBuffer.class));
      LOGGER.info(
          "Inflate supports ByteBuffer, buffer size defaulting to {}",
          BYTE_BUFFER_ZLIB_BUFFER_SIZE);
    } catch (Exception ignore) {
      LOGGER.info("Inflate does not support ByteBuffer, using byte arrays");
    }

    SET_INPUT_METHOD_HANDLE = setInputMethodHandle;
    INFLATE_METHOD_HANDLE = inflateMethodHandle;

    ZLIB_BUFFER_SIZE =
        (setInputMethodHandle == null || inflateMethodHandle == null)
            ? Integer.getInteger("thrift.zlib.buffer-size", 1024)
            : BYTE_BUFFER_ZLIB_BUFFER_SIZE;
  }

  public static ByteBuf inflate(ByteBufAllocator alloc, ByteBuf in) {
    Objects.requireNonNull(alloc);
    Objects.requireNonNull(in);

    try {
      if (!in.isReadable()) {
        return in;
      }

      Inflater inflater = new Inflater();

      if (supportsByteBuffer()) {
        return inflateWithByteBuffer(alloc, in, inflater);
      } else {
        return inflateWithByteArray(alloc, in, inflater);
      }
    } finally {
      if (in.refCnt() > 1) {
        in.release();
      }
    }
  }

  static boolean supportsByteBuffer() {
    return SET_INPUT_METHOD_HANDLE != null && INFLATE_METHOD_HANDLE != null;
  }

  static void setInput(Inflater inflater, ByteBuf in) {
    try {
      SET_INPUT_METHOD_HANDLE.invokeExact(inflater, in.nioBuffer());
    } catch (Throwable e) {
      throw Exceptions.propagate(e);
    }
  }

  static void inflate(ByteBufAllocator alloc, Inflater inflater, CompositeByteBuf out) {
    try {
      ByteBuf byteBuf = alloc.directBuffer(ZLIB_BUFFER_SIZE, ZLIB_BUFFER_SIZE);
      ByteBuffer byteBuffer = byteBuf.internalNioBuffer(0, ZLIB_BUFFER_SIZE);
      int r = (int) INFLATE_METHOD_HANDLE.invokeExact(inflater, byteBuffer);
      byteBuf.writerIndex(r);
      out.addComponent(true, byteBuf);
    } catch (Throwable e) {
      throw Exceptions.propagate(e);
    }
  }

  static ByteBuf inflateWithByteBuffer(ByteBufAllocator alloc, ByteBuf in, Inflater inflater) {
    CompositeByteBuf out = null;
    try {
      out = alloc.compositeBuffer();
      setInput(inflater, in);
      while (!inflater.finished()) {
        inflate(alloc, inflater, out);
      }
    } catch (Throwable e) {
      if (out != null && out.refCnt() > 0) {
        out.release();
      }
      throw Exceptions.propagate(e);
    }

    return out;
  }

  static ByteBuf inflateWithByteArray(ByteBufAllocator alloc, ByteBuf in, Inflater inflater) {
    ByteBuf out = null;
    ByteBuf buf = null;
    try {
      int readerIndex = in.readerIndex();
      int len = in.readableBytes();

      final byte[] input = ByteBufUtil.getBytes(in, readerIndex, len, false);
      inflater.setInput(input);

      out = alloc.buffer();
      buf = alloc.heapBuffer(ZLIB_BUFFER_SIZE, ZLIB_BUFFER_SIZE);
      int offset = buf.arrayOffset();
      byte[] buffer = buf.array();

      while (!inflater.finished()) {
        int read = inflater.inflate(buffer, offset, ZLIB_BUFFER_SIZE);
        buf.writerIndex(read);
        out.writeBytes(buf);
        buf.clear();
      }

      return out;
    } catch (Throwable e) {
      if (out != null) {
        ReferenceCountUtil.safeRelease(out);
      }

      throw Exceptions.propagate(e);
    } finally {
      if (buf != null && buf.refCnt() > 0) {
        buf.release();
      }
    }
  }
}
