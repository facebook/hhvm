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

use namespace HH\Lib\Str;
use namespace HH\Lib\_Private\_OS;

/** Convert an INET (IPv4) address from network format to presentation
 * (dotted) format.
 *
 * See `man inet_ntop`
 *
 * @see `inet_ntop_inet6` for an IPv6 version
 */
function inet_ntop_inet(in_addr $addr): string {
  // FIXME: add native builtin, use that instead
  // this actually takes a string and immediately converts to int
  /* HH_FIXME[2049] PHP stdlib */
  /* HH_FIXME[4107] PHP stdlib */
  return \long2ip((string)$addr);
}

/** Convert an INET6 (IPv6) address from network format to presentation
 * (colon) format.
 *
 * See `man inet_ntop`
 *
 * @see `inet_ntop_inet` for an IPv4 version
 */
function inet_ntop_inet6(in6_addr $addr): string {
  /* HH_FIXME[2049] PHP stdlib */
  /* HH_FIXME[4107] PHP stdlib */
  return \inet_ntop($addr as string);
}


/** Convert an INET or INET6 address to presentation format.
 *
 * See `man inet_ntop`
 *
 * Fails with:
 * - `EAFNOSUPPORT` if the address family is not supported
 * - `EINVAL` if the address is the wrong type for the family
 *
 * @see inet_ntop_inet for a better-typed version for IPv4
 * @see inet_ntop_inet6 for a better-typed version for IPv6
 */
function inet_ntop(AddressFamily $af, dynamic $addr): string {
  switch ($af) {
    case AddressFamily::AF_INET:
      if (!$addr is int) {
        _OS\throw_errno(
          Errno::EINVAL,
          "AF_INET address must be an int",
        );
      }
      // NetLongs are always uint32
      if ($addr < 0 || $addr >= (1 << 32)) {
        _OS\throw_errno(
          Errno::EINVAL,
          "AF_INET address must fit in a uint32",
        );
      }
      return inet_ntop_inet($addr);
    case AddressFamily::AF_INET6:
      if (
        !(
          $addr is string &&
          /* HH_FIXME[2049] PHP stdlib */
          /* HH_FIXME[4107] PHP stdlib */
          \filter_var($addr, \FILTER_VALIDATE_IP, \FILTER_FLAG_IPV6)
        )
      ) {
        _OS\throw_errno(Errno::EINVAL, "AF_INET6 address must be an in6_addr");
      }
      return inet_ntop_inet6(_OS\string_as_in6_addr_UNSAFE($addr as string));
    default:
      _OS\throw_errno(Errno::EAFNOSUPPORT, 'inet_ntop()');
  }
}
