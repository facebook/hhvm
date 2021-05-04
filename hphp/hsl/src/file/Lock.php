<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the root directory of this source tree.
 *
 */

namespace HH\Lib\File;

use namespace HH\Lib\{OS, _Private\_OS};

/**
 * A File Lock, which is unlocked as a disposable. To acquire one, call `lock`
 * on a Base object.
 *
 * Note that in some cases, such as the non-blocking lock types, we may throw
 * an `LockAcquisitionException` instead of acquiring the lock. If this
 * is not desired behavior it should be guarded against.
 */
final class Lock implements \IDisposable {

  public function __construct(private OS\FileDescriptor $fd) {
  }

  final public function __dispose(): void {
    try {
      OS\flock($this->fd, _OS\LOCK_UN);
    } catch (OS\ErrnoException $e) {
      if ($e->getErrno() !== OS\Errno::EBADF) {
        throw $e;
      }
    }
  }
}
