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

import static org.assertj.core.api.Assertions.assertThat;

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.List;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.io.TempDir;

class NativeLibraryLoaderTest {

  private static final boolean IS_MAC =
      System.getProperty("os.name", "").toLowerCase().contains("mac");
  private static final String REQUIRED_SYMBOL = "ZSTD_versionString";
  private static final String[] EXACT_FILE_NAMES =
      IS_MAC
          ? new String[] {"libzstd.dylib", "libzstd.1.dylib"}
          : new String[] {"libzstd.so", "libzstd.so.1"};
  private static final String NON_CANONICAL_FILE_NAME =
      IS_MAC ? "libzstd-jni.dylib" : "libzstd-jni.so";

  @TempDir Path tempDir;

  @Test
  void prefersExactCanonicalName() throws IOException {
    Path realLibrary = resolveRealLibrary();
    Path exactMatch = Files.createSymbolicLink(tempDir.resolve(EXACT_FILE_NAMES[0]), realLibrary);
    Files.createSymbolicLink(tempDir.resolve(NON_CANONICAL_FILE_NAME), realLibrary);

    NativeLibraryLoader.LoadResult result =
        NativeLibraryLoader.findLibrary(
            List.of(new NativeLibraryLoader.SearchDirectory(tempDir, "temp")),
            REQUIRED_SYMBOL,
            EXACT_FILE_NAMES);

    assertThat(result).isNotNull();
    assertThat(result.path()).isEqualTo(exactMatch.toString());
    assertThat(result.source()).isEqualTo("temp");
  }

  @Test
  void skipsWrongExactCandidate() throws IOException {
    Path realLibrary = resolveRealLibrary();
    Path wrongExact =
        Files.createSymbolicLink(
            tempDir.resolve(EXACT_FILE_NAMES[0]),
            Path.of(ProcessHandle.current().info().command().orElseThrow()));
    Path fallbackExact =
        Files.createSymbolicLink(tempDir.resolve(EXACT_FILE_NAMES[1]), realLibrary);

    NativeLibraryLoader.LoadResult result =
        NativeLibraryLoader.findLibrary(
            List.of(new NativeLibraryLoader.SearchDirectory(tempDir, "temp")),
            REQUIRED_SYMBOL,
            EXACT_FILE_NAMES);

    assertThat(result).isNotNull();
    assertThat(result.path()).isNotEqualTo(wrongExact.toString());
    assertThat(result.path()).isEqualTo(fallbackExact.toString());
  }

  private static Path resolveRealLibrary() {
    NativeLibraryLoader.LoadResult result =
        NativeLibraryLoader.findLibrary(REQUIRED_SYMBOL, EXACT_FILE_NAMES);
    if (result == null) {
      result = NativeLibraryLoader.findLibraryByName(REQUIRED_SYMBOL, "zstd");
    }
    assertThat(result).isNotNull();
    return Path.of(result.path());
  }
}
