/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

package com.facebook.watchman.environment;

import java.io.IOException;
import java.nio.charset.Charset;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;

import com.google.common.base.Function;
import com.google.common.base.Optional;
import com.google.common.base.Splitter;
import com.google.common.collect.FluentIterable;
import com.google.common.collect.ImmutableCollection;
import com.google.common.collect.ImmutableMap;
import com.google.common.collect.ImmutableSet;
import com.sun.jna.Platform;

import static java.io.File.pathSeparator;

/**
 * Given the name of an executable, search a set of (possibly platform-specific) known locations for
 * that executable.
 */
public class ExecutableFinder {

  private static final ImmutableSet<String> DEFAULT_WINDOWS_EXTENSIONS =
      ImmutableSet.of(
          ".bat",
          ".cmd",
          ".com",
          ".cpl",
          ".exe",
          ".js",
          ".jse",
          ".msc",
          ".vbs",
          ".wsf",
          ".wsh");
  // Avoid using MorePaths.TO_PATH because of circular deps in this package
  private static final Function<String, Path> TO_PATH = new Function<String, Path>() {
    @Override
    public Path apply(String path) {
      return Paths.get(path);
    }
  };

  private static final Function<Path, Boolean> IS_EXECUTABLE = new Function<Path, Boolean>() {
    @Override
    public Boolean apply(Path path) {
      return ExecutableFinder.isExecutable(path);
    }
  };

  public static Path getExecutable(
      Path suggestedExecutable,
      ImmutableMap<String, String> env) {
    Optional<Path> exe = getOptionalExecutable(suggestedExecutable, env);
    if (!exe.isPresent()) {
      throw new RuntimeException(String.format(
          "Unable to locate %s on PATH, or it's not marked as being executable",
          suggestedExecutable));
    }
    return exe.get();
  }

  public static Optional<Path> getOptionalExecutable(
      Path suggestedExecutable,
      ImmutableMap<String, String> env) {
    return getOptionalExecutable(suggestedExecutable, getPaths(env), getExecutableSuffixes(env));
  }

  public static Optional<Path> getOptionalExecutable(
      Path suggestedExecutable,
      ImmutableCollection<Path> path,
      ImmutableCollection<String> fileSuffixes) {

    // Fast path out of here.
    if (isExecutable(suggestedExecutable)) {
      return Optional.of(suggestedExecutable);
    }

    Optional<Path> executable = FileFinder.getOptionalFile(
        FileFinder.combine(
            /* prefixes */ null,
            suggestedExecutable.toString(),
            ImmutableSet.copyOf(fileSuffixes)),
        path,
        IS_EXECUTABLE);

    return executable;
  }

  private static boolean isExecutable(Path exe) {
    if (!Files.exists(exe)) {
      return false;
    }

    if (Files.isSymbolicLink(exe)) {
      try {
        Path target = Files.readSymbolicLink(exe);
        return isExecutable(exe.resolveSibling(target).normalize());
      } catch (IOException e) { // NOPMD
      } catch (SecurityException e) { // NOPMD

      }
    }

    if (Files.isDirectory(exe)) {
      return false;
    }

    if (!Files.isExecutable(exe) && !Files.isSymbolicLink(exe)) {
      return false;
    }

    return true;
  }

  private static ImmutableSet<Path> getPaths(ImmutableMap<String, String> env) {
    ImmutableSet.Builder<Path> paths = ImmutableSet.builder();

    // Add the empty path so that when we iterate over it, we can check for the suffixed version of
    // a given path, be it absolute or not.
    paths.add(Paths.get(""));

    String pathEnv = env.get("PATH");
    if (pathEnv != null) {
      paths.addAll(
          FluentIterable.from(Splitter.on(pathSeparator).omitEmptyStrings().split(pathEnv))
              .transform(TO_PATH));
    }

    if (Platform.isMac()) {
      Path osXPaths = Paths.get("/etc/paths");
      if (Files.exists(osXPaths)) {
        try {
          paths.addAll(
              FluentIterable.from(Files.readAllLines(osXPaths, Charset.defaultCharset()))
                  .transform(TO_PATH));
        } catch (IOException e) {
        }
      }
    }

    return paths.build();
  }

  private static ImmutableSet<String> getExecutableSuffixes(ImmutableMap<String, String> env) {
    if (Platform.isWindows()) {
      String pathext = env.get("PATHEXT");
      if (pathext == null) {
        return DEFAULT_WINDOWS_EXTENSIONS;
      }
      return ImmutableSet.<String>builder()
          .addAll(Splitter.on(";").omitEmptyStrings().split(pathext))
          .build();
    }
    return ImmutableSet.of("");
  }


  /**
   * Constructor hidden; there is no reason to instantiate this class.
   */
  private ExecutableFinder() {}
}
