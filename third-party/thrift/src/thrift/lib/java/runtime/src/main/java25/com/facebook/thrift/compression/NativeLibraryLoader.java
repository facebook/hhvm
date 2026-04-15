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

import java.lang.foreign.Arena;
import java.lang.foreign.FunctionDescriptor;
import java.lang.foreign.Linker;
import java.lang.foreign.MemorySegment;
import java.lang.foreign.SymbolLookup;
import java.lang.foreign.ValueLayout;
import java.lang.invoke.MethodHandle;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.List;

/**
 * Utility for loading native libraries for FFI. Searches {@code DYLD_LIBRARY_PATH} (macOS) and
 * {@code LD_LIBRARY_PATH} (Linux) directories, then common system directories.
 *
 * <p>Searches exact filenames first and only accepts a candidate if it exports the expected native
 * symbol. This avoids accidentally loading JNI wrappers whose filenames merely contain the codec
 * name.
 */
public final class NativeLibraryLoader {

  private static final boolean IS_MAC =
      System.getProperty("os.name", "").toLowerCase().contains("mac");

  private static final String[] LIBRARY_PATH_ENV_VARS =
      IS_MAC
          ? new String[] {"DYLD_LIBRARY_PATH", "LD_LIBRARY_PATH"}
          : new String[] {"LD_LIBRARY_PATH"};

  private static final String[] SYSTEM_LIB_DIRS =
      IS_MAC
          ? new String[] {"/usr/local/lib", "/opt/homebrew/lib", "/usr/lib"}
          : new String[] {
            "/usr/lib64", "/lib64", "/usr/lib", "/lib", "/usr/local/lib64", "/usr/local/lib"
          };

  private NativeLibraryLoader() {}

  record SearchDirectory(Path dir, String source) {}

  /** Result of a library load, carrying both the lookup and diagnostic info. */
  public record LoadResult(SymbolLookup lookup, String source, String path) {}

  /**
   * Finds and loads a native shared library by exact filename. Searches library path env vars and
   * system directories, and only accepts a candidate if it exports {@code requiredSymbol}.
   *
   * @param requiredSymbol symbol that must be present in the loaded library
   * @param exactFileNames exact filenames to search for, in priority order
   * @return a LoadResult if found, or null if not found
   */
  public static LoadResult findLibrary(String requiredSymbol, String... exactFileNames) {
    return findLibrary(librarySearchDirectories(), requiredSymbol, exactFileNames);
  }

  /**
   * Falls back to the platform loader's library-name resolution (e.g. loading {@code "zstd"} via
   * {@code System.mapLibraryName("zstd")}). A result is only returned if the loaded library exports
   * {@code requiredSymbol}.
   */
  public static LoadResult findLibraryByName(String requiredSymbol, String... libraryNames) {
    for (String libraryName : libraryNames) {
      try {
        SymbolLookup lookup = SymbolLookup.libraryLookup(libraryName, Arena.global());
        if (hasRequiredSymbol(lookup, requiredSymbol)) {
          return new LoadResult(lookup, "libraryLookup", libraryName);
        }
      } catch (IllegalArgumentException ignored) {
      }
    }
    return null;
  }

  static LoadResult findLibrary(
      Iterable<SearchDirectory> searchDirectories,
      String requiredSymbol,
      String... exactFileNames) {
    for (SearchDirectory searchDirectory : searchDirectories) {
      for (String exactFileName : exactFileNames) {
        LoadResult result =
            tryLoadFile(
                searchDirectory.dir().resolve(exactFileName),
                searchDirectory.source(),
                requiredSymbol);
        if (result != null) {
          return result;
        }
      }
    }
    return null;
  }

  /**
   * Queries the library version by calling a version function. Returns the version string, or
   * "unknown" if the function is not found or fails.
   */
  public static String queryVersion(SymbolLookup lookup, String versionFunctionName) {
    try {
      MethodHandle versionFn =
          Linker.nativeLinker()
              .downcallHandle(
                  lookup.find(versionFunctionName).orElseThrow(),
                  FunctionDescriptor.of(ValueLayout.ADDRESS));
      MemorySegment ptr = (MemorySegment) versionFn.invokeExact();
      return ptr.reinterpret(256).getString(0);
    } catch (Throwable t) {
      return "unknown";
    }
  }

  static boolean hasRequiredSymbol(SymbolLookup lookup, String requiredSymbol) {
    return lookup.find(requiredSymbol).isPresent();
  }

  private static List<SearchDirectory> librarySearchDirectories() {
    List<SearchDirectory> searchDirectories = new ArrayList<>();

    for (String envVar : LIBRARY_PATH_ENV_VARS) {
      String libPath = System.getenv(envVar);
      if (libPath == null || libPath.isEmpty()) {
        continue;
      }
      for (String dir : libPath.split(":")) {
        if (!dir.isEmpty()) {
          searchDirectories.add(new SearchDirectory(Path.of(dir), envVar));
        }
      }
    }

    for (String dir : SYSTEM_LIB_DIRS) {
      searchDirectories.add(new SearchDirectory(Path.of(dir), "system"));
    }

    return searchDirectories;
  }

  private static LoadResult tryLoadFile(Path libFile, String source, String requiredSymbol) {
    if (!Files.isRegularFile(libFile)) {
      return null;
    }
    try {
      SymbolLookup lookup = SymbolLookup.libraryLookup(libFile, Arena.global());
      if (!hasRequiredSymbol(lookup, requiredSymbol)) {
        return null;
      }
      return new LoadResult(lookup, source, libFile.toString());
    } catch (IllegalArgumentException ignored) {
      return null;
    }
  }
}
