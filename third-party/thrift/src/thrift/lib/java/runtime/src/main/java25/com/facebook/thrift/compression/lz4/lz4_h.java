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
 * FFI bindings for liblz4, following jextract naming conventions. Provides static method wrappers
 * for each native C function, backed by pre-resolved {@link MethodHandle} downcall handles.
 *
 * <p>Uses the safe decompression API ({@code LZ4_decompress_safe}) and external-state compression
 * ({@code LZ4_compress_fast_extState}) for thread-local state reuse.
 */
public final class lz4_h {

  private static final Logger LOGGER = LoggerFactory.getLogger(lz4_h.class);
  private static final boolean IS_MAC =
      System.getProperty("os.name", "").toLowerCase().contains("mac");
  private static final String REQUIRED_SYMBOL = "LZ4_versionString";
  private static final String[] LIBRARY_FILE_NAMES =
      IS_MAC
          ? new String[] {"liblz4.dylib", "liblz4.1.dylib"}
          : new String[] {"liblz4.so", "liblz4.so.1"};

  private static final Linker LINKER = Linker.nativeLinker();
  private static final SymbolLookup LIB = loadLibrary();

  // int LZ4_compressBound(int inputSize)
  private static final MethodHandle LZ4_compressBound$MH =
      LINKER.downcallHandle(
          LIB.find("LZ4_compressBound").orElseThrow(),
          FunctionDescriptor.of(ValueLayout.JAVA_INT, ValueLayout.JAVA_INT));

  // int LZ4_sizeofState(void)
  private static final MethodHandle LZ4_sizeofState$MH =
      LINKER.downcallHandle(
          LIB.find("LZ4_sizeofState").orElseThrow(), FunctionDescriptor.of(ValueLayout.JAVA_INT));

  // LZ4_stream_t* LZ4_initStream(void* buffer, size_t size)
  private static final MethodHandle LZ4_initStream$MH =
      LINKER.downcallHandle(
          LIB.find("LZ4_initStream").orElseThrow(),
          FunctionDescriptor.of(ValueLayout.ADDRESS, ValueLayout.ADDRESS, ValueLayout.JAVA_LONG));

  // int LZ4_compress_fast_extState_fastReset(void* state, const char* src, char* dst,
  //                                         int srcSize, int dstCapacity, int acceleration)
  private static final MethodHandle LZ4_compress_fast_extState_fastReset$MH =
      LINKER.downcallHandle(
          LIB.find("LZ4_compress_fast_extState_fastReset").orElseThrow(),
          FunctionDescriptor.of(
              ValueLayout.JAVA_INT,
              ValueLayout.ADDRESS,
              ValueLayout.ADDRESS,
              ValueLayout.ADDRESS,
              ValueLayout.JAVA_INT,
              ValueLayout.JAVA_INT,
              ValueLayout.JAVA_INT));

  // int LZ4_decompress_safe(const char* src, char* dst, int compressedSize, int dstCapacity)
  private static final MethodHandle LZ4_decompress_safe$MH =
      LINKER.downcallHandle(
          LIB.find("LZ4_decompress_safe").orElseThrow(),
          FunctionDescriptor.of(
              ValueLayout.JAVA_INT,
              ValueLayout.ADDRESS,
              ValueLayout.ADDRESS,
              ValueLayout.JAVA_INT,
              ValueLayout.JAVA_INT));

  private lz4_h() {}

  public static int LZ4_compressBound(int inputSize) {
    try {
      return (int) LZ4_compressBound$MH.invokeExact(inputSize);
    } catch (Throwable ex$) {
      throw new AssertionError("should not reach here", ex$);
    }
  }

  public static int LZ4_sizeofState() {
    try {
      return (int) LZ4_sizeofState$MH.invokeExact();
    } catch (Throwable ex$) {
      throw new AssertionError("should not reach here", ex$);
    }
  }

  public static MemorySegment LZ4_initStream(MemorySegment buffer, long size) {
    try {
      return (MemorySegment) LZ4_initStream$MH.invokeExact(buffer, size);
    } catch (Throwable ex$) {
      throw new AssertionError("should not reach here", ex$);
    }
  }

  public static int LZ4_compress_fast_extState_fastReset(
      MemorySegment state,
      MemorySegment src,
      MemorySegment dst,
      int srcSize,
      int dstCapacity,
      int acceleration) {
    try {
      return (int)
          LZ4_compress_fast_extState_fastReset$MH.invokeExact(
              state, src, dst, srcSize, dstCapacity, acceleration);
    } catch (Throwable ex$) {
      throw new AssertionError("should not reach here", ex$);
    }
  }

  public static int LZ4_decompress_safe(
      MemorySegment src, MemorySegment dst, int compressedSize, int dstCapacity) {
    try {
      return (int) LZ4_decompress_safe$MH.invokeExact(src, dst, compressedSize, dstCapacity);
    } catch (Throwable ex$) {
      throw new AssertionError("should not reach here", ex$);
    }
  }

  private static SymbolLookup loadLibrary() {
    NativeLibraryLoader.LoadResult result =
        NativeLibraryLoader.findLibrary(REQUIRED_SYMBOL, LIBRARY_FILE_NAMES);
    if (result == null) {
      result = NativeLibraryLoader.findLibraryByName(REQUIRED_SYMBOL, "lz4");
    }
    if (result != null) {
      String version = NativeLibraryLoader.queryVersion(result.lookup(), REQUIRED_SYMBOL);
      LOGGER.info(
          "LZ4 FFI loaded: version={}, source={}, path={}",
          version,
          result.source(),
          result.path());
      return result.lookup();
    }

    throw new RuntimeException(
        "Cannot load liblz4 for FFI: not found in LD_LIBRARY_PATH or system directories");
  }
}
