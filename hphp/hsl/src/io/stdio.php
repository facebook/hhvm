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

use namespace HH\Lib\OS;
use namespace HH\Lib\_Private\_IO;

/** Return the output handle for the current request.
 *
 * This should generally be used for sending data to clients. In CLI mode, this
 * is usually the process STDOUT.
 *
 * This MAY be a `CloseableWriteFDHandle`.
 *
 * @see requestOutput
 */
<<__Memoize>>
function request_output(): WriteHandle {
  try {
    return new _IO\StdioWriteHandle(OS\stdout());
  } catch (OS\ErrnoException $e) {
    if ($e->getErrno() === OS\Errno::EBADF) {
      return new _IO\ResponseWriteHandle();
    }
    throw $e;
  }
}

/** Return the error output handle for the current request.
 *
 * This is usually only available for CLI scripts; it will return null in most
 * other contexts, including HTTP requests.
 *
 * For a throwing version, use `request_errorx()`.
 *
 * In CLI mode, this is usually the process STDERR.
 */
function request_error(): ?CloseableWriteFDHandle {
  try {
    return request_errorx();
  } catch (OS\ErrnoException $e) {
    if ($e->getErrno() === OS\Errno::EBADF) {
      return null;
    }
    throw $e;
  }
}

/** Return the error output handle for the current request.
 *
 * This is usually only available for CLI scripts; it will fail with `EBADF.
 * in most other contexts, including HTTP requests.
 *
 * For a non-throwing version, use `request_error()`.
 *
 * In CLI mode, this is usually the process STDERR.
 */
function request_errorx(): CloseableWriteFDHandle {
  return new _IO\StdioWriteHandle(OS\stderr());
}

/** Return the input handle for the current request.
 *
 * In CLI mode, this is likely STDIN; for HTTP requests, it may contain the
 * POST data, if any.
 *
 * This MAY be a `CloseableReadFDHandle`.
 */
<<__Memoize>>
function request_input(): ReadHandle {
  try {
    return new _IO\StdioReadHandle(OS\stdin());
  } catch (OS\ErrnoException $e) {
    if ($e->getErrno() === OS\Errno::EBADF) {
      return new _IO\RequestReadHandle();
    }
    throw $e;
  }
}
