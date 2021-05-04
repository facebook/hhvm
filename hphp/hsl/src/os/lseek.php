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

enum SeekWhence: int as int {
  SEEK_SET = _OS\SEEK_SET;
  SEEK_CUR = _OS\SEEK_CUR;
  SEEK_END = _OS\SEEK_END;
  SEEK_HOLE = _OS\SEEK_HOLE;
  SEEK_DATA = _OS\SEEK_DATA;
}
/** Reposition the current file offset.
 *
 * See `man 2 lseek` for details. On error, an `ErrnoException` will be thrown.
 */
function lseek(FileDescriptor $fd, int $offset, SeekWhence $whence): int {
  return _OS\wrap_impl(() ==> _OS\lseek($fd, $offset, $whence));
}
