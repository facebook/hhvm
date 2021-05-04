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

/** Listen for new connections to a socket.
 *
 * See `man 2 listen` for details.
 *
 * @see `socket()`
 * @see `bind()`
 * @see `accept()`
 * @see `connect()`
 */
function listen(FileDescriptor $fd, int $backlog): void {
  _OS\wrap_impl(() ==> _OS\listen($fd, $backlog));
}
