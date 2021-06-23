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

use namespace HH\Lib\{IO, OS};
use namespace HH\Lib\_Private\_OS;

final class RequestReadHandle implements IO\ReadHandle {
  use IO\ReadHandleConvenienceMethodsTrait;

  public function readImpl(?int $max_bytes = null): string {
    $max_bytes ??= DEFAULT_READ_BUFFER_SIZE;
    _OS\arg_assert($max_bytes > 0, '$max_bytes must be null or positive');
    return namespace\request_read($max_bytes);
  }

  public async function readAllowPartialSuccessAsync(
    ?int $max_bytes = null,
    ?int $timeout_ns = null,
  ): Awaitable<string> {
    return $this->readImpl($max_bytes);
  }
}
