<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the hphp/hsl/ subdirectory of this source tree.
 *
 */

namespace HH\Lib\File;

use namespace HH\Lib\IO;

interface Handle extends IO\SeekableFDHandle {
  /**
   * Get the name of this file.
   */
  public function getPath(): string;

  /**
   * Get the size of the file.
   */
  public function getSize(): int;

  /**
   * Get a shared or exclusive lock on the file.
   *
   * This will block until it acquires the lock, which may be forever.
   *
   * This involves a blocking syscall; async code will not execute while
   * waiting for a lock.
   */
  <<__ReturnDisposable>>
  public function lock(LockType $type): Lock;

  /**
   * Immediately get a shared or exclusive lock on a file, or throw.
   *
   * @throws `File\AlreadyLockedException` if `lock()` would block. **This
   *   is not a subclass of `OS\ErrnoException`**.
   * @throws `OS\ErrnoException` in any other case.
   */
  <<__ReturnDisposable>>
  public function tryLockx(LockType $type): Lock;
}

interface ReadHandle extends Handle, IO\SeekableReadFDHandle {
}

interface WriteHandle extends Handle, IO\SeekableWriteFDHandle {
}

interface ReadWriteHandle extends WriteHandle, ReadHandle, IO\SeekableReadWriteFDHandle {
}
