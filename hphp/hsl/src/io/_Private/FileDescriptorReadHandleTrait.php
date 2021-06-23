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

use namespace HH\Lib\{IO, Str, OS, Math};
use namespace HH\Lib\_Private\_OS;

trait FileDescriptorReadHandleTrait implements IO\ReadHandle {
  require extends FileDescriptorHandle;
  use IO\ReadHandleConvenienceMethodsTrait;

  final public function readImpl(?int $max_bytes = null): string {
    $max_bytes ??= DEFAULT_READ_BUFFER_SIZE;

    _OS\arg_assert($max_bytes > 0, '$max_bytes must be null, or > 0');
    return OS\read($this->impl, $max_bytes);
  }

  final public async function readAllowPartialSuccessAsync(
    ?int $max_bytes = null,
    ?int $timeout_ns = null,
  ): Awaitable<string> {
    $max_bytes ??= DEFAULT_READ_BUFFER_SIZE;

    _OS\arg_assert($max_bytes > 0, '$max_bytes must be null, or > 0');
    _OS\arg_assert(
      $timeout_ns is null || $timeout_ns > 0,
      '$timeout_ns must be null, or > 0',
    );
    $timeout_ns ??= 0;

    try {
      return $this->readImpl($max_bytes);
    } catch (OS\BlockingIOException $_) {
      // this means we need to wait for data, which we do below...
    }

    await $this->selectAsync(\STREAM_AWAIT_READ, $timeout_ns);
    return $this->readImpl($max_bytes);
  }
}
