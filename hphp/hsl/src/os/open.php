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

// Not using an enum (for now?) as it's a bit mask
const int O_RDONLY = _OS\O_RDONLY;
const int O_WRONLY = _OS\O_WRONLY;
const int O_RDWR = _OS\O_RDWR;

const int O_NONBLOCK = _OS\O_NONBLOCK;
const int O_APPEND = _OS\O_APPEND;
const int O_CREAT = _OS\O_CREAT;
const int O_TRUNC = _OS\O_TRUNC;
const int O_EXCL = _OS\O_EXCL;
const int O_NOFOLLOW = _OS\O_NOFOLLOW;
const int O_CLOEXEC = _OS\O_CLOEXEC;

/** Open the specified path.
 *
 * See `man 2 open` for details. On error, an `ErrnoException` will be thrown.
 *
 * @param $flags a bitmask of `O_` flags; one out of `O_RDONLY`, `O_WRONLY`,
 *    and `O_RDWR` **must** be specified. `O_CLOEXEC` is implicit, so that
 *    standalone CLI mode is consistent with server modes. If needed, this can
 *    be removed with `OS\fcntl()`.
 * @param $mode specify the mode of the file to create if `O_CREAT` is specified
 *    and the file does not exist.
 */
function open(string $path, int $flags, ?int $mode = null): FileDescriptor {
  if (
    ($flags & O_RDONLY) !== O_RDONLY &&
    ($flags & O_WRONLY) !== O_WRONLY &&
    ($flags & O_RDWR) !== O_RDWR
  ) {
    _OS\throw_errno(
      Errno::EINVAL,
      'O_RDONLY, O_WRONLY, or O_RDWR must be specified',
    );
  }
  if ($mode !== null && ($flags & O_CREAT) !== O_CREAT) {
    _OS\throw_errno(
      Errno::EINVAL,
      'mode should only be specified in combination with O_CREAT',
    );
  }
  if (($flags & O_CREAT) && $mode === null) {
    _OS\throw_errno(
      Errno::EINVAL,
      'mode must be specified when O_CREAT is specified',
    );
  }
  $flags |= _OS\O_CLOEXEC;
  return _OS\wrap_impl(() ==> _OS\open($path, $flags, $mode ?? 0));
}
