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

final class Server implements Network\Server<CloseableSocket> {
  /** Path */
  const type TAddress = string;

  private function __construct(private OS\FileDescriptor $impl) {
  }

  /** Create a bound and listening instance */
  public static async function createAsync(string $path): Awaitable<this> {
    return await _Network\socket_create_bind_listen_async(
      OS\SocketDomain::PF_UNIX,
      OS\SocketType::SOCK_STREAM,
      /* proto = */ 0,
      new OS\sockaddr_un($path),
      /* backlog = */ 16,
      /* socket options = */ shape(),
    )
      |> new self($$);
  }

  public async function nextConnectionAsync(): Awaitable<CloseableSocket> {
    return await _Network\socket_accept_async($this->impl)
      |> new _Unix\CloseableSocket($$);
  }

  public function getLocalAddress(): string {
    return (OS\getsockname($this->impl) as OS\sockaddr_un)->getPath()
      as nonnull;
  }

  public function stopListening(): void {
    OS\close($this->impl);
  }
}
