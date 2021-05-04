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

/** Get address of the connected peer.
 *
 * See `man 2 getpeername` for details.
 *
 * @see getsockname
 * @see sockaddr_in
 * @see sockaddr_in6
 * @see sockaddr_un
 */
function getpeername(FileDescriptor $fd): sockaddr {
  $sa = _OS\wrap_impl(() ==> _OS\getpeername($fd));
  return _OS\sockaddr_from_native_sockaddr($sa);
}
