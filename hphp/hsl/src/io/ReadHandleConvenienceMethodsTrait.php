<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the hphp/hsl/ subdirectory of this source tree.
 *
 */

namespace HH\Lib\IO;

use namespace HH\Lib\{Math, Str, OS};
use namespace HH\Lib\_Private\{_IO, _OS};

/** Trait implementing `ReadHandle` methods that can be implemented in terms
 * of more basic methods.
 */
trait ReadHandleConvenienceMethodsTrait {
  require implements ReadHandle;

  public async function readAllAsync(
    ?int $max_bytes = null,
    ?int $timeout_ns = null,
  ): Awaitable<string> {
    _OS\arg_assert(
      $max_bytes is null || $max_bytes > 0,
      'Max bytes must be null, or > 0',
    );
    _OS\arg_assert(
      $timeout_ns is null || $timeout_ns > 0,
      'Timeout must be null, or > 0',
    );

    $to_read = $max_bytes ?? Math\INT64_MAX;

    $data = '';
    $timer = new \HH\Lib\_Private\OptionalIncrementalTimeout(
      $timeout_ns,
      () ==> {
        _OS\throw_errno(
          OS\Errno::ETIMEDOUT,
          "Reached timeout before %s data could be read.",
          $data === '' ? 'any' : 'all',
        );
      },
    );

    do {
      $chunk_size = Math\minva($to_read, _IO\DEFAULT_READ_BUFFER_SIZE);
      /* @lint-ignore AWAIT_IN_LOOP */
      $chunk = await $this->readAllowPartialSuccessAsync(
        $chunk_size,
        $timer->getRemainingNS(),
      );
      $data .= $chunk;
      $to_read -= Str\length($chunk);
    } while ($to_read > 0 && $chunk !== '');
    return $data;
  }

  public async function readFixedSizeAsync(
    int $size,
    ?int $timeout_ns = null,
  ): Awaitable<string> {
    $data = await $this->readAllAsync($size);
    if (Str\length($data) !== $size) {
      _OS\throw_errno(
        OS\Errno::EPIPE,
        "%d bytes were requested, but only able to read %d bytes",
        $size,
        Str\length($data),
      );
    }
    return $data;
  }

}
