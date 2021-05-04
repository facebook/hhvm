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

/** Bind a socket to an address.
 *
 * See `man 2 bind` for details.
 *
 * @see `socket()`
 * @see `listen()`
 * @see `accept()`
 * @see `connect()`
 */
function bind(FileDescriptor $fd, sockaddr $sa): void {
  _OS\wrap_impl(() ==> _OS\bind($fd, _OS\native_sockaddr_from_sockaddr($sa)));
}
