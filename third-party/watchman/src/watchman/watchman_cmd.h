/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <stdexcept>
#include "watchman/CommandRegistry.h"
#include "watchman/PDU.h"
#include "watchman/watchman_preprocessor.h"
#include "watchman/watchman_system.h"

namespace watchman {
class Client;
class Command;
class Root;
class UntypedResponse;
} // namespace watchman

// For commands that take the root dir as the second parameter,
// realpath's that parameter on the client side and updates the
// argument list
void w_cmd_realpath_root(watchman::Command& command);

// Try to find a project root that contains the path `resolved`. If found,
// modify `resolved` to hold the path to the root project and return true.
// Else, return false.
// root_files should be derived from a call to cfg_compute_root_files, and it
// should not be null.  cfg_compute_root_files ensures that .watchmanconfig is
// first in the returned list of files.  This is important because it is the
// definitive indicator for the location of the project root.
bool find_project_root(
    const json_ref& root_files,
    w_string_piece& resolved,
    w_string_piece& relpath);

// Resolve the root. Failure will throw a RootResolveError exception
std::shared_ptr<watchman::Root> resolveRoot(
    watchman::Client* client,
    const json_ref& args);

// Resolve the root, or if not found and the configuration permits,
// attempt to create it. throws RootResolveError on failure.
std::shared_ptr<watchman::Root> resolveOrCreateRoot(
    watchman::Client* client,
    const json_ref& args);

// Similar to resolveRoot() or resolveOrCreateRoot() but takes a root
// string instead of json_ref.
std::shared_ptr<watchman::Root> resolveRootByName(
    watchman::Client* client,
    const char* rootName,
    bool create = false);

void add_root_warnings_to_response(
    watchman::UntypedResponse& response,
    const std::shared_ptr<watchman::Root>& root);

/* vim:ts=2:sw=2:et:
 */
