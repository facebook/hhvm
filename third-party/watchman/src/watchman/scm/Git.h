/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <chrono>
#include <vector>
#include "watchman/ChildProcess.h"
#include "watchman/LRUCache.h"
#include "watchman/scm/SCM.h"

namespace watchman {

class Git : public SCM {
 public:
  Git(w_string_piece rootPath, w_string_piece scmRoot);
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
  std::vector<w_string> getCommitsPriorToAndIncluding(
      w_string_piece commitId,
      int numCommits,
      const std::optional<w_string>& requestId = std::nullopt) const override;

 private:
  std::string indexPath_;
  mutable LRUCache<std::string, std::vector<w_string>> commitsPrior_;
  mutable LRUCache<std::string, w_string> mergeBases_;
  mutable LRUCache<std::string, std::vector<w_string>>
      filesChangedSinceMergeBaseWith_;

  ChildProcess::Options makeGitOptions(
      const std::optional<w_string>& requestId) const;
  struct timespec getIndexMtime() const;
};

} // namespace watchman
