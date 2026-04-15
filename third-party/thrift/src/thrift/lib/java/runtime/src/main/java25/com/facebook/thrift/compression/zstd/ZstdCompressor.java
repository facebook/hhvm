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

package com.facebook.thrift.compression.zstd;

import static com.facebook.thrift.compression.zstd.zstd_h.ZSTD_compressBound;
import static com.facebook.thrift.compression.zstd.zstd_h.ZSTD_compressCCtx;
import static com.facebook.thrift.compression.zstd.zstd_h.ZSTD_createCCtx;
import static com.facebook.thrift.compression.zstd.zstd_h.ZSTD_createDCtx;
import static com.facebook.thrift.compression.zstd.zstd_h.ZSTD_decompressDCtx;
import static com.facebook.thrift.compression.zstd.zstd_h.ZSTD_defaultCLevel;
import static com.facebook.thrift.compression.zstd.zstd_h.ZSTD_freeCCtx;
import static com.facebook.thrift.compression.zstd.zstd_h.ZSTD_freeDCtx;
import static com.facebook.thrift.compression.zstd.zstd_h.ZSTD_getErrorName;
import static com.facebook.thrift.compression.zstd.zstd_h.ZSTD_getFrameContentSize;
import static com.facebook.thrift.compression.zstd.zstd_h.ZSTD_isError;

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
 * Java 25+ ZSTD compressor using the Foreign Function &amp; Memory API to call native libzstd
 * directly via {@link zstd_h}. Replaces the base JNI version via multi-release JAR.
 *
 * <p>For direct Netty ByteBufs, compression and decompression are fully zero-copy via {@link
 * MemorySegment#ofAddress}. Thread-local ZSTD_CCtx and ZSTD_DCtx are cached on Netty event loop
 * threads to avoid native context allocation per request.
 */
public final class ZstdCompressor implements ThriftCompressor {

  public static final ZstdCompressor INSTANCE = new ZstdCompressor();

  private static final int DEFAULT_LEVEL = ZSTD_defaultCLevel();

  private static final FastThreadLocal<MemorySegment> CCTX_CACHE =
      new FastThreadLocal<>() {
        @Override
        protected void onRemoval(MemorySegment value) {
          ZSTD_freeCCtx(value);
        }
      };

  private static final FastThreadLocal<MemorySegment> DCTX_CACHE =
      new FastThreadLocal<>() {
        @Override
        protected void onRemoval(MemorySegment value) {
          ZSTD_freeDCtx(value);
        }
      };

  ZstdCompressor() {}

  @Override
  public ByteBuf compress(ByteBufAllocator allocator, ByteBuf data) {
    try {
      int readableBytes = data.readableBytes();
      long maxCompressedSize = ZSTD_compressBound(readableBytes);
      if (ZSTD_isError(maxCompressedSize)) {
        throw new IllegalArgumentException(
            "ZSTD compressBound failed: " + ZSTD_getErrorName(maxCompressedSize));
      }
      if (maxCompressedSize > Integer.MAX_VALUE) {
        throw new IllegalArgumentException(
            "ZSTD compressed bound exceeds maximum buffer size: " + maxCompressedSize);
      }

      boolean cached = Thread.currentThread() instanceof FastThreadLocalThread;
      MemorySegment cctx = cached ? acquireCCtx() : ZSTD_createCCtx();

      ByteBuf output = null;
      try (Arena arena = Arena.ofConfined()) {
        MemorySegment source = BufferUtil.toNativeSegment(data, arena);

        output = allocator.directBuffer((int) maxCompressedSize);
        MemorySegment destination =
            MemorySegment.ofAddress(output.memoryAddress()).reinterpret(maxCompressedSize);

        long compressedSize =
            ZSTD_compressCCtx(
                cctx, destination, maxCompressedSize, source, readableBytes, DEFAULT_LEVEL);
        if (ZSTD_isError(compressedSize)) {
          throw new RuntimeException(
              "ZSTD compression failed: " + ZSTD_getErrorName(compressedSize));
        }

        output.writerIndex((int) compressedSize);
        return output;
      } catch (Exception exception) {
        ReferenceCountUtil.safeRelease(output);
        throw exception;
      } finally {
        if (!cached) {
          ZSTD_freeCCtx(cctx);
        }
      }
    } finally {
      ReferenceCountUtil.safeRelease(data);
    }
  }

  @Override
  public ByteBuf decompress(ByteBufAllocator allocator, ByteBuf data) {
    try {
      boolean cached = Thread.currentThread() instanceof FastThreadLocalThread;
      MemorySegment dctx = cached ? acquireDCtx() : ZSTD_createDCtx();

      ByteBuf output = null;
      try (Arena arena = Arena.ofConfined()) {
        MemorySegment source = BufferUtil.toNativeSegment(data, arena);

        long decompressedSize = ZSTD_getFrameContentSize(source, data.readableBytes());
        if (decompressedSize < 0) {
          throw new IllegalStateException(
              "Cannot determine ZSTD decompressed size from frame header");
        }
        if (decompressedSize > Integer.MAX_VALUE) {
          throw new IllegalStateException(
              "ZSTD decompressed size exceeds maximum: " + decompressedSize);
        }

        output = allocator.directBuffer((int) decompressedSize);
        MemorySegment destination =
            MemorySegment.ofAddress(output.memoryAddress()).reinterpret(decompressedSize);

        long result =
            ZSTD_decompressDCtx(dctx, destination, decompressedSize, source, data.readableBytes());
        if (ZSTD_isError(result)) {
          throw new RuntimeException("ZSTD decompression failed: " + ZSTD_getErrorName(result));
        }

        output.writerIndex((int) result);
        return output;
      } catch (Exception exception) {
        ReferenceCountUtil.safeRelease(output);
        throw exception;
      } finally {
        if (!cached) {
          ZSTD_freeDCtx(dctx);
        }
      }
    } finally {
      ReferenceCountUtil.safeRelease(data);
    }
  }

  private static MemorySegment acquireCCtx() {
    MemorySegment cctx = CCTX_CACHE.get();
    if (cctx == null) {
      cctx = ZSTD_createCCtx();
      CCTX_CACHE.set(cctx);
    }
    return cctx;
  }

  private static MemorySegment acquireDCtx() {
    MemorySegment dctx = DCTX_CACHE.get();
    if (dctx == null) {
      dctx = ZSTD_createDCtx();
      DCTX_CACHE.set(dctx);
    }
    return dctx;
  }
}
