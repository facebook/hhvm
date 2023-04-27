/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>

namespace watchman {
class Root;
}

std::shared_ptr<watchman::Root> w_root_resolve(
    const char* path,
    bool auto_watch);

std::shared_ptr<watchman::Root> w_root_resolve_for_client_mode(
    const char* filename);

std::shared_ptr<watchman::Root>
root_resolve(const char* filename, bool auto_watch, bool* created);
