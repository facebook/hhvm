/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "watchman/saved_state/SavedStateFactory.h"
#include "watchman/ClientContext.h"
#include "watchman/Errors.h"
#include "watchman/root/Root.h"
#include "watchman/saved_state/LocalSavedStateInterface.h"

#if HAVE_MANIFOLD
#include "watchman/facebook/saved_state/ManifoldSavedStateInterface.h" // @manual
#endif

namespace watchman {

std::unique_ptr<SavedStateInterface> getInterface(
    w_string_piece storageType,
    const json_ref& savedStateConfig,
    const SCM* scm,
    [[maybe_unused]] Configuration config,
    [[maybe_unused]] std::function<void(RootMetadata&)> collectRootMetadata,
    [[maybe_unused]] ClientContext clientInfo) {
#if HAVE_MANIFOLD
  if (storageType == "manifold") {
    return std::make_unique<ManifoldSavedStateInterface>(
        savedStateConfig,
        scm,
        std::move(config),
        std::move(collectRootMetadata),
        std::move(clientInfo));
  }
#endif
  if (storageType == "local") {
    return std::make_unique<LocalSavedStateInterface>(savedStateConfig, scm);
  }
  QueryParseError::throwf("invalid storage type '{}'", storageType);
}

} // namespace watchman
