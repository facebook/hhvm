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

use namespace HH\Lib\{Network, OS};
use namespace HH\Lib\_Private\{_Network, _OS, _TCP};

/** A poor alternative to OS\getaddrinfo, which doesn't exist yet. */
function resolve_hostname(OS\AddressFamily $af, string $host): ?string {
  // FIXME: add OS\getaddrinfo, kill this function.
  switch ($af) {
    case OS\AddressFamily::AF_INET:
      // if it's already a valid IP, it just returns the input.
      /* HH_FIXME[2049] PHP stdlib */
      /* HH_FIXME[4107] PHP stdlib */
      return \gethostbyname($host);
    case OS\AddressFamily::AF_INET6:
      /* HH_FIXME[2049] PHP stdlib */
      /* HH_FIXME[4107] PHP stdlib */
      if (\filter_var($host, \FILTER_VALIDATE_IP, \FILTER_FLAG_IPV6)) {
        return $host;
      }
      $authns = null;
      $addtl = null;
      /* HH_FIXME[2049] PHP stdlib */
      /* HH_FIXME[4107] PHP stdlib */
      return \dns_get_record(
        $host,
        \DNS_AAAA,
        inout $authns,
        inout $addtl,
      )['AAAA'] ??
        null;
    default:
      _OS\throw_errno(
        OS\Errno::EAFNOSUPPORT,
        "Can only resolve hostnames to IPv4 and IPv6 addresses",
      );
  }
}
