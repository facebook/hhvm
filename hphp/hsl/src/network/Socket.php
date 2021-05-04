<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the root directory of this source tree.
 *
 */

namespace HH\Lib\Network;

use namespace HH\Lib\{IO, TCP, Unix};

/** A handle representing a connection between processes.
 *
 * It is possible for both ends to be connected to the same process,
 * and to either be local or across a network.
 *
 * @see `TCP\Socket`
 * @see `Unix\Socket`
 */
<<__Sealed(
  CloseableSocket::class,
  TCP\Socket::class,
  Unix\Socket::class,
)>>
interface Socket extends IO\ReadWriteHandle {
  /** A local or peer address.
   *
   * For IP-based sockets, this is likely to be a host and port;
   * for Unix sockets, it is likely to be a filesystem path.
   */
  abstract const type TAddress;

  /** Returns the address of the local side of the socket */
  public function getLocalAddress(): this::TAddress;
  /** Returns the address of the remote side of the socket */
  public function getPeerAddress(): this::TAddress;
}
