<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the root directory of this source tree.
 *
 */

namespace HH\Lib\_Private\_OS;
use namespace HH\Lib\OS;

function sockaddr_from_native_sockaddr(namespace\sockaddr $sa): OS\sockaddr {
  if ($sa is namespace\sockaddr_un_pathname) {
    return new OS\sockaddr_un($sa->sun_path);
  }
  if ($sa is namespace\sockaddr_un_unnamed) {
    return new OS\sockaddr_un(null);
  }

  if ($sa is namespace\sockaddr_in) {
    return new OS\sockaddr_in(
      $sa->sin_port,
      $sa->sin_addr,
    );
  }

  if ($sa is namespace\sockaddr_in6) {
    return new OS\sockaddr_in6(
      $sa->sin6_port,
      $sa->sin6_flowinfo,
      string_as_in6_addr_UNSAFE($sa->sin6_addr),
      $sa->sin6_scope_id,
    );
  }

  throw_errno(
    OS\Errno::EOPNOTSUPP,
    "Unhandled builtin sockaddr class %s",
    \get_class($sa),
  );
}
