/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/init/Init.h>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include "watchman/fs/FileSystem.h"
#include "watchman/fs/ParallelWalk.h"

void walk(watchman::AbsolutePath path, size_t threadCountHint) {
  std::cout << path << std::endl;

  auto start_time = std::chrono::steady_clock::now();
  std::shared_ptr<watchman::FileSystem> fileSystem(
      std::shared_ptr<watchman::FileSystem>{}, &watchman::realFileSystem);
  auto walker = watchman::ParallelWalker(
      fileSystem,
      path,
      fileSystem->getFileInformation(path.c_str()),
      threadCountHint);
  size_t directory_count = 0;
  size_t path_count = 0;
  off_t size = 0;
  while (true) {
    auto result = walker.nextResult();
    if (!result.has_value()) {
      break;
    }
    directory_count += 1;
    auto& entries = result.value().entries;
    path_count += entries.size();
    for (const auto& entry : entries) {
      size += entry.stat.size;
    }
  }
  auto end_time = std::chrono::steady_clock::now();
  auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(
      end_time - start_time);
  double seconds = milliseconds.count() / 1000.;

  while (true) {
    auto maybe_error = walker.nextError();
    if (!maybe_error) {
      break;
    }
    auto error = maybe_error.value();
    std::cout << "  Error: " << error.fullPath << ": " << error.error.what()
              << std::endl;
  }

  std::cout << "  Dir#:  " << directory_count << std::endl;
  std::cout << "  Path#: " << path_count << std::endl;
  std::cout << "  Size:  " << size << std::endl;
  std::cout << "  Time:  " << seconds << " seconds" << std::endl;
}

int main(int argc, char* argv[]) {
  folly::init(&argc, &argv);

  if (argc == 1) {
    std::cerr << "Provide at least a root path to walk" << std::endl;
  } else {
    size_t thread_count_hint = 0;
    const char* env = std::getenv("PWALK_THREAD");
    if (env) {
      thread_count_hint = atoi(env);
      if (thread_count_hint) {
        std::cerr << "Using min(" << thread_count_hint << ", nproc) threads"
                  << std::endl;
      }
    }
    for (int i = 1; i < argc; ++i) {
      walk(watchman::AbsolutePath(argv[i]), thread_count_hint);
    }
  }
  return 0;
}
