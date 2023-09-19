/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Synchronized.h>
#include <atomic>
#include <memory>
#include <unordered_map>
#include "watchman/thirdparty/jansson/jansson.h"

namespace watchman {

class Root;

extern std::atomic<long> live_roots;

extern folly::Synchronized<std::unordered_map<w_string, std::shared_ptr<Root>>>
    watched_roots;

// Given a filename, walk the current set of watches.
// If a watch is a prefix match for filename then we consider it to
// be an enclosing watch and we'll return the root path and the relative
// path to filename.
// Returns NULL if there were no matches.
// If multiple watches have the same prefix, it is undefined which one will
// match.
bool findEnclosingRoot(
    const w_string& fileName,
    w_string_piece& prefix,
    w_string_piece& relativePath);

void w_root_free_watched_roots();
json_ref w_root_stop_watch_all();
json_ref w_root_watch_list_to_json();

} // namespace watchman
