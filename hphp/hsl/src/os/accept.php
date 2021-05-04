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

/** Accept a connection on a socket.
 *
 * See `man 2 accept` for details.
 *
 * @see `socket()`
 * @see `bind()`
 * @see `listen()`
 * @see `connect()`
 */
function accept(FileDescriptor $fd): (FileDescriptor, sockaddr) {
  list($fd, $sa) = _OS\wrap_impl(() ==> _OS\accept($fd));
  return tuple($fd, _OS\sockaddr_from_native_sockaddr($sa));
}
