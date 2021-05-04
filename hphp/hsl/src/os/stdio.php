<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the root directory of this source tree.
 *
 */

namespace HH\Lib\OS;

use namespace HH\Lib\_Private\_OS;

/** Get a file descriptor for request STDIN.
 *
 * Fails with EBADF if a request-specific file descriptor is not available, for
 * example, when running in HTTP server mode.
 */
function stdin(): FileDescriptor {
  return _OS\wrap_impl(() ==> _OS\request_stdio_fd(_OS\STDIN_FILENO));
}

/** Get a file descriptor for request STDOUT.
 *
 * Fails with EBADF if a request-specific file descriptor is not available, for
 * example, when running in HTTP server mode.
 */
function stdout(): FileDescriptor {
  return _OS\wrap_impl(() ==> _OS\request_stdio_fd(_OS\STDOUT_FILENO));
}

/** Get a file descriptor for request STDERR.
 *
 * Fails with EBADF if a request-specific file descriptor is not available, for
 * example, when running in HTTP server mode.
 */

function stderr(): FileDescriptor {
  return _OS\wrap_impl(() ==> _OS\request_stdio_fd(_OS\STDERR_FILENO));
}
