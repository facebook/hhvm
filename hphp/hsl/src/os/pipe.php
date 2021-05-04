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

/** Create a pair of connected file descriptors.
 *
 * See `man 2 pipe` for details. On error, an `ErrnoException` will be thrown.
 *
 * `O_CLOEXEC` is implicitly set for consistent behavior between standalone CLI
 * mode and server modes. Use `OS\fcntl()` to remove if needed.
 *
 * @returns Two `FileDescriptor`s; the first is read-only, and the second is
 *   write-only. Data written to the second can be read from the first.
 */
function pipe(): (FileDescriptor, FileDescriptor) {
  list($r, $w) = _OS\wrap_impl(() ==> _OS\pipe());
  _OS\fcntl($r, _OS\F_SETFL, _OS\fcntl($r, _OS\F_GETFL) as int | _OS\O_CLOEXEC);
  _OS\fcntl($w, _OS\F_SETFL, _OS\fcntl($w, _OS\F_GETFL) as int | _OS\O_CLOEXEC);
  return tuple($r, $w);
}
