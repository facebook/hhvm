<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the root directory of this source tree.
 *
 */

namespace HH\Lib\_Private\_Network;

use namespace HH\Lib\OS;
use namespace HH\Lib\_Private\_OS;
/** Accept a socket connection, waiting if necessary */
async function socket_accept_async(
  OS\FileDescriptor $server,
): Awaitable<OS\FileDescriptor> {
  try {
    list($fd, $_addr) = OS\accept($server);
    return $fd;
  } catch (OS\BlockingIOException $_) {
    // accept (3P) defines select() as indicating the FD ready for read when there's a connection
    try {
      $result = await _OS\poll_async(
        $server,
        \STREAM_AWAIT_READ, /* timeout = */
        0,
      );
    } catch (_OS\ErrnoException $e) {
      _OS\throw_errno($e->getCode() as OS\Errno, '%s', $e->getMessage());
    }
    if ($result === \STREAM_AWAIT_CLOSED) {
      _OS\throw_errno(
        OS\Errno::ECONNABORTED,
        "Server socket closed while waiting for connection",
      );
    }
    list($fd, $_addr) = OS\accept($server);
    return $fd;
  }
}
