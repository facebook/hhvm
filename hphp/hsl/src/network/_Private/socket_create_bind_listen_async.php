<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the hphp/hsl/ subdirectory of this source tree.
 *
 */

namespace HH\Lib\_Private\_Network;

use namespace HH\Lib\{Network, OS};
use namespace HH\Lib\_Private\_OS;

/** Create a server socket and start listening */
async function socket_create_bind_listen_async(
  OS\SocketDomain $domain,
  OS\SocketType $type,
  int $proto,
  OS\sockaddr $addr,
  int $backlog,
  Network\SocketOptions $socket_options,
): Awaitable<OS\FileDescriptor> {
  $sock = OS\socket($domain, $type, $proto);

  if ($socket_options['SO_REUSEADDR'] ?? false) {
    _OS\wrap_impl(
      () ==> _OS\setsockopt_int($sock, _OS\SOL_SOCKET, _OS\SO_REUSEADDR, 1),
    );
  }
  $ops = OS\fcntl($sock, OS\FcntlOp::F_GETFL);
  OS\fcntl($sock, OS\FcntlOp::F_SETFL, ($ops as int) | OS\O_NONBLOCK);

  try {
    OS\bind($sock, $addr);
  } catch (OS\BlockingIOException $_) {
    await _OS\poll_async($sock, \STREAM_AWAIT_READ_WRITE, /* timeout = */ 0);

    $errno = _OS\wrap_impl(
      () ==> _OS\getsockopt_int($sock, _OS\SOL_SOCKET, _OS\SO_ERROR),
    ) as OS\Errno;
    if ($errno !== 0) {
      _OS\throw_errno($errno, 'bind() failed');
    }
  }

  OS\listen($sock, $backlog);

  return $sock;
}
