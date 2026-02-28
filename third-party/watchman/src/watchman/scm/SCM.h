/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <chrono>
#include <optional>
#include <vector>
#include "watchman/Errors.h"
#include "watchman/watchman_string.h"
#include "watchman/watchman_system.h"

namespace watchman {

// Walks the paths from rootPath up to the root of the filesystem.
// At each level, checks to see if any of the candidate filenames
// in the provided candidates list exist.  Returns the name of
// the first one it finds.  If no candidates are found, returns
// nullopt.
std::optional<w_string> findFileInDirTree(
    w_string_piece rootPath,
    std::initializer_list<w_string_piece> candidates);

class SCMError : public WatchmanError<SCMError> {
 public:
  static constexpr char* prefix = nullptr;
  using WatchmanError::WatchmanError;
};

class SCM {
 protected:
  // Construct an SCM instance for the specified rootPath on disk.
  // rootPath may be a child directory of the true SCM root.
  SCM(w_string_piece rootPath, w_string_piece scmRoot);

 public:
  virtual ~SCM();

  // Figure out an appropriate SCM implementation for the specified
  // rootPath.  Returns a managed pointer to it if successful.
  // Returns nullptr if rootPath doesn't appear to be tracked
  // by any source control systems known to watchman.
  // Will throw an exception if watchman encounters a problem
  // in setting up the SCM instance.
  static std::unique_ptr<SCM> scmForPath(w_string_piece rootPath);

  // Returns the root path provided during construction
  const w_string& getRootPath() const;

  // Returns the directory which is considered to be the root of
  // of the repository.  This may be a parent of the rootPath that
  // was used to construct this SCM instance.
  const w_string& getSCMRoot() const;

  // Compute the merge base between the working copy revision and the
  // specified commitId.  The commitId is typically a branch name like "main".
  virtual w_string mergeBaseWith(
      w_string_piece commitId,
      const std::optional<w_string>& requestId = std::nullopt) const = 0;

  // Compute the set of paths that have changed in the commits
  // starting in the working copy and going back to the merge base
  // with the specified commitId.  This list also includes the
  // set of files that show as modified in the "status" output,
  // but NOT those that are ignored.
  virtual std::vector<w_string> getFilesChangedSinceMergeBaseWith(
      w_string_piece commitId,
      w_string_piece clock,
      const std::optional<w_string>& requestId = std::nullopt) const = 0;

  // Compute the source control date associated with the specified
  // commit.
  virtual std::chrono::time_point<std::chrono::system_clock> getCommitDate(
      w_string_piece commitId,
      const std::optional<w_string>& requestId = std::nullopt) const = 0;

  // Compute the numCommits commits prior to and including the specified commit
  // in source control history. Returns an ordered list with the most recent
  // commit (the one specified) first.
  virtual std::vector<w_string> getCommitsPriorToAndIncluding(
      w_string_piece commitId,
      int numCommits,
      const std::optional<w_string>& requestId = std::nullopt) const = 0;

 private:
  w_string rootPath_;
  w_string scmRoot_;
};
} // namespace watchman
