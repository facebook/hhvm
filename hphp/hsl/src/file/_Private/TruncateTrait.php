<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the hphp/hsl/ subdirectory of this source tree.
 *
 */

namespace HH\Lib\_Private\_File;

use namespace HH\Lib\{IO, OS};
use namespace HH\Lib\_Private\_IO;

/**
 * This method can't be added directly to `_IO\FileDescriptorWriteHandleTrait`,
 * because only real files can be truncated on both MacOS and Linux.
 * On Linux you can also truncate posix shared memory, but that is not
 * supported on MacOS.
 */
trait TruncateTrait {
  require extends _IO\FileDescriptorHandle;

  final public function truncate(?int $length = null): void {
    OS\ftruncate($this->impl, $length ?? 0);
  }
}
