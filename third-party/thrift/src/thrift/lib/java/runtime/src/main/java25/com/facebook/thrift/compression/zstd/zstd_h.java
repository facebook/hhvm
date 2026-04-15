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

import com.facebook.thrift.compression.NativeLibraryLoader;
import java.lang.foreign.FunctionDescriptor;
import java.lang.foreign.Linker;
import java.lang.foreign.MemorySegment;
import java.lang.foreign.SymbolLookup;
import java.lang.foreign.ValueLayout;
import java.lang.invoke.MethodHandle;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 * FFI bindings for libzstd, following jextract naming conventions. Provides static method wrappers
 * for each native C function, backed by pre-resolved {@link MethodHandle} downcall handles.
 *
 * <p>Library loading searches LD_LIBRARY_PATH (JEX nativelibs/) then system directories. Version
 * and path are logged at load time for diagnostics.
 */
public final class zstd_h {

  private static final Logger LOGGER = LoggerFactory.getLogger(zstd_h.class);
  private static final boolean IS_MAC =
      System.getProperty("os.name", "").toLowerCase().contains("mac");
  private static final String REQUIRED_SYMBOL = "ZSTD_versionString";
  private static final String[] LIBRARY_FILE_NAMES =
      IS_MAC
          ? new String[] {"libzstd.dylib", "libzstd.1.dylib"}
          : new String[] {"libzstd.so", "libzstd.so.1"};

  private static final Linker LINKER = Linker.nativeLinker();
  private static final SymbolLookup LIB = loadLibrary();

  // size_t ZSTD_compressBound(size_t srcSize)
  private static final MethodHandle ZSTD_compressBound$MH =
      LINKER.downcallHandle(
          LIB.find("ZSTD_compressBound").orElseThrow(),
          FunctionDescriptor.of(ValueLayout.JAVA_LONG, ValueLayout.JAVA_LONG));

  // size_t ZSTD_compressCCtx(ZSTD_CCtx*, void* dst, size_t dstCap, const void* src, size_t
  // srcSize, int level)
  private static final MethodHandle ZSTD_compressCCtx$MH =
      LINKER.downcallHandle(
          LIB.find("ZSTD_compressCCtx").orElseThrow(),
          FunctionDescriptor.of(
              ValueLayout.JAVA_LONG,
              ValueLayout.ADDRESS,
              ValueLayout.ADDRESS,
              ValueLayout.JAVA_LONG,
              ValueLayout.ADDRESS,
              ValueLayout.JAVA_LONG,
              ValueLayout.JAVA_INT));

  // size_t ZSTD_decompressDCtx(ZSTD_DCtx*, void* dst, size_t dstCap, const void* src, size_t
  // srcSize)
  private static final MethodHandle ZSTD_decompressDCtx$MH =
      LINKER.downcallHandle(
          LIB.find("ZSTD_decompressDCtx").orElseThrow(),
          FunctionDescriptor.of(
              ValueLayout.JAVA_LONG,
              ValueLayout.ADDRESS,
              ValueLayout.ADDRESS,
              ValueLayout.JAVA_LONG,
              ValueLayout.ADDRESS,
              ValueLayout.JAVA_LONG));

  // ZSTD_CCtx* ZSTD_createCCtx(void)
  private static final MethodHandle ZSTD_createCCtx$MH =
      LINKER.downcallHandle(
          LIB.find("ZSTD_createCCtx").orElseThrow(), FunctionDescriptor.of(ValueLayout.ADDRESS));

  // size_t ZSTD_freeCCtx(ZSTD_CCtx*)
  private static final MethodHandle ZSTD_freeCCtx$MH =
      LINKER.downcallHandle(
          LIB.find("ZSTD_freeCCtx").orElseThrow(),
          FunctionDescriptor.of(ValueLayout.JAVA_LONG, ValueLayout.ADDRESS));

  // ZSTD_DCtx* ZSTD_createDCtx(void)
  private static final MethodHandle ZSTD_createDCtx$MH =
      LINKER.downcallHandle(
          LIB.find("ZSTD_createDCtx").orElseThrow(), FunctionDescriptor.of(ValueLayout.ADDRESS));

  // size_t ZSTD_freeDCtx(ZSTD_DCtx*)
  private static final MethodHandle ZSTD_freeDCtx$MH =
      LINKER.downcallHandle(
          LIB.find("ZSTD_freeDCtx").orElseThrow(),
          FunctionDescriptor.of(ValueLayout.JAVA_LONG, ValueLayout.ADDRESS));

  // unsigned long long ZSTD_getFrameContentSize(const void* src, size_t srcSize)
  private static final MethodHandle ZSTD_getFrameContentSize$MH =
      LINKER.downcallHandle(
          LIB.find("ZSTD_getFrameContentSize").orElseThrow(),
          FunctionDescriptor.of(ValueLayout.JAVA_LONG, ValueLayout.ADDRESS, ValueLayout.JAVA_LONG));

  // unsigned ZSTD_isError(size_t code)
  private static final MethodHandle ZSTD_isError$MH =
      LINKER.downcallHandle(
          LIB.find("ZSTD_isError").orElseThrow(),
          FunctionDescriptor.of(ValueLayout.JAVA_INT, ValueLayout.JAVA_LONG));

  // const char* ZSTD_getErrorName(size_t code)
  private static final MethodHandle ZSTD_getErrorName$MH =
      LINKER.downcallHandle(
          LIB.find("ZSTD_getErrorName").orElseThrow(),
          FunctionDescriptor.of(ValueLayout.ADDRESS, ValueLayout.JAVA_LONG));

  // int ZSTD_defaultCLevel(void)
  private static final MethodHandle ZSTD_defaultCLevel$MH =
      LINKER.downcallHandle(
          LIB.find("ZSTD_defaultCLevel").orElseThrow(),
          FunctionDescriptor.of(ValueLayout.JAVA_INT));

  private zstd_h() {}

  public static long ZSTD_compressBound(long srcSize) {
    try {
      return (long) ZSTD_compressBound$MH.invokeExact(srcSize);
    } catch (Throwable ex$) {
      throw new AssertionError("should not reach here", ex$);
    }
  }

  public static long ZSTD_compressCCtx(
      MemorySegment cctx,
      MemorySegment dst,
      long dstCapacity,
      MemorySegment src,
      long srcSize,
      int level) {
    try {
      return (long) ZSTD_compressCCtx$MH.invokeExact(cctx, dst, dstCapacity, src, srcSize, level);
    } catch (Throwable ex$) {
      throw new AssertionError("should not reach here", ex$);
    }
  }

  public static long ZSTD_decompressDCtx(
      MemorySegment dctx, MemorySegment dst, long dstCapacity, MemorySegment src, long srcSize) {
    try {
      return (long) ZSTD_decompressDCtx$MH.invokeExact(dctx, dst, dstCapacity, src, srcSize);
    } catch (Throwable ex$) {
      throw new AssertionError("should not reach here", ex$);
    }
  }

  public static MemorySegment ZSTD_createCCtx() {
    MemorySegment cctx;
    try {
      cctx = (MemorySegment) ZSTD_createCCtx$MH.invokeExact();
    } catch (Throwable ex$) {
      throw new AssertionError("should not reach here", ex$);
    }
    if (cctx.equals(MemorySegment.NULL)) {
      throw new OutOfMemoryError("ZSTD_createCCtx failed: native memory allocation failed");
    }
    return cctx;
  }

  public static void ZSTD_freeCCtx(MemorySegment cctx) {
    try {
      long ignored = (long) ZSTD_freeCCtx$MH.invokeExact(cctx);
    } catch (Throwable ex$) {
      throw new AssertionError("should not reach here", ex$);
    }
  }

  public static MemorySegment ZSTD_createDCtx() {
    MemorySegment dctx;
    try {
      dctx = (MemorySegment) ZSTD_createDCtx$MH.invokeExact();
    } catch (Throwable ex$) {
      throw new AssertionError("should not reach here", ex$);
    }
    if (dctx.equals(MemorySegment.NULL)) {
      throw new OutOfMemoryError("ZSTD_createDCtx failed: native memory allocation failed");
    }
    return dctx;
  }

  public static void ZSTD_freeDCtx(MemorySegment dctx) {
    try {
      long ignored = (long) ZSTD_freeDCtx$MH.invokeExact(dctx);
    } catch (Throwable ex$) {
      throw new AssertionError("should not reach here", ex$);
    }
  }

  public static long ZSTD_getFrameContentSize(MemorySegment src, long srcSize) {
    try {
      return (long) ZSTD_getFrameContentSize$MH.invokeExact(src, srcSize);
    } catch (Throwable ex$) {
      throw new AssertionError("should not reach here", ex$);
    }
  }

  public static boolean ZSTD_isError(long code) {
    try {
      return (int) ZSTD_isError$MH.invokeExact(code) != 0;
    } catch (Throwable ex$) {
      throw new AssertionError("should not reach here", ex$);
    }
  }

  public static String ZSTD_getErrorName(long code) {
    try {
      MemorySegment namePtr = (MemorySegment) ZSTD_getErrorName$MH.invokeExact(code);
      return namePtr.reinterpret(256).getString(0);
    } catch (Throwable ex$) {
      throw new AssertionError("should not reach here", ex$);
    }
  }

  public static int ZSTD_defaultCLevel() {
    try {
      return (int) ZSTD_defaultCLevel$MH.invokeExact();
    } catch (Throwable ex$) {
      throw new AssertionError("should not reach here", ex$);
    }
  }

  private static SymbolLookup loadLibrary() {
    NativeLibraryLoader.LoadResult result =
        NativeLibraryLoader.findLibrary(REQUIRED_SYMBOL, LIBRARY_FILE_NAMES);
    if (result == null) {
      result = NativeLibraryLoader.findLibraryByName(REQUIRED_SYMBOL, "zstd");
    }
    if (result != null) {
      String version = NativeLibraryLoader.queryVersion(result.lookup(), REQUIRED_SYMBOL);
      LOGGER.info(
          "ZSTD FFI loaded: version={}, source={}, path={}",
          version,
          result.source(),
          result.path());
      return result.lookup();
    }

    throw new RuntimeException(
        "Cannot load libzstd for FFI: not found in LD_LIBRARY_PATH or system directories");
  }
}
