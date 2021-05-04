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

/** Convert a presentation-format (dotted) INET (IPv4)) address to network
 * format.
 *
 * See `man inet_pton`.
 *
 * @see inet_pton_inet6 for IPv6
 */
function inet_pton_inet(string $addr): in_addr {
  // FIXME: add native builtin, use that instead
  /* HH_FIXME[2049] PHP stdlib */
  /* HH_FIXME[4107] PHP stdlib */
  if (!\filter_var($addr, \FILTER_VALIDATE_IP, \FILTER_FLAG_IPV4)) {
    _OS\throw_errno(
      Errno::EINVAL,
      "'%s' does not look like an IPv4 address",
      $addr,
    );
  }
  /* HH_FIXME[2049] PHP stdlib */
  /* HH_FIXME[4107] PHP stdlib */
  return \ip2long($addr);
}

/** Convert a presentation-format (colon-separated) INET6 (IPv6) address to
 * network format.
 *
 * See `man inet_pton`.
 *
 * @see inet_pton_inet for IPv4
 */

function inet_pton_inet6(string $addr): in6_addr {
  // FIXME: add native builtin, use that instead.
  /* HH_FIXME[2049] PHP stdlib */
  /* HH_FIXME[4107] PHP stdlib */
  if (!\filter_var($addr, \FILTER_VALIDATE_IP, \FILTER_FLAG_IPV4)) {
    _OS\throw_errno(
      Errno::EINVAL,
      "'%s' does not look like an IPv6 address",
      $addr,
    );
  }
  /* HH_FIXME[2049] PHP stdlib */
  /* HH_FIXME[4107] PHP stdlib */
  return _OS\string_as_in6_addr_UNSAFE(\inet_pton($addr));
}

/** Convert a presentation-format INET/INET6 address to network format.
 *
 * See `man inet_pton`
 *
 * @see inet_pton_inet() for a better-typed version for IPv4
 * @see inet_pton_inet6() for a better-typed version for IPv6
 */
function inet_pton(AddressFamily $af, string $addr): mixed {
  switch ($af) {
    case AddressFamily::AF_INET:
      return inet_pton_inet($addr);
    case AddressFamily::AF_INET6:
      return inet_pton_inet6($addr);
    default:
      _OS\throw_errno(
        Errno::EAFNOSUPPORT,
        "Address family is not supported by inet_pton",
      );
  }
}
