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

/** Address of an INET (IPv4) socket.
 *
 * See `man 7 ip` (Linux) or `man 4 inet` (BSD) for details.
 *
 * @see `sockaddr_in6` for INET6 (IPv6) sockets.
 */
final class sockaddr_in extends sockaddr {
  /** Construct a `sockaddr_in`.
   *
   * Unlike the C API, all integers are in host byte order, not network byte
   * order.
   */
  public function __construct(
    private int $port,
    private in_addr $address,
  ) {
  }

  <<__Override>>
  final public function getFamily(): AddressFamily {
    return AddressFamily::AF_INET;
  }

  /** Get the port, in host byte order. */
  final public function getPort(): int{
    return $this->port;
  }

  /** Get the IP address, as a 32-bit integer, in host byte order. */
  final public function getAddress(): in_addr {
    return $this->address;
  }

  final public function __debugInfo(): darray<string, mixed> {
    return darray[
      'port (host byte order)' => $this->port,
      'address (uint32)' => $this->address,
      'address (presentation format)' =>
        inet_ntop_inet($this->address),
    ];
  }
}
