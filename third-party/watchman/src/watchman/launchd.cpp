/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/String.h>
#include "watchman/Logging.h"
#include "watchman/fs/FileDescriptor.h"

#ifdef __APPLE__
#include <launch.h> // @manual

using watchman::FileDescriptor;
using namespace watchman;

/* When running under launchd, we prefer to obtain our listening
 * socket from it.  We don't strictly need to run this way, but if we didn't,
 * when the user runs `watchman shutdown-server` the launchd job is left in
 * a waiting state and needs to be explicitly triggered to get it working
 * again.
 * By having the socket registered in our job description, launchd knows
 * that we want to be activated in this way and takes care of it for us.
 *
 * This is made more fun because Yosemite introduces launch_activate_socket()
 * as a shortcut for this flow and deprecated pretty much everything else
 * in launch.h.  We use the deprecated functions so that we can run on
 * older releases.
 * */
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
FileDescriptor w_get_listener_socket_from_launchd() {
  launch_data_t req, resp, socks;

  req = launch_data_new_string(LAUNCH_KEY_CHECKIN);
  if (req == NULL) {
    logf(ERR, "unable to create LAUNCH_KEY_CHECKIN\n");
    return FileDescriptor();
  }

  resp = launch_msg(req);
  launch_data_free(req);

  if (resp == NULL) {
    logf(ERR, "launchd checkin failed {}\n", folly::errnoStr(errno));
    return FileDescriptor();
  }

  if (launch_data_get_type(resp) == LAUNCH_DATA_ERRNO) {
    logf(
        ERR,
        "launchd checkin failed: {}\n",
        folly::errnoStr(launch_data_get_errno(resp)));
    launch_data_free(resp);
    return FileDescriptor();
  }

  socks = launch_data_dict_lookup(resp, LAUNCH_JOBKEY_SOCKETS);
  if (socks == NULL) {
    logf(ERR, "launchd didn't provide any sockets\n");
    launch_data_free(resp);
    return FileDescriptor();
  }

  // the "sock" name here is coupled with the plist in main.c
  socks = launch_data_dict_lookup(socks, "sock");
  if (socks == NULL) {
    logf(ERR, "launchd: \"sock\" wasn't present in Sockets\n");
    launch_data_free(resp);
    return FileDescriptor();
  }

  return FileDescriptor(
      launch_data_get_fd(launch_data_array_get_index(socks, 0)),
      FileDescriptor::FDType::Unknown);
}
#endif

/* vim:ts=2:sw=2:et:
 */
