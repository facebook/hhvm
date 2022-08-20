/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "watchman/saved_state/SavedStateInterface.h"

namespace watchman {

// Identifies the most recent saved state for a given commit from saved states
// stored on the local filesystem. The local storage path must contain a
// subdirectory for the project, and within the project directory the saved
// state for a given commit must be in a file whose name is the source control
// commit hash. If project metadata is not specified, then only saved states
// with no project metadata will be returned. If project metadata is specified,
// then the most recent saved state with the specified project metadata will be
// returned.
//
// Checks the most recent n commits to find a saved state, if available. If a
// saved state is not available, returns an error message in the saved state
// info JSON. If a saved state is available, returns the local path for the
// state in the saved state info JSON, along with the saved state commit id.
class LocalSavedStateInterface : public SavedStateInterface {
 public:
  LocalSavedStateInterface(const json_ref& savedStateConfig, const SCM* scm);

  SavedStateInterface::SavedStateResult getMostRecentSavedStateImpl(
      w_string_piece lookupCommitId) const override;
  w_string getLocalPath(w_string_piece commitId) const;

 private:
  json_int_t maxCommits_;
  w_string localStoragePath_;
  const SCM* scm_;
};
} // namespace watchman
