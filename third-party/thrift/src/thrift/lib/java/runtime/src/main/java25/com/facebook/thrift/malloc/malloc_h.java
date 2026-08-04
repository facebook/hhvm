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

package com.facebook.thrift.malloc;

import java.lang.foreign.FunctionDescriptor;
import java.lang.foreign.Linker;
import java.lang.foreign.MemoryLayout;
import java.lang.foreign.MemorySegment;
import java.lang.foreign.SegmentAllocator;
import java.lang.foreign.StructLayout;
import java.lang.foreign.SymbolLookup;
import java.lang.foreign.ValueLayout;
import java.lang.invoke.MethodHandle;
import java.util.Optional;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 * FFI bindings for the glibc {@code mallinfo2(3)} allocator-statistics call, following jextract
 * naming conventions. Derived from jextract output over {@code <malloc.h>}, reduced to the single
 * call this runtime needs.
 *
 * <p>{@code mallinfo2} is a glibc extension. It is absent under jemalloc, tcmalloc, musl, and on
 * non-Linux platforms, so resolution is strictly best effort: {@link #isAvailable()} reports
 * whether the symbol was found and no code path here throws during class initialization. This is
 * the reason the binding uses {@link SymbolLookup#find} (returning an {@code Optional}) rather than
 * jextract's generated {@code findOrThrow}, which would raise {@code ExceptionInInitializerError}
 * on the first touch when the symbol is missing.
 *
 * <p>Unlike the older {@code mallinfo(3)}, {@code mallinfo2} uses {@code size_t} fields and so does
 * not wrap past 4GB. It aggregates every arena, not just the main one, and therefore reflects
 * whole-process glibc allocator state for a multi-threaded JVM.
 */
public final class malloc_h {

  private static final Logger LOGGER = LoggerFactory.getLogger(malloc_h.class);

  private static final String MALLINFO2_SYMBOL = "mallinfo2";

  /** {@code size_t}. Eight bytes on every LP64 platform where glibc exports {@code mallinfo2}. */
  private static final ValueLayout.OfLong C_SIZE_T = resolveSizeT();

  /**
   * {@snippet lang=c : struct mallinfo2 { size_t arena; size_t ordblks; size_t smblks; size_t
   * hblks; size_t hblkhd; size_t usmblks; size_t fsmblks; size_t uordblks; size_t fordblks; size_t
   * keepcost; } }
   *
   * <p>Field order is ABI-frozen; {@code usmblks} is retained in the layout purely to keep the
   * subsequent field offsets correct (glibc always reports it as 0).
   */
  public static final StructLayout MALLINFO2 =
      MemoryLayout.structLayout(
          C_SIZE_T.withName("arena"),
          C_SIZE_T.withName("ordblks"),
          C_SIZE_T.withName("smblks"),
          C_SIZE_T.withName("hblks"),
          C_SIZE_T.withName("hblkhd"),
          C_SIZE_T.withName("usmblks"),
          C_SIZE_T.withName("fsmblks"),
          C_SIZE_T.withName("uordblks"),
          C_SIZE_T.withName("fordblks"),
          C_SIZE_T.withName("keepcost"));

  private static final long OFFSET_ARENA = offsetOf("arena");
  private static final long OFFSET_ORDBLKS = offsetOf("ordblks");
  private static final long OFFSET_SMBLKS = offsetOf("smblks");
  private static final long OFFSET_HBLKS = offsetOf("hblks");
  private static final long OFFSET_HBLKHD = offsetOf("hblkhd");
  private static final long OFFSET_FSMBLKS = offsetOf("fsmblks");
  private static final long OFFSET_UORDBLKS = offsetOf("uordblks");
  private static final long OFFSET_FORDBLKS = offsetOf("fordblks");
  private static final long OFFSET_KEEPCOST = offsetOf("keepcost");

  /** Null when the symbol could not be resolved. Never throws out of the static initializer. */
  private static final MethodHandle MALLINFO2_HANDLE = resolveMallinfo2();

  private malloc_h() {}

  /**
   * Whether {@code mallinfo2} was resolved and can be called. False under a non-glibc allocator, on
   * non-Linux platforms, or when native access is denied.
   */
  public static boolean isAvailable() {
    return MALLINFO2_HANDLE != null;
  }

  /**
   * {@snippet lang=c : extern struct mallinfo2 mallinfo2(void) }
   *
   * <p>The struct is 80 bytes and so is returned via a hidden pointer under the SysV ABI, which is
   * why an allocator is required. Callers should pass a confined {@link java.lang.foreign.Arena}.
   *
   * @throws UnsupportedOperationException if {@link #isAvailable()} is false
   */
  public static MemorySegment mallinfo2(SegmentAllocator allocator) {
    MethodHandle handle = MALLINFO2_HANDLE;
    if (handle == null) {
      throw new UnsupportedOperationException("mallinfo2 is not available on this platform");
    }
    try {
      return (MemorySegment) handle.invokeExact(allocator);
    } catch (Error | RuntimeException ex) {
      throw ex;
    } catch (Throwable ex) {
      throw new AssertionError("should not reach here", ex);
    }
  }

  /** Total bytes obtained from the system in non-mmapped arenas (sbrk plus per-thread heaps). */
  public static long arena(MemorySegment info) {
    return info.get(C_SIZE_T, OFFSET_ARENA);
  }

  /** Number of free chunks across all arenas. */
  public static long ordblks(MemorySegment info) {
    return info.get(C_SIZE_T, OFFSET_ORDBLKS);
  }

  /** Number of free fastbin chunks. */
  public static long smblks(MemorySegment info) {
    return info.get(C_SIZE_T, OFFSET_SMBLKS);
  }

  /** Number of live mmapped regions (allocations above the mmap threshold). */
  public static long hblks(MemorySegment info) {
    return info.get(C_SIZE_T, OFFSET_HBLKS);
  }

  /** Bytes held in mmapped regions. */
  public static long hblkhd(MemorySegment info) {
    return info.get(C_SIZE_T, OFFSET_HBLKHD);
  }

  /** Bytes held in free fastbin chunks. */
  public static long fsmblks(MemorySegment info) {
    return info.get(C_SIZE_T, OFFSET_FSMBLKS);
  }

  /** Bytes currently handed out to the application. */
  public static long uordblks(MemorySegment info) {
    return info.get(C_SIZE_T, OFFSET_UORDBLKS);
  }

  /** Bytes free inside the arenas: retained by glibc, not returned to the OS. */
  public static long fordblks(MemorySegment info) {
    return info.get(C_SIZE_T, OFFSET_FORDBLKS);
  }

  /**
   * Size of the main arena's top chunk. This is a floor on what can be reclaimed, not an estimate
   * of it: glibc sets {@code keepcost} only for the main arena, while {@code malloc_trim} walks
   * every arena and madvises free pages out of any chunk, so it routinely releases far more.
   */
  public static long keepcost(MemorySegment info) {
    return info.get(C_SIZE_T, OFFSET_KEEPCOST);
  }

  private static ValueLayout.OfLong resolveSizeT() {
    try {
      ValueLayout layout = (ValueLayout) Linker.nativeLinker().canonicalLayouts().get("size_t");
      if (layout instanceof ValueLayout.OfLong sizeT) {
        return sizeT;
      }
    } catch (Throwable t) {
      LOGGER.debug("Could not resolve the canonical size_t layout; assuming 64-bit", t);
    }
    return ValueLayout.JAVA_LONG;
  }

  private static long offsetOf(String fieldName) {
    return MALLINFO2.byteOffset(MemoryLayout.PathElement.groupElement(fieldName));
  }

  private static MethodHandle resolveMallinfo2() {
    // Every failure mode here is expected on some supported deployment (non-glibc allocator,
    // non-Linux, or native access denied by --illegal-native-access=deny), so nothing escapes.
    try {
      Linker linker = Linker.nativeLinker();
      Optional<MemorySegment> address =
          SymbolLookup.loaderLookup().or(linker.defaultLookup()).find(MALLINFO2_SYMBOL);
      // mallinfo2 is a weak symbol, so it can resolve to a null address rather than be absent.
      if (address.isEmpty() || address.get().equals(MemorySegment.NULL)) {
        return null;
      }
      return linker.downcallHandle(address.get(), FunctionDescriptor.of(MALLINFO2));
    } catch (Throwable t) {
      LOGGER.debug("mallinfo2 could not be resolved; glibc allocator stats are disabled", t);
      return null;
    }
  }
}
