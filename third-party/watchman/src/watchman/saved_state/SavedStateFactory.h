/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "watchman/WatchmanConfig.h"
#include "watchman/thirdparty/jansson/jansson.h"
#include "watchman/watchman_string.h"

namespace watchman {

struct RootMetadata;
class SCM;
class SavedStateInterface;

/**
 * Returns an appropriate SavedStateInterface implementation for the
 * specified storage type. Returns a managed pointer to the saved state
 * interface if successful. Throws if the storage type is not recognized, or
 * if the saved state interface does not successfully parse the saved state
 * config.
 */
std::unique_ptr<SavedStateInterface> getInterface(
    w_string_piece storageType,
    const json_ref& savedStateConfig,
    const SCM* scm,
    Configuration config,
    std::function<void(RootMetadata&)> collectRootMetadata);

} // namespace watchman
