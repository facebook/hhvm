/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "watchman/watchman_string.h"

namespace watchman {
/**
 * Convert a path to a glob pattern that matches that path literally.
 * NOTE: `/` is the only allowed separator. `\` is treated as a literal
 * character and NOT a separator, and is therefore escaped.
 */
w_string convertLiteralPathToGlob(w_string_piece literal);

/**
 * Convert a glob pattern written for `noescape: true` to a glob pattern that
 * can be used without the `noescape` flag.
 */
w_string convertNoEscapeGlobToGlob(w_string_piece noescapePattern);
} // namespace watchman
