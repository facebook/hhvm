/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "watchman/saved_state/LocalSavedStateInterface.h"
#include "watchman/CommandRegistry.h"
#include "watchman/Errors.h"
#include "watchman/Logging.h"
#include "watchman/fs/FileInformation.h"
#include "watchman/scm/SCM.h"

static const int kDefaultMaxCommits{10};

W_CAP_REG("saved-state-local")

namespace watchman {

LocalSavedStateInterface::LocalSavedStateInterface(
    const json_ref& savedStateConfig,
    const SCM* scm)
    : SavedStateInterface(savedStateConfig), scm_(scm) {
  // Max commits to search in source control history for a saved state
  auto maxCommits = savedStateConfig.get_optional("max-commits");
  if (maxCommits) {
    if (!maxCommits->isInt()) {
      throw QueryParseError("'max-commits' must be an integer");
    }
    maxCommits_ = maxCommits->asInt();
    if (maxCommits_ < 1) {
      throw QueryParseError("'max-commits' must be a positive integer");
    }
  } else {
    maxCommits_ = kDefaultMaxCommits;
  }
  // Local path to search for saved states. This path will only ever be read,
  // never written.
  auto localStoragePath = savedStateConfig.get_optional("local-storage-path");
  if (!localStoragePath) {
    throw QueryParseError(
        "'local-storage-path' must be present in saved state config");
  }
  if (!localStoragePath->isString()) {
    throw QueryParseError("'local-storage-path' must be a string");
  }
  localStoragePath_ = json_to_w_string(*localStoragePath);
  if (!w_string_path_is_absolute(localStoragePath_)) {
    throw QueryParseError("'local-storage-path' must be an absolute path");
  }
  // The saved state project must be a sub-directory in the local storage
  // path.
  if (w_string_path_is_absolute(project_)) {
    throw QueryParseError("'project' must be a relative path");
  }
}

SavedStateInterface::SavedStateResult
LocalSavedStateInterface::getMostRecentSavedStateImpl(
    w_string_piece lookupCommitId) const {
  auto commitIds =
      scm_->getCommitsPriorToAndIncluding(lookupCommitId, maxCommits_);
  for (auto& commitId : commitIds) {
    auto path = getLocalPath(commitId);
    // We could return a path that no longer exists if the path is removed
    // (for example by saved state GC) after we check that the path exists
    // here, but before the client reads the state. We've explicitly chosen to
    // return the state without additional safety guarantees, and leave it to
    // the client to ensure GC happens only after states are no longer likely
    // to be used.
    if (w_path_exists(path.c_str())) {
      log(DBG, "Found saved state for commit ", commitId, "\n");
      SavedStateInterface::SavedStateResult result;
      result.commitId = commitId;
      result.savedStateInfo = json_object(
          {{"local-path", w_string_to_json(path)},
           {"commit-id", w_string_to_json(commitId)}});
      return result;
    }
  }
  SavedStateInterface::SavedStateResult result;
  result.commitId = w_string();
  result.savedStateInfo = json_object(
      {{"error", w_string_to_json("No suitable saved state found")}});
  return result;
}

w_string LocalSavedStateInterface::getLocalPath(w_string_piece commitId) const {
  w_string filename;
  if (!projectMetadata_) {
    filename = w_string::build(commitId);
  } else {
    filename = w_string::build(commitId, w_string("_"), *projectMetadata_);
  }
  return w_string::pathCat({localStoragePath_, project_, filename});
}
} // namespace watchman
