/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <stdio.h>
#include <string>
#include <vector>

namespace watchman {

struct Flags {
  int show_help = 0;
#ifndef _WIN32
  int inetd_style = 0;
#endif
  int no_site_spawner = 0;
  int show_version = 0;
  std::string named_pipe_path;
  std::string unix_sock_name;
  std::string pid_file;
  int persistent = 0;
  int dont_save_state = 0;
  std::string watchman_state_file;
  int json_input_arg = 0;
  std::string output_encoding;
  std::string server_encoding;
  int foreground = 0;
  int yes_pretty = 0;
  int no_pretty = 0;
  int no_spawn = 0;
  int no_local = 0;
  std::string test_state_dir;
};

extern Flags flags;

enum ArgType {
  OPT_NONE,
  REQ_STRING,
  REQ_INT,
};

struct OptDesc {
  /* name of long option: --optname */
  const char* optname;
  /* if non-zero, short option character */
  int shortopt;
  /* help text shown in the usage information */
  const char* helptext;
  /* whether we accept an argument */
  ArgType argtype;
  /* if an argument was provided, *val will be set to
   * point to the option value.
   * Because we only update the option if one was provided
   * by the user, you can safely pre-initialize the val
   * pointer to your choice of default.
   * */
  void* val;

  /* if argtype != OPT_NONE, this is the label used to
   * refer to the argument in the help text.  If left
   * blank, we'll use the string "ARG" as a generic
   * alternative */
  const char* arglabel;

  // Whether this option should be passed to the child
  // when spawning the daemon
  int is_daemon;
#define IS_DAEMON 1
#define NOT_DAEMON 0
};

/**
 * Populates the globals in `flags`.
 *
 * Returns daemon-specific arguments.
 */
std::vector<std::string> parseOptions(int* argcp, char*** argvp);

// The following are largely for internal use.

bool w_getopt(OptDesc* opts, int* argcp, char*** argvp, char*** daemon_argv);
[[noreturn]] void usage(struct watchman_getopt* opts, FILE* where);

} // namespace watchman
