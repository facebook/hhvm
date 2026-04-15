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

package com.facebook.thrift.compression;

import io.netty.buffer.ByteBuf;
import java.lang.foreign.Arena;
import java.lang.foreign.MemorySegment;
import java.nio.ByteBuffer;

/**
 * Utilities for exposing Netty {@link ByteBuf} instances to native codecs through the Foreign
 * Function &amp; Memory API.
 *
 * <p>The helper prefers zero-copy views when the readable region is already backed by a single
 * native allocation. Otherwise, it copies the readable bytes into native memory owned by the caller
 * provided {@link Arena}.
 */
public final class BufferUtil {

  private BufferUtil() {}

  /**
   * Returns a native {@link MemorySegment} covering the readable bytes of {@code buf}.
   *
   * <p>The returned segment is always scoped to {@code arena}. For zero-copy paths we retain a
   * readable slice and register its release as arena cleanup so the Netty-managed memory stays
   * alive for the full segment lifetime. For non-native or non-contiguous buffers we allocate fresh
   * native memory from the arena and copy the bytes into it.
   */
  public static MemorySegment toNativeSegment(ByteBuf buf, Arena arena) {
    // Check the original buffer for a stable address/contiguous layout before slicing. The slice
    // gives us the correct offset-adjusted view and the retained lifetime we need for native calls.
    boolean useAddress = buf.hasMemoryAddress() && buf.isContiguous();
    ByteBuf slice = buf.retainedSlice();
    boolean release = true;
    try {
      int length = slice.readableBytes();

      // Fast path: the readable bytes are already one contiguous native region, so wrap the
      // address directly and hand slice release to the arena.
      if (useAddress && slice.hasMemoryAddress()) {
        MemorySegment segment =
            MemorySegment.ofAddress(slice.memoryAddress())
                .reinterpret(length, arena, ignored -> slice.release());
        release = false;
        return segment;
      }

      // Fallback zero-copy path for direct buffers that cannot expose a raw address but can
      // still provide a single direct ByteBuffer view.
      if (slice.nioBufferCount() == 1) {
        ByteBuffer nioBuffer = slice.nioBuffer();
        if (nioBuffer.isDirect()) {
          MemorySegment segment =
              MemorySegment.ofBuffer(nioBuffer).reinterpret(arena, ignored -> slice.release());
          release = false;
          return segment;
        }
      }

      // Final fallback: materialize the readable bytes into fresh native memory owned by the arena.
      return copyToNativeSegment(slice, arena);
    } finally {
      if (release) {
        slice.release();
      }
    }
  }

  private static MemorySegment copyToNativeSegment(ByteBuf slice, Arena arena) {
    int length = slice.readableBytes();

    // The compression bindings treat payloads as raw byte buffers, so byte alignment is sufficient.
    MemorySegment destination = arena.allocate(length, 1);
    if (length == 0) {
      return destination;
    }

    // Heap-backed slices can copy directly from their backing array into native memory.
    if (slice.hasArray()) {
      int offset = slice.arrayOffset() + slice.readerIndex();
      destination.copyFrom(MemorySegment.ofArray(slice.array()).asSlice(offset, length));
      return destination;
    }

    // Generic fallback for composite or opaque buffers: let Netty write into the native buffer's
    // direct ByteBuffer view.
    slice.getBytes(slice.readerIndex(), destination.asByteBuffer());
    return destination;
  }
}
