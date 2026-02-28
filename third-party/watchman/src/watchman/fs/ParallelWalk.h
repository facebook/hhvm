/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/ExceptionWrapper.h>
#include <folly/FBString.h>
#include <optional>
#include <vector>
#include "watchman/fs/FileInformation.h"
#include "watchman/fs/FileSystem.h"

/** Parallel filesystem walker. Collect path names and stats recursively. */

namespace watchman {

// Consider eden/common/utils/PathFuncs.h, if builds.
using PathComponent = folly::fbstring;
using AbsolutePath = folly::fbstring;

/** Similar to `DirEntry` but always contains stat and owns the strings. */
struct DirEntryOwned {
  PathComponent name;
  FileInformation stat;
};

/** ReadDir result: names and stats of direct children of a directory. */
struct ReadDirResult {
  // Full directory path.
  AbsolutePath dirFullPath;

  // Entries and their stats.
  std::vector<DirEntryOwned> entries;

  // Count of subdir entries. Useful for pre-allocation.
  size_t subdirCount = 0;
};

/** Error message with a path associated */
struct IoErrorWithPath {
  // Full path causing the error. Could be a file or a directory.
  AbsolutePath fullPath;

  // Actual error.
  folly::exception_wrapper error;

  // Name of the operation. Useful for error messages.
  const char* operationName;
};

struct ParallelWalkerContext;

class ParallelWalker final {
 public:
  /**
   * Start reading rootPath recursively. Does not block.
   *
   * threadCountHint specifies the desired thread count to initialize the
   * global thread pool. It is ignored if the thread pool was initialized.
   * threadCountHint can be 0, which means the hardware concurrency.
   * threadCountHint caps at hardware concurrency.
   *
   * Use nextResult() to obtain ReadDirResults.
   * Use nextError() to obtain IoErrorWithPaths.
   */
  explicit ParallelWalker(
      std::shared_ptr<FileSystem> fileSystem,
      AbsolutePath rootPath,
      std::optional<FileInformation> rootStat,
      size_t threadCountHint = 0);

  /**
   * Obtain the next ReadDirResult. Might block.
   *
   * Parent directory is guarnateed to be provided before child directories.
   * After completion, always return nullopt without blocking.
   */
  std::optional<ReadDirResult> nextResult();

  /**
   * Obtain an occured error. Might block.
   *
   * After completion, always return nullopt without blocking.
   */
  std::optional<IoErrorWithPath> nextError();

  /**
   * Discard the walker. Does not block.
   *
   * Existing tasks will still run to completion but new tasks will exit
   * immediately without spawning tasks for subdirectories.
   * context_ will be dropped after completeing all tasks.
   */
  ~ParallelWalker();

  ParallelWalker() = delete;
  ParallelWalker(ParallelWalker&&) = delete;
  ParallelWalker(const ParallelWalker&) = delete;
  ParallelWalker& operator=(ParallelWalker&&) = delete;
  ParallelWalker& operator=(const ParallelWalker&) = delete;

 private:
  std::shared_ptr<ParallelWalkerContext> context_;
};

} // namespace watchman
