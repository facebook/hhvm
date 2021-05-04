<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the root directory of this source tree.
 *
 */

namespace HH\Lib\OS {

  /** The type of the network form of an INET6 (IPv6) address.
   *
   * @see `in4_addr` for IPv4
   */
  newtype in6_addr = string;

}

namespace HH\Lib\_Private\_OS {
  function string_as_in6_addr_UNSAFE(string $in): \HH\Lib\OS\in6_addr {
    return $in;
  }
}
