/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "watchman/Connect.h"
#include "watchman/WatchmanConfig.h"
#include "watchman/sockname.h"
#include "watchman/watchman_stream.h"

using namespace watchman;

ResultErrno<std::unique_ptr<Stream>> w_stm_connect(int timeoutms) {
  // Default to using unix domain sockets unless disabled by config
  auto use_unix_domain = Configuration().getBool("use-unix-domain", true);

  // We have to return some kind of error if use_unix_domain is false and
  // disabled_named_pipe is true. "Destination address required" seems to fit.
  int err = EDESTADDRREQ;

  if (use_unix_domain && !disable_unix_socket) {
    auto stm = w_stm_connect_unix(get_unix_sock_name().c_str(), timeoutms);
    if (stm.hasValue()) {
      return stm;
    }
    err = stm.error();
  }

#ifdef _WIN32
  if (!disable_named_pipe) {
    std::unique_ptr<Stream> stm =
        w_stm_connect_named_pipe(get_named_pipe_sock_path().c_str(), timeoutms);
    if (stm) {
      return stm;
    }
    err = errno;
  }
#endif

  return err;
}
