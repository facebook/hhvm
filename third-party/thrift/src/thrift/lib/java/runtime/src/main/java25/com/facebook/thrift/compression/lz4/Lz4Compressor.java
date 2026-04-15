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

package com.facebook.thrift.compression.lz4;

import static com.facebook.thrift.compression.lz4.lz4_h.LZ4_compressBound;
import static com.facebook.thrift.compression.lz4.lz4_h.LZ4_compress_fast_extState_fastReset;
import static com.facebook.thrift.compression.lz4.lz4_h.LZ4_decompress_safe;
import static com.facebook.thrift.compression.lz4.lz4_h.LZ4_initStream;
import static com.facebook.thrift.compression.lz4.lz4_h.LZ4_sizeofState;

import com.facebook.thrift.compression.BufferUtil;
import com.facebook.thrift.compression.ThriftCompressor;
import io.netty.buffer.ByteBuf;
import io.netty.buffer.ByteBufAllocator;
import io.netty.util.ReferenceCountUtil;
import io.netty.util.concurrent.FastThreadLocal;
import io.netty.util.concurrent.FastThreadLocalThread;
import java.lang.foreign.Arena;
import java.lang.foreign.MemorySegment;

/**
 * Java 25+ LZ4 compressor using the Foreign Function &amp; Memory API to call native liblz4
 * directly via {@link lz4_h}. Replaces the base JNI version via multi-release JAR.
 *
 * <p>Uses {@code LZ4_decompress_safe()} (the safe API) and {@code
 * LZ4_compress_fast_extState_fastReset()} with a thread-local state buffer on Netty event loop
 * threads. State is explicitly initialized via {@code LZ4_initStream()} on first allocation. The
 * fastReset variant avoids re-zeroing the ~16KB hash table on each call, which benefits repeated
 * compression of small (&lt;4KB) payloads. Decompression is stateless.
 */
public final class Lz4Compressor implements ThriftCompressor {

  public static final Lz4Compressor INSTANCE = new Lz4Compressor();

  private static final int STATE_SIZE = LZ4_sizeofState();

  private static final FastThreadLocal<MemorySegment> STATE_CACHE = new FastThreadLocal<>();

  Lz4Compressor() {}

  @Override
  public ByteBuf compress(ByteBufAllocator allocator, ByteBuf data) {
    try {
      int uncompressedSize = data.readableBytes();
      int maxCompressedSize = LZ4_compressBound(uncompressedSize);

      if (maxCompressedSize <= 0) {
        throw new IllegalArgumentException(
            "LZ4 compressBound failed for input size: " + uncompressedSize);
      }

      boolean cached = Thread.currentThread() instanceof FastThreadLocalThread;
      MemorySegment state = cached ? acquireState() : allocateState();

      int varintLength = varintSize(uncompressedSize);
      ByteBuf output = allocator.directBuffer(varintLength + maxCompressedSize);
      try (Arena arena = Arena.ofConfined()) {
        writeVarint(output, uncompressedSize);

        MemorySegment source = BufferUtil.toNativeSegment(data, arena);
        MemorySegment destination =
            MemorySegment.ofAddress(output.memoryAddress() + output.writerIndex())
                .reinterpret(maxCompressedSize);

        int compressedSize =
            LZ4_compress_fast_extState_fastReset(
                state, source, destination, uncompressedSize, maxCompressedSize, 1);
        if (compressedSize <= 0) {
          throw new RuntimeException("LZ4 compression failed: returned " + compressedSize);
        }

        output.writerIndex(output.writerIndex() + compressedSize);
        return output;
      } catch (Exception exception) {
        output.release();
        throw exception;
      }
    } finally {
      ReferenceCountUtil.safeRelease(data);
    }
  }

  @Override
  public ByteBuf decompress(ByteBufAllocator allocator, ByteBuf data) {
    try {
      long uncompressedSize = readVarint(data);
      if (uncompressedSize < 0 || uncompressedSize > Integer.MAX_VALUE) {
        throw new IllegalStateException("LZ4 decompressed size out of range: " + uncompressedSize);
      }
      int destLen = (int) uncompressedSize;

      ByteBuf output = allocator.directBuffer(destLen);
      try (Arena arena = Arena.ofConfined()) {
        MemorySegment source = BufferUtil.toNativeSegment(data, arena);
        MemorySegment destination =
            MemorySegment.ofAddress(output.memoryAddress()).reinterpret(destLen);

        int result = LZ4_decompress_safe(source, destination, data.readableBytes(), destLen);
        if (result < 0) {
          throw new RuntimeException("LZ4 decompression failed: error code " + result);
        }

        output.writerIndex(result);
        return output;
      } catch (Exception exception) {
        output.release();
        throw exception;
      }
    } finally {
      ReferenceCountUtil.safeRelease(data);
    }
  }

  private static MemorySegment acquireState() {
    MemorySegment state = STATE_CACHE.get();
    if (state == null) {
      state = allocateState();
      STATE_CACHE.set(state);
    }
    return state;
  }

  private static MemorySegment allocateState() {
    MemorySegment state = Arena.ofAuto().allocate(STATE_SIZE, 8);
    MemorySegment result = LZ4_initStream(state, STATE_SIZE);
    if (result.equals(MemorySegment.NULL)) {
      throw new RuntimeException("LZ4_initStream failed: size=" + STATE_SIZE);
    }
    return state;
  }

  public static void writeVarint(ByteBuf buffer, long value) {
    while (value >= 0x80) {
      buffer.writeByte((int) (0x80 | (value & 0x7F)));
      value >>>= 7;
    }
    buffer.writeByte((int) value);
  }

  public static long readVarint(ByteBuf buffer) {
    long result = 0;
    int shift = 0;
    while (shift < 64) {
      byte b = buffer.readByte();
      result |= (long) (b & 0x7F) << shift;
      if ((b & 0x80) == 0) {
        return result;
      }
      shift += 7;
    }
    throw new IllegalStateException("Varint too long");
  }

  public static int varintSize(long value) {
    int size = 1;
    while (value >= 0x80) {
      value >>>= 7;
      size++;
    }
    return size;
  }
}
