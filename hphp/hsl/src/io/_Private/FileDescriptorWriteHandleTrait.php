<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the hphp/hsl/ subdirectory of this source tree.
 *
 */

namespace HH\Lib\_Private\_IO;

use namespace HH\Lib\{IO, Math, OS, Str};
use namespace HH\Lib\_Private\_OS;

trait FileDescriptorWriteHandleTrait implements IO\WriteHandle {
  require extends FileDescriptorHandle;
  use IO\WriteHandleConvenienceMethodsTrait;

  final protected function writeImpl(string $bytes): int {
    return OS\write($this->impl, $bytes);
  }

  final public async function writeAllowPartialSuccessAsync(
    string $bytes,
    ?int $timeout_ns = null,
  ): Awaitable<int> {
    _OS\arg_assert(
      $timeout_ns is null || $timeout_ns > 0,
      '$timeout_ns must be null, or > 0',
    );
    $timeout_ns ??= 0;

    try {
      return $this->writeImpl($bytes);
    } catch (OS\BlockingIOException $_) {
      // We need to wait, which we do below...
    }
    await $this->selectAsync(\STREAM_AWAIT_WRITE, $timeout_ns);
    return $this->writeImpl($bytes);
  }
}
