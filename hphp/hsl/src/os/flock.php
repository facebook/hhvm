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
/** Exclusive lock */
const int LOCK_EX = _OS\LOCK_EX;
/** Shared lock */
const int LOCK_SH = _OS\LOCK_SH;
/** Do not block on attempt to lock; throw instead */
const int LOCK_NB = _OS\LOCK_NB;
/** Unlock. */
const int LOCK_UN = _OS\LOCK_UN;

/** Acquire or remove an advisory lock on a file descriptor.
 *
 * See `man 2 flock` for details. On error, an `ErrnoException` will be thrown.
 *
 * A shared lock can also be 'upgraded' to an exclusive lock, however this
 * operation is not guaranteed to be atomic: systems may implement this by
 * releasing the shared lock, then attempting to acquire an exclusive lock. This
 * may lead to an upgrade attempt meaning that a lock is lost entirely, without
 * a replacement, as another process may potentially acquire a lock between
 * these operations.
 *
 * @param $flags a bitmask of `LOCK_` flags; one out of `LOCK_EX`, `LOCK_SH`, or
 *    `LOCK_UN` **must** be specified.
 */
function flock(FileDescriptor $fd, int $flags): void {
  if (
    ($flags & LOCK_EX) !== LOCK_EX &&
    ($flags & LOCK_SH) !== LOCK_SH &&
    ($flags & LOCK_UN) !== LOCK_UN
  ) {
    _OS\throw_errno(
      Errno::EINVAL,
      'LOCK_EX, LOCK_SH, or LOCK_UN must be specified',
    );
  }
  _OS\wrap_impl(() ==> _OS\flock($fd, $flags));
}
