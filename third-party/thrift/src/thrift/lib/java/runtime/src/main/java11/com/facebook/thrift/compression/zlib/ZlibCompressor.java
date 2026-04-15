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

package com.facebook.thrift.compression.zlib;

import com.facebook.thrift.compression.ThriftCompressor;
import io.netty.buffer.ByteBuf;
import io.netty.buffer.ByteBufAllocator;
import io.netty.util.ReferenceCountUtil;
import io.netty.util.concurrent.FastThreadLocal;
import io.netty.util.concurrent.FastThreadLocalThread;
import java.nio.ByteBuffer;
import java.util.zip.DataFormatException;
import java.util.zip.Deflater;
import java.util.zip.Inflater;
import reactor.core.Exceptions;

/**
 * Java 11+ ZLIB compressor that uses ByteBuffer-based Deflater/Inflater APIs for zero-copy I/O.
 * Replaces the base Java 8 version via multi-release JAR.
 *
 * <p>Uses {@link Deflater#setInput(ByteBuffer)} + {@link Deflater#deflate(ByteBuffer)} and {@link
 * Inflater#setInput(ByteBuffer)} + {@link Inflater#inflate(ByteBuffer)} (added in Java 11) with
 * {@link ByteBuf#nioBuffer()} to avoid all heap byte array copies.
 *
 * <p>On Netty event loop threads ({@link FastThreadLocalThread}), Deflater and Inflater instances
 * are cached in {@link FastThreadLocal} to avoid native resource allocation per request.
 */
public final class ZlibCompressor implements ThriftCompressor {

  public static final ZlibCompressor INSTANCE = new ZlibCompressor();

  private static final int MIN_WRITABLE_BYTES = 1024;

  private static final FastThreadLocal<Deflater> DEFLATER_CACHE = new FastThreadLocal<>();
  private static final FastThreadLocal<Inflater> INFLATER_CACHE = new FastThreadLocal<>();

  ZlibCompressor() {}

  @Override
  public ByteBuf compress(ByteBufAllocator allocator, ByteBuf data) {
    ByteBuf output = null;
    boolean cached = Thread.currentThread() instanceof FastThreadLocalThread;
    Deflater deflater = cached ? acquireDeflater() : new Deflater();
    try {
      deflater.setInput(data.nioBuffer());
      deflater.finish();

      output = allocator.directBuffer();
      while (!deflater.finished()) {
        output.ensureWritable(MIN_WRITABLE_BYTES);
        ByteBuffer destination = output.nioBuffer(output.writerIndex(), output.writableBytes());
        int written = deflater.deflate(destination);
        output.writerIndex(output.writerIndex() + written);
      }

      return output;
    } catch (Exception exception) {
      ReferenceCountUtil.safeRelease(output);
      throw exception;
    } finally {
      ReferenceCountUtil.safeRelease(data);
      if (cached) {
        deflater.reset();
      } else {
        deflater.end();
      }
    }
  }

  @Override
  public ByteBuf decompress(ByteBufAllocator allocator, ByteBuf data) {
    ByteBuf output = null;
    boolean cached = Thread.currentThread() instanceof FastThreadLocalThread;
    Inflater inflater = cached ? acquireInflater() : new Inflater();
    try {
      inflater.setInput(data.nioBuffer());

      output = allocator.directBuffer();
      while (!inflater.finished()) {
        output.ensureWritable(MIN_WRITABLE_BYTES);
        ByteBuffer destination = output.nioBuffer(output.writerIndex(), output.writableBytes());
        int written = inflater.inflate(destination);
        output.writerIndex(output.writerIndex() + written);
        if (written == 0 && inflater.needsInput()) {
          break;
        }
      }

      return output;
    } catch (DataFormatException exception) {
      ReferenceCountUtil.safeRelease(output);
      throw Exceptions.propagate(exception);
    } catch (Exception exception) {
      ReferenceCountUtil.safeRelease(output);
      throw exception;
    } finally {
      ReferenceCountUtil.safeRelease(data);
      if (cached) {
        inflater.reset();
      } else {
        inflater.end();
      }
    }
  }

  private static Deflater acquireDeflater() {
    Deflater deflater = DEFLATER_CACHE.get();
    if (deflater == null) {
      deflater = new Deflater();
      DEFLATER_CACHE.set(deflater);
    }
    return deflater;
  }

  private static Inflater acquireInflater() {
    Inflater inflater = INFLATER_CACHE.get();
    if (inflater == null) {
      inflater = new Inflater();
      INFLATER_CACHE.set(inflater);
    }
    return inflater;
  }
}
