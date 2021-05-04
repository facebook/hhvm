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

/** Get the name of the local end of a socket.
 *
 * See `man 2 getsockname` for details.
 *
 * @see getpeername()
 * @see sockaddr_in
 * @see sockaddr_in6
 * @see sockaddr_un
 */
function getsockname(FileDescriptor $fd): sockaddr {
  $sa = _OS\wrap_impl(() ==> _OS\getsockname($fd));
  return _OS\sockaddr_from_native_sockaddr($sa);
}
