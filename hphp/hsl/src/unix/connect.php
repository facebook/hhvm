<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the root directory of this source tree.
 *
 */

namespace HH\Lib\Unix;

use namespace HH\Lib\{Network, OS};
use namespace HH\Lib\_Private\{_Network, _Unix};

/** Asynchronously connect to the specified unix socket. */
async function connect_async(
  string $path,
  ConnectOptions $opts = shape(),
): Awaitable<CloseableSocket> {
  $timeout_ns = $opts['timeout_ns'] ?? 0;
  $sock = OS\socket(OS\SocketDomain::PF_UNIX, OS\SocketType::SOCK_STREAM, 0);
  await _Network\socket_connect_async(
    $sock,
    new OS\sockaddr_un($path),
    $timeout_ns,
  );
  return new _Unix\CloseableSocket($sock);
}

<<__Deprecated('use connect_async() instead')>>
async function connect_nd_async(
  string $path,
  ConnectOptions $opts,
): Awaitable<CloseableSocket> {
  return await connect_async($path, $opts);

}
