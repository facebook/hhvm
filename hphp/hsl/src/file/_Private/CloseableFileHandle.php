<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the root directory of this source tree.
 *
 */

namespace HH\Lib\_Private\_File;

use namespace HH\Lib\Str;
use namespace HH\Lib\_Private\{_IO, _OS};
use namespace HH\Lib\{IO, File, OS};

<<__ConsistentConstruct>>
abstract class CloseableFileHandle
  extends _IO\FileDescriptorHandle
  implements File\Handle, IO\CloseableHandle {

  final public function __construct(
    OS\FileDescriptor $fd,
    protected string $filename,
  ) {
    parent::__construct($fd);
  }

  <<__Memoize>>
  final public function getPath(): string {
    return $this->filename;
  }

  final public function getSize(): int {
    $pos = OS\lseek($this->impl, 0, OS\SeekWhence::SEEK_CUR);
    $size = OS\lseek($this->impl, 0, OS\SeekWhence::SEEK_END);
    OS\lseek($this->impl, $pos, OS\SeekWhence::SEEK_SET);

    return $size;
  }

  final public function seek(int $offset): void {
    OS\lseek($this->impl, $offset, OS\SeekWhence::SEEK_SET);
  }

  final public function tell(): int {
    return OS\lseek($this->impl, 0, OS\SeekWhence::SEEK_CUR);
  }

  <<__ReturnDisposable>>
  final public function lock(File\LockType $type): File\Lock {
    OS\flock($this->impl, $type);
    return new File\Lock($this->impl);
  }

  <<__ReturnDisposable>>
  final public function tryLockx(File\LockType $type): File\Lock {
    try {
      OS\flock($this->impl, $type | OS\LOCK_NB);
      return new File\Lock($this->impl);
    } catch (OS\BlockingIOException $e) {
      if ($e->getErrno() === OS\Errno::EAGAIN) {
        throw new File\AlreadyLockedException();
      }
      throw $e;
    }
  }
}
