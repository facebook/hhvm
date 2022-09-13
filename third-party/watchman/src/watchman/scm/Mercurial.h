/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include "watchman/watchman_system.h"

#include <string>
#include "watchman/ChildProcess.h"
#include "watchman/LRUCache.h"
#include "watchman/scm/SCM.h"

namespace watchman {

class Mercurial : public SCM {
 public:
  Mercurial(w_string_piece rootPath, w_string_piece scmRoot);
  w_string mergeBaseWith(
      w_string_piece commitId,
      const std::optional<w_string>& requestId = std::nullopt) const override;
  std::vector<w_string> getFilesChangedSinceMergeBaseWith(
      w_string_piece commitId,
      w_string_piece clock,
      const std::optional<w_string>& requestId = std::nullopt) const override;
  std::chrono::time_point<std::chrono::system_clock> getCommitDate(
      w_string_piece commitId,
      const std::optional<w_string>& requestId = std::nullopt) const override;
  // public for testing
  static std::chrono::time_point<std::chrono::system_clock> convertCommitDate(
      const char* commitDate);
  std::vector<w_string> getCommitsPriorToAndIncluding(
      w_string_piece commitId,
      int numCommits,
      const std::optional<w_string>& requestId = std::nullopt) const override;

 private:
  std::string dirStatePath_;
  mutable LRUCache<std::string, std::vector<w_string>> commitsPrior_;
  mutable LRUCache<std::string, w_string> mergeBases_;
  mutable LRUCache<std::string, std::vector<w_string>>
      filesChangedSinceMergeBaseWith_;

  // Returns options for invoking hg
  ChildProcess::Options makeHgOptions(
      const std::optional<w_string>& requestId) const;
  struct timespec getDirStateMtime() const;
};

} // namespace watchman
