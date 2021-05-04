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

/** Address of a socket.
 *
 * @see `sockaddr_un` for UNIX domain sockets.
 * @see `sockaddr_in` for INET (IPv4) sockets.
 * @see `sockaddr_in6` for INET6 (IPv6) sockets.
 */
abstract class sockaddr {
  /** Get the address family of the socket.
   *
   * It may be more useful to check the type of the sockaddr object instead,
   * e.g.
   *
   * ```
   * - if ($sa->getFamily() === OS\AddressFamily::AF_UNIX) {
   * + if ($sa is OS\sockaddr_un) {
   * ```
   */
  abstract public function getFamily(): AddressFamily;
}
