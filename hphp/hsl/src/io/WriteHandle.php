<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the root directory of this source tree.
 *
 */

namespace HH\Lib\IO;

use namespace HH\Lib\Fileystem;
use namespace HH\Lib\_Private;

/** An interface for a writable Handle.
 *
 * Order of operations is guaranteed, *except* for `writeImplBlocking`;
 * `writeImplBlocking()` will immediately try to write to the handle.
 */
interface WriteHandle extends Handle {
  /** An immediate unordered write.
   *
   * @see `writeAllAsync()`
   * @see `writeAllowPartialSuccessAsync()`
   * @throws `OS\BlockingIOException` if the handle is a socket or similar,
   *   and the write would block.
   * @returns the number of bytes written on success, which may be 0
   */
  protected function writeImpl(string $bytes): int;

  /** Write data, waiting if necessary.
   *
   * A wrapper around `write()` that will wait if `write()` would throw
   * an `OS\BlockingIOException`
   *
   * It is possible for the write to *partially* succeed - check the return
   * value and call again if needed.
   *
   * @returns the number of bytes written, which may be less than the length of
   *   input string.
   */
  public function writeAllowPartialSuccessAsync(
    string $bytes,
    ?int $timeout_ns = null,
  ): Awaitable<int>;

  /** Write all of the requested data.
   *
   * A wrapper aroudn `writeAsync()` that will:
   * - do multiple writes if necessary to write the entire provided buffer
   * - fail with EPIPE if it is not possible to write all the requested data
   *
   * It is possible for this to never return, e.g. if called on a pipe or
   * or socket which the other end keeps open forever. Set a timeout if you
   * do not want this to happen.
   */
  public function writeAllAsync(
    string $bytes,
    ?int $timeout_ns = null,
  ): Awaitable<void>;
}
