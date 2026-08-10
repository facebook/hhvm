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

import java.lang.foreign.Arena;
import java.lang.foreign.FunctionDescriptor;
import java.lang.foreign.Linker;
import java.lang.foreign.MemorySegment;
import java.lang.foreign.SymbolLookup;
import java.lang.foreign.ValueLayout;
import java.lang.invoke.MethodHandle;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.List;
import java.util.Optional;
import java.util.OptionalLong;
import java.util.stream.Collectors;
import java.util.stream.Stream;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 * FFI bindings for jemalloc's {@code mallctl(3)} introspection interface.
 *
 * <p>{@code mallctl} is jemalloc's single entry point for reading statistics and options. It is a
 * dynamically typed interface: the caller passes a buffer and its length, and jemalloc rejects the
 * call with {@code EINVAL} if the width does not match the type it holds for that name. Those types
 * live in jemalloc's {@code ctl.c}, not in any header, so each accessor here fixes the width and
 * every counter must be read through the matching one.
 *
 * <p>Absent under glibc, tcmalloc, and musl, so resolution is best effort exactly as in {@link
 * malloc_h}: {@link #isAvailable()} reports whether the symbol was found and nothing here throws
 * during class initialization. jemalloc declares {@code mallctl} weak, so a present-but-null
 * address is possible and is treated as absent.
 *
 * <p>Statistics are cached. {@link #refreshEpoch()} must be called before a batch of reads or the
 * values returned are those from the previous refresh.
 */
public final class jemalloc_h {

  private static final Logger LOGGER = LoggerFactory.getLogger(jemalloc_h.class);

  /** Index that selects the merged view over all arenas ({@code MALLCTL_ARENAS_ALL}). */
  public static final int ARENAS_ALL = 4096;

  private static final ValueLayout.OfLong C_SIZE_T = resolveSizeT();

  /**
   * {@snippet lang=c : int mallctl(const char *name, void *oldp, size_t *oldlenp, void *newp,
   * size_t newlen) }
   */
  private static final MethodHandle MALLCTL_HANDLE =
      resolve(
          "mallctl",
          FunctionDescriptor.of(
              ValueLayout.JAVA_INT,
              ValueLayout.ADDRESS,
              ValueLayout.ADDRESS,
              ValueLayout.ADDRESS,
              ValueLayout.ADDRESS,
              C_SIZE_T));

  private jemalloc_h() {}

  /** Whether jemalloc is the active allocator and {@code mallctl} can be called. */
  public static boolean isAvailable() {
    return MALLCTL_HANDLE != null;
  }

  /**
   * Advances jemalloc's statistics epoch, which is what actually recomputes the cached values.
   * Callers must do this once before each batch of reads.
   *
   * @return true if the refresh succeeded
   */
  public static boolean refreshEpoch() {
    if (MALLCTL_HANDLE == null) {
      return false;
    }
    try (Arena arena = Arena.ofConfined()) {
      MemorySegment name = arena.allocateFrom("epoch");
      MemorySegment value = arena.allocate(ValueLayout.JAVA_LONG);
      value.set(ValueLayout.JAVA_LONG, 0, 1L);
      MemorySegment length = arena.allocate(C_SIZE_T);
      length.set(C_SIZE_T, 0, Long.BYTES);
      // Writing epoch is what triggers the refresh; the read-back is incidental.
      return invoke(name, value, length, value, Long.BYTES);
    } catch (Throwable t) {
      LOGGER.debug("jemalloc epoch refresh failed", t);
      return false;
    }
  }

  /**
   * Reads a name whose value is 8 bytes wide: {@code size_t}, {@code uint64_t}, or {@code ssize_t}.
   * Signed names such as {@code opt.dirty_decay_ms} come back sign-extended.
   */
  public static OptionalLong readLong(String name) {
    return read(name, Long.BYTES, (segment) -> segment.get(ValueLayout.JAVA_LONG, 0));
  }

  /** Reads a name whose value is a 4-byte {@code unsigned}, such as {@code arenas.narenas}. */
  public static OptionalLong readUnsigned(String name) {
    return read(
        name,
        Integer.BYTES,
        (segment) -> Integer.toUnsignedLong(segment.get(ValueLayout.JAVA_INT, 0)));
  }

  /** Reads a name whose value is a one-byte {@code bool}, such as {@code opt.tcache}. */
  public static Optional<Boolean> readBoolean(String name) {
    OptionalLong raw = read(name, 1, (segment) -> (long) segment.get(ValueLayout.JAVA_BYTE, 0));
    return raw.isPresent() ? Optional.of(raw.getAsLong() != 0) : Optional.empty();
  }

  /**
   * Reads a name whose value is a {@code const char *}, such as {@code version}. jemalloc returns a
   * pointer to a string it owns, so the pointer itself is what lands in the buffer.
   */
  public static Optional<String> readString(String name) {
    if (MALLCTL_HANDLE == null) {
      return Optional.empty();
    }
    try (Arena arena = Arena.ofConfined()) {
      MemorySegment out = arena.allocate(ValueLayout.ADDRESS);
      if (!readInto(arena, name, out, ValueLayout.ADDRESS.byteSize())) {
        return Optional.empty();
      }
      MemorySegment pointer = out.get(ValueLayout.ADDRESS, 0);
      if (pointer.equals(MemorySegment.NULL)) {
        return Optional.empty();
      }
      return Optional.of(pointer.reinterpret(Long.MAX_VALUE).getString(0));
    } catch (Throwable t) {
      LOGGER.debug("jemalloc mallctl read of {} failed", name, t);
      return Optional.empty();
    }
  }

  private interface Decoder {
    long decode(MemorySegment segment);
  }

  private static OptionalLong read(String name, long byteSize, Decoder decoder) {
    if (MALLCTL_HANDLE == null) {
      return OptionalLong.empty();
    }
    try (Arena arena = Arena.ofConfined()) {
      MemorySegment out = arena.allocate(byteSize);
      if (!readInto(arena, name, out, byteSize)) {
        return OptionalLong.empty();
      }
      return OptionalLong.of(decoder.decode(out));
    } catch (Throwable t) {
      LOGGER.debug("jemalloc mallctl read of {} failed", name, t);
      return OptionalLong.empty();
    }
  }

  private static boolean readInto(Arena arena, String name, MemorySegment out, long byteSize)
      throws Throwable {
    MemorySegment namePtr = arena.allocateFrom(name);
    MemorySegment length = arena.allocate(C_SIZE_T);
    length.set(C_SIZE_T, 0, byteSize);
    return invoke(namePtr, out, length, MemorySegment.NULL, 0L);
  }

  private static boolean invoke(
      MemorySegment name,
      MemorySegment oldp,
      MemorySegment oldlenp,
      MemorySegment newp,
      long newlen)
      throws Throwable {
    // A non-zero return is an errno; EINVAL here means the width did not match jemalloc's type
    // for that name. Treated as "not readable" rather than an error, so a version that renames or
    // retypes a statistic degrades to omitting it.
    int rc = (int) MALLCTL_HANDLE.invokeExact(name, oldp, oldlenp, newp, newlen);
    return rc == 0;
  }

  private static ValueLayout.OfLong resolveSizeT() {
    try {
      Object layout = Linker.nativeLinker().canonicalLayouts().get("size_t");
      if (layout instanceof ValueLayout.OfLong sizeT) {
        return sizeT;
      }
    } catch (Throwable t) {
      LOGGER.debug("Could not resolve the canonical size_t layout; assuming 64-bit", t);
    }
    return ValueLayout.JAVA_LONG;
  }

  /** Resolves a symbol to a downcall handle, or null if it is absent. Never throws. */
  private static MethodHandle resolve(String symbol, FunctionDescriptor descriptor) {
    try {
      Linker linker = Linker.nativeLinker();
      Optional<MemorySegment> address =
          SymbolLookup.loaderLookup().or(linker.defaultLookup()).find(symbol);
      if (address.isEmpty()) {
        address = findInMappedJemalloc(symbol);
      }
      // mallctl is declared weak, so it can resolve to a null address rather than be absent.
      if (address.isEmpty() || address.get().equals(MemorySegment.NULL)) {
        return null;
      }
      return linker.downcallHandle(address.get(), descriptor);
    } catch (Throwable t) {
      LOGGER.debug("Symbol {} could not be resolved; jemalloc stats are disabled", symbol, t);
      return null;
    }
  }

  /**
   * Looks up a symbol in a jemalloc that the dynamic loader has already mapped into this process.
   *
   * <p>Needed because a JVM cannot be relinked, so jemalloc is supplied with {@code LD_PRELOAD} —
   * and {@link Linker#defaultLookup} does not find preloaded symbols. It searches a fixed set of
   * libraries rather than the global scope, so {@code mallctl} is invisible to it even when
   * jemalloc is unquestionably serving every allocation in the process.
   *
   * <p>Deliberately restricted to libraries already present in {@code /proc/self/maps} instead of
   * loading one by name. Loading a jemalloc that is not the active allocator would succeed and then
   * report the statistics of an idle second instance — near-zero values that look like a service
   * using no memory. Only a jemalloc the loader placed here is actually serving allocations.
   */
  private static Optional<MemorySegment> findInMappedJemalloc(String symbol) {
    for (Path library : mappedJemallocLibraries()) {
      try {
        Optional<MemorySegment> address =
            SymbolLookup.libraryLookup(library, Arena.global()).find(symbol);
        if (address.isPresent()) {
          LOGGER.debug("Resolved {} from preloaded {}", symbol, library);
          return address;
        }
      } catch (IllegalArgumentException ignored) {
        // Not a loadable library; try the next mapping.
      }
    }
    return Optional.empty();
  }

  private static List<Path> mappedJemallocLibraries() {
    Path maps = Path.of("/proc/self/maps");
    if (!Files.isReadable(maps)) {
      return List.of();
    }
    try (Stream<String> lines = Files.lines(maps)) {
      return lines
          .map(line -> line.substring(line.lastIndexOf(' ') + 1))
          .filter(path -> path.startsWith("/") && path.contains("libjemalloc"))
          .distinct()
          .map(Path::of)
          .collect(Collectors.toList());
    } catch (Throwable t) {
      LOGGER.debug("Could not scan process mappings for jemalloc", t);
      return List.of();
    }
  }
}
