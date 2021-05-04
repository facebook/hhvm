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

/** Close the specified `FileDescriptor`.
 *
 * See `man 2 close` for details. On error, an `ErrnoException` will be thrown.
 *
 * This function is not automatically retried on `EINTR`, as `close()` is not
 * safe to retry on `EINTR`.
 */
function close(FileDescriptor $fd): void {
  _OS\wrap_impl(() ==> _OS\close($fd));
}
