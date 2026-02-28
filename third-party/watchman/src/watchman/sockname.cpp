/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "watchman/Options.h"

using namespace watchman;

namespace watchman {

bool disable_unix_socket = false;
bool disable_named_pipe = false;

const char* get_sock_name_legacy() {
#ifdef _WIN32
  return flags.named_pipe_path.c_str();
#else
  return flags.unix_sock_name.c_str();
#endif
}

const std::string& get_unix_sock_name() {
  return flags.unix_sock_name;
}

const std::string& get_named_pipe_sock_path() {
  return flags.named_pipe_path;
}

} // namespace watchman
