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

use namespace HH\Lib\Fileystem;
use namespace HH\Lib\_Private;

/** An `IO\Handle` that is readable.
 *
 * If implementing this interface, you may wish to use
 * `ReadHandleConvenienceAccessorTrait`, which implements `readAllAsync()` and
 * `readFixedSizeAsync()` on top of `readAsync`.
 */
interface ReadHandle extends Handle {
  /** An immediate, unordered read.
   *
   * @see `readAsync`
   * @see `readAllAsync`
   * @param $max_bytes the maximum number of bytes to read
   *   - if `null`, an internal default will be used.
   *   - if 0, `EINVAL` will be raised.
   *   - up to `$max_bytes` may be allocated in a buffer; large values may lead
   *     to unnecessarily hitting the request memory limit.
   * @throws `OS\BlockingIOException` if there is no more
   *   data available to read. If you want to wait for more
   *   data, use `readAsync` instead.
   * @returns
   *   - the read data on success.
   *   - the empty string if the end of file is reached.
   */
  public function readImpl(?int $max_bytes = null): string;

  /** Read from the handle, waiting for data if necessary.
   *
   * A wrapper around `read()` that will wait for more data if there is none
   * available at present.
   *
   * @see `readAllAsync`
   * @param max_bytes the maximum number of bytes to read
   *   - if `null`, an internal default will be used.
   *   - if 0, `EINVAL` will be raised.
   *   - up to `$max_bytes` may be allocated in a buffer; large values may lead
   *     to unnecessarily hitting the request memory limit.
   * @returns
   *   - the read data on success
   *   - the empty string if the end of file is reached.
   */
  public function readAllowPartialSuccessAsync(
    ?int $max_bytes = null,
    ?int $timeout_ns = null,
  ): Awaitable<string>;

  /** Read until there is no more data to read.
   *
   * It is possible for this to never return, e.g. if called on a pipe or
   * or socket which the other end keeps open forever. Set a timeout if you
   * do not want this to happen.
   *
   * Up to `$max_bytes` may be allocated in a buffer; large values may lead to
   * unnecessarily hitting the request memory limit.
   */
  public function readAllAsync(
    ?int $max_bytes = null,
    ?int $timeout_ns = null,
  ): Awaitable<string>;

  /** Read a fixed amount of data.
   *
   * Will fail with `EPIPE` if the file is closed before that much data is
   * available.
   *
   * It is possible for this to never return, e.g. if called on a pipe or
   * or socket which the other end keeps open forever. Set a timeout if you
   * do not want this to happen.
   */
  public function readFixedSizeAsync(
    int $size,
    ?int $timeout_ns = null,
  ): Awaitable<string>;

}
