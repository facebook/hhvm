/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

package com.facebook.watchman.environment;

import javax.annotation.Nullable;

import java.nio.file.Files;
import java.nio.file.Path;
import java.util.Set;

import com.google.common.base.Function;
import com.google.common.base.Optional;
import com.google.common.collect.ImmutableSet;

/**
 * Methods for finding files.
 */
public class FileFinder {
  /**
   * Filter that checks that a file exists.
   */
  public static final Function<Path, Boolean> EXISTS = new Function<Path, Boolean>() {
    @Override
    public Boolean apply(Path path) {
      return Files.exists(path);
    }
  };

  /**
   * Filter that tests if a file is executable.
   */
  public static final Function<Path, Boolean> IS_EXECUTABLE = new Function<Path, Boolean>() {
    @Override
    public Boolean apply(Path path) {
      return Files.isExecutable(path);
    }
  };

  /**
   * Filter that tests if a file is a regular file.
   */
  public static final Function<Path, Boolean> IS_REGULAR_FILE = new Function<Path, Boolean>() {
    @Override
    public Boolean apply(Path path) {
      return Files.isRegularFile(path);
    }
  };

  /**
   * Combines prefixes, base, and suffixes to create a set of file names.
   * @param prefixes set of prefixes. May be null or empty.
   * @param base base name. May be empty.
   * @param suffixes set of suffixes. May be null or empty.
   * @return a set containing all combinations of prefix, base, and suffix.
   */
  public static ImmutableSet<String> combine(
      @Nullable Set<String> prefixes,
      String base,
      @Nullable Set<String> suffixes) {

    ImmutableSet<String> suffixedSet;
    if (suffixes == null || suffixes.isEmpty()) {
      suffixedSet = ImmutableSet.of(base);
    } else {
      ImmutableSet.Builder<String> suffixedBuilder = ImmutableSet.builder();
      for (String suffix : suffixes) {
        suffixedBuilder.add(base + suffix);
      }
      suffixedSet = suffixedBuilder.build();
    }

    if (prefixes == null || prefixes.isEmpty()) {
      return suffixedSet;
    } else {
      ImmutableSet.Builder<String> builder = ImmutableSet.builder();
      for (String prefix : prefixes) {
        for (String suffix : suffixedSet) {
          builder.add(prefix + suffix);
        }
      }
      return builder.build();
    }
  }

  /**
   * Tries to find a file with a specific name in a search path.
   * @param name file name to look for.
   * @param searchPath directories to search.
   * @return if found: the path to the file. if not found, Optional.absent().
   */
  public static Optional<Path> getOptionalFile(
      String name,
      Iterable<Path> searchPath) {
    return getOptionalFile(name, searchPath, EXISTS);
  }

  /**
   * Tries to find a file with a specific name in a search path.
   * @param name file name to look for.
   * @param searchPath directories to search.
   * @param filter additional check that discovered paths must pass to be eligible.
   * @return if found: the path to the file. if not found, Optional.absent().
   */
  public static Optional<Path> getOptionalFile(
      String name,
      Iterable<Path> searchPath,
      Function<Path, Boolean> filter) {
    return getOptionalFile(
        ImmutableSet.of(name),
        searchPath,
        filter);
  }

  /**
   * Tries to find a file with one of a number of possible names in a search path.
   * @param possibleNames file names to look for.
   * @param searchPath directories to search.
   * @return if found: the path to the file. if not found, Optional.absent().
   */
  public static Optional<Path> getOptionalFile(
      Set<String> possibleNames,
      Iterable<Path> searchPath) {
    return getOptionalFile(possibleNames, searchPath, EXISTS);
  }

  /**
   * Tries to find a file with one of a number of possible names in a search path.
   * @param possibleNames file names to look for.
   * @param searchPath directories to search.
   * @param filter additional check that discovered paths must pass to be eligible.
   * @return if found: the path to the file. if not found, Optional.absent().
   */
  public static Optional<Path> getOptionalFile(
      Set<String> possibleNames,
      Iterable<Path> searchPath,
      Function<Path, Boolean> filter) {

    for (Path path : searchPath) {
      for (String filename : possibleNames) {
        Path resolved = path.resolve(filename);
        if (filter.apply(resolved)) {
          return Optional.of(resolved);
        }
      }
    }

    return Optional.absent();
  }

  /**
   * Constructor hidden; there is no reason to instantiate this class.
   */
  private FileFinder() {}
}
