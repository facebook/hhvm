/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <string>

namespace watchman {

/** Returns the legacy socket name.
 * It is legacy because its meaning is system dependent and
 * a little confusing, but needs to be retained for backwards
 * compatibility reasons as it is exported into the environment
 * in a number of scenarios.
 * You should prefer to use get_unix_sock_name() or
 * get_named_pipe_sock_path() instead
 */
const char* get_sock_name_legacy();

/** Returns the configured unix domain socket path. */
const std::string& get_unix_sock_name();

/** Returns the configured named pipe socket path */
const std::string& get_named_pipe_sock_path();

extern bool disable_unix_socket;
extern bool disable_named_pipe;

} // namespace watchman
