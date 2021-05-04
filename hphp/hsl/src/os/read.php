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

/** Read from the specified `FileDescriptor`.
 *
 * See `man 2 read` for details. On error, an `ErrnoException` will be thrown.
 */
function read(FileDescriptor $fd, int $max_bytes): string {
  return _OS\wrap_impl(() ==> _OS\read($fd, $max_bytes));
}
