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
  private static final CodecSpec ZSTD =
      new CodecSpec(
          "zstd",
          "ZSTD_versionString",
          IS_MAC
              ? new String[] {"libzstd.dylib", "libzstd.1.dylib"}
              : new String[] {"libzstd.so", "libzstd.so.1"},
          IS_MAC ? "libzstd-jni.dylib" : "libzstd-jni.so");
  private static final CodecSpec LZ4 =
      new CodecSpec(
          "lz4",
          "LZ4_versionString",
          IS_MAC
              ? new String[] {"liblz4.dylib", "liblz4.1.dylib"}
              : new String[] {"liblz4.so", "liblz4.so.1"},
          IS_MAC ? "liblz4-java.dylib" : "liblz4-java.so");

  @TempDir Path tempDir;

  @Test
  void zstdPrefersExactCanonicalName() throws IOException {
    assertPrefersExactCanonicalName(ZSTD);
  }

  @Test
  void zstdSkipsBrokenExactCandidate() throws IOException {
    assertSkipsWrongExactCandidate(ZSTD);
  }

  @Test
  void lz4PrefersExactCanonicalName() throws IOException {
    assertPrefersExactCanonicalName(LZ4);
  }

  @Test
  void lz4SkipsBrokenExactCandidate() throws IOException {
    assertSkipsWrongExactCandidate(LZ4);
  }

  private void assertPrefersExactCanonicalName(CodecSpec codec) throws IOException {
    Path realLibrary = resolveRealLibrary(codec);
    Path exactMatch =
        Files.createSymbolicLink(tempDir.resolve(codec.exactFileNames()[0]), realLibrary);
    Files.createSymbolicLink(tempDir.resolve(codec.nonCanonicalFileName()), realLibrary);

    NativeLibraryLoader.LoadResult result =
        NativeLibraryLoader.findLibrary(
            List.of(new NativeLibraryLoader.SearchDirectory(tempDir, "temp")),
            codec.requiredSymbol(),
            codec.exactFileNames());

    assertThat(result).isNotNull();
    assertThat(result.path()).isEqualTo(exactMatch.toString());
    assertThat(result.source()).isEqualTo("temp");
  }

  private void assertSkipsWrongExactCandidate(CodecSpec codec) throws IOException {
    Path realLibrary = resolveRealLibrary(codec);
    Path wrongExact =
        Files.createSymbolicLink(
            tempDir.resolve(codec.exactFileNames()[0]),
            Path.of(ProcessHandle.current().info().command().orElseThrow()));
    Path fallbackExact =
        Files.createSymbolicLink(tempDir.resolve(codec.exactFileNames()[1]), realLibrary);

    NativeLibraryLoader.LoadResult result =
        NativeLibraryLoader.findLibrary(
            List.of(new NativeLibraryLoader.SearchDirectory(tempDir, "temp")),
            codec.requiredSymbol(),
            codec.exactFileNames());

    assertThat(result).isNotNull();
    assertThat(result.path()).isNotEqualTo(wrongExact.toString());
    assertThat(result.path()).isEqualTo(fallbackExact.toString());
  }

  private static Path resolveRealLibrary(CodecSpec codec) {
    NativeLibraryLoader.LoadResult result =
        NativeLibraryLoader.findLibrary(codec.requiredSymbol(), codec.exactFileNames());
    if (result == null) {
      result = NativeLibraryLoader.findLibraryByName(codec.requiredSymbol(), codec.libraryName());
    }
    assertThat(result).isNotNull();
    return Path.of(result.path());
  }

  private record CodecSpec(
      String libraryName,
      String requiredSymbol,
      String[] exactFileNames,
      String nonCanonicalFileName) {}
}
