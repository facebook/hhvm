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

function native_sockaddr_from_sockaddr(OS\sockaddr $sa): namespace\sockaddr {
  if ($sa is OS\sockaddr_un) {
    $path = $sa->getPath();
    if ($path is null) {
      return new namespace\sockaddr_un_unnamed();
    }
    return new namespace\sockaddr_un_pathname($path);
  }

  if ($sa is OS\sockaddr_in) {
    return new namespace\sockaddr_in(
      $sa->getPort(),
      $sa->getAddress(),
    );
  }

  if ($sa is OS\sockaddr_in6) {
    return new namespace\sockaddr_in6(
      $sa->getPort(),
      $sa->getFlowInfo(),
      $sa->getAddress() as string,
      $sa->getScopeID(),
    );
  }

  throw_errno(
    OS\Errno::EAFNOSUPPORT,
    "Unhandled sockaddr class %s",
    \get_class($sa),
  );
}
