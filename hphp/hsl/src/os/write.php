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

/** Write from the specified `FileDescriptor`.
 *
 * See `man 2 write` for details. On error, an `ErrnoException` will be thrown.
 *
 * @returns the number of bytes written; it is possible for this function to
 *   succeed with a partial write.
 */
function write(FileDescriptor $fd, string $data): int {
  return _OS\wrap_impl(() ==> _OS\write($fd, $data));
}
