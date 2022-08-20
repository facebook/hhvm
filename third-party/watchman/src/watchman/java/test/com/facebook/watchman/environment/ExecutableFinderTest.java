/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

package com.facebook.watchman.environment;

import java.io.IOException;
import java.nio.file.FileSystems;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.attribute.PosixFilePermission;
import java.util.Set;

import com.facebook.watchman.util.TemporaryPaths;

import com.google.common.base.Optional;
import com.google.common.collect.ImmutableList;
import com.google.common.collect.ImmutableMap;
import org.junit.Rule;
import org.junit.Test;

import static java.nio.charset.StandardCharsets.UTF_8;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

public class ExecutableFinderTest {
  @Rule
  public TemporaryPaths tmp = new TemporaryPaths();

  @Test
  public void testSearchPathsFileFoundReturnsPath() throws IOException {
    Path dir1 = tmp.newFolder("foo");
    Path dir2 = tmp.newFolder("bar");
    Path dir3 = tmp.newFolder("baz");
    Path file = createExecutable("bar/blech");

    assertEquals(
        Optional.of(file),
        ExecutableFinder.getOptionalExecutable(
                Paths.get("blech"),
                ImmutableList.of(dir1, dir2, dir3),
                ImmutableList.<String>of()));
  }

  @Test
  public void testSearchPathsNonExecutableFileIsIgnored() throws IOException {
    Path dir1 = tmp.newFolder("foo");
    // Note this is not executable.
    tmp.newFile("foo/blech");
    Path dir2 = tmp.newFolder("bar");
    Path dir3 = tmp.newFolder("baz");
    Path file = createExecutable("bar/blech");

    assertEquals(
        Optional.of(file),
        ExecutableFinder.getOptionalExecutable(
            Paths.get("blech"),
            ImmutableList.of(dir1, dir2, dir3),
            ImmutableList.<String>of()));
  }

  @Test
  public void testSearchPathsDirAndFileFoundReturnsFileNotDir() throws IOException {
    Path dir1 = tmp.newFolder("foo");
    // We don't want to find this folder.
    tmp.newFolder("foo", "foo");
    Path dir2 = tmp.newFolder("bar");
    Path file = createExecutable("bar/foo");

    assertEquals(
        Optional.of(file),
        ExecutableFinder.getOptionalExecutable(
            Paths.get("foo"),
            ImmutableList.of(dir1, dir2),
            ImmutableList.<String>of()));
  }

  @Test
  public void testSearchPathsMultipleFileFoundReturnsFirstPath() throws IOException {
    Path dir1 = tmp.newFolder("foo");
    Path dir2 = tmp.newFolder("bar");
    Path dir3 = tmp.newFolder("baz");
    Path file1 = createExecutable("bar/blech");
    createExecutable("baz/blech");

    assertEquals(
        Optional.of(file1),
        ExecutableFinder.getOptionalExecutable(
            Paths.get("blech"),
            ImmutableList.of(dir1, dir2, dir3),
            ImmutableList.<String>of()));
  }

  @Test
  public void testSearchPathsSymlinkToExecutableInsidePathReturnsPath() throws IOException {
    Path dir2 = tmp.newFolder("bar");
    createExecutable("bar/blech_target");
    Path file1 = dir2.resolve("blech");
    Files.createSymbolicLink(file1, Paths.get("blech_target"));

    assertEquals(
        Optional.of(file1),
        ExecutableFinder.getOptionalExecutable(
            Paths.get("blech"),
            ImmutableList.of(dir2),
            ImmutableList.<String>of()));
  }

  @Test
  public void testSearchPathsSymlinkToExecutableOutsideSearchPathReturnsPath() throws IOException {
    Path dir1 = tmp.newFolder("foo");
    Path dir2 = tmp.newFolder("bar");
    Path dir3 = tmp.newFolder("baz");
    tmp.newFolder("unsearched");
    Path binary = createExecutable("unsearched/binary");
    Path file1 = dir2.resolve("blech");
    Files.createSymbolicLink(file1, binary);

    assertEquals(
        Optional.of(file1),
        ExecutableFinder.getOptionalExecutable(
            Paths.get("blech"),
            ImmutableList.of(dir1, dir2, dir3),
            ImmutableList.<String>of()));
  }

  @Test
  public void testSearchPathsFileNotFoundReturnsAbsent() throws IOException {
    Path dir1 = tmp.newFolder("foo");
    Path dir2 = tmp.newFolder("bar");
    Path dir3 = tmp.newFolder("baz");

    assertEquals(
        Optional.<Path>absent(),
        ExecutableFinder.getOptionalExecutable(
            Paths.get("blech"),
            ImmutableList.of(dir1, dir2, dir3),
            ImmutableList.<String>of()));
  }

  @Test
  public void testSearchPathsEmptyReturnsAbsent() throws IOException {
    assertEquals(
        Optional.<Path>absent(),
        ExecutableFinder.getOptionalExecutable(
            Paths.get("blech"),
            ImmutableList.<Path>of(),
            ImmutableList.<String>of()));
  }

  @Test
  public void testSearchPathsWithIsExecutableFunctionFailure() throws IOException {
    // Path to search
    Path baz = tmp.newFolder("baz");

    // Unexecutable "executable"
    Path bar = baz.resolve("bar");
    Files.write(bar, "".getBytes(UTF_8));
    assertTrue(bar.toFile().setExecutable(false));

    assertEquals(
        Optional.<Path>absent(),
        ExecutableFinder.getOptionalExecutable(
            Paths.get("bar"),
            ImmutableList.of(baz),
            ImmutableList.<String>of()));
  }

  @Test
  public void testSearchPathsWithExtensions() throws IOException {
    Path dir = tmp.newFolder("foo");
    Path file = createExecutable("foo/bar.EXE");

    assertEquals(
        Optional.of(file),
        ExecutableFinder.getOptionalExecutable(
            Paths.get("bar"),
            ImmutableList.of(dir),
            ImmutableList.of(".BAT", ".EXE")));
  }

  @Test
  public void testSearchPathsWithExtensionsNoMatch() throws IOException {
    Path dir = tmp.newFolder("foo");
    createExecutable("foo/bar.COM");

    assertEquals(
        Optional.absent(),
        ExecutableFinder.getOptionalExecutable(
            Paths.get("bar"),
            ImmutableList.of(dir),
            ImmutableList.of(".BAT", ".EXE")));
  }

  @Test
  public void testThatADirectoryIsNotConsideredAnExecutable() throws IOException {
    Path dir = tmp.newFolder();
    Path exe = dir.resolve("exe");
    Files.createDirectories(exe);

    assertEquals(
        Optional.absent(),
        ExecutableFinder.getOptionalExecutable(
            exe.toAbsolutePath(),
            ImmutableMap.<String, String>of()));
  }

  private Path createExecutable(String executablePath) throws IOException {
    Path file = tmp.newFile(executablePath);
    makeExecutable(file);
    return file;
  }

  public static void makeExecutable(Path file) throws IOException {
    if (FileSystems.getDefault().supportedFileAttributeViews().contains("posix")) {
      Set<PosixFilePermission> permissions = Files.getPosixFilePermissions(file);

      if (permissions.contains(PosixFilePermission.OWNER_READ)) {
        permissions.add(PosixFilePermission.OWNER_EXECUTE);
      }
      if (permissions.contains(PosixFilePermission.GROUP_READ)) {
        permissions.add(PosixFilePermission.GROUP_EXECUTE);
      }
      if (permissions.contains(PosixFilePermission.OTHERS_READ)) {
        permissions.add(PosixFilePermission.OTHERS_EXECUTE);
      }

      Files.setPosixFilePermissions(file, permissions);
    } else {
      if (!file.toFile().setExecutable(/* executable */ true, /* ownerOnly */ true)) {
        throw new IOException("The file could not be made executable");
      }
    }
  }
}
