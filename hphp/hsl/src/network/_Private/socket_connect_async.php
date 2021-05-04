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

/** Asynchronously connect to a socket.
 *
 * Returns a PHP Socket Error Code:
 * - 0 for success
 * - errno if > 0
 * - -(10000 + h_errno) if < 0
 */
async function socket_connect_async(
  OS\FileDescriptor $sock,
  OS\sockaddr $sa,
  int $timeout_ns,
): Awaitable<void> {
  $opts = OS\fcntl($sock, OS\FcntlOp::F_GETFL);
  OS\fcntl($sock, OS\FcntlOp::F_SETFL, ($opts as int) | OS\O_NONBLOCK);
  try {
    OS\connect($sock, $sa);
  } catch (OS\BlockingIOException $_) {
    // connect(2) documents non-blocking sockets as being ready for write
    // when complete
    try {
      $res = await _OS\poll_async($sock, \STREAM_AWAIT_WRITE, $timeout_ns);
    } catch (\Exception $e) {
      throw $e;
    }
    if ($res === \STREAM_AWAIT_CLOSED) {
      _OS\throw_errno(OS\Errno::ECONNRESET, 'connect');
    }
    if ($res === \STREAM_AWAIT_TIMEOUT) {
      _OS\throw_errno(OS\Errno::ETIMEDOUT, 'connect');
    }

    $errno = _OS\wrap_impl(
      () ==> _OS\getsockopt_int($sock, _OS\SOL_SOCKET, _OS\SO_ERROR),
    );

    if ($errno !== 0) {
      _OS\throw_errno($errno as OS\Errno, 'connect() failed');
    }
  }
}
