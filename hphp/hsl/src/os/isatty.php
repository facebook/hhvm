<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the root directory of this source tree.
 *
 */

namespace HH\Lib\OS;

use namespace HH\Lib\_Private\_OS;

/** Test if a file descriptor refers to a terminal.
 *
 * If the native call fails with `ENOTTY` (for example, on MacOS), this function
 * will return false.
 *
 * If the native call fails with any other error (for example, `EBADF`), this
 * function will throw.
 */
function isatty(FileDescriptor $fd): bool {
  return _OS\wrap_impl(
    () ==> _OS\isatty($fd),
  );
}
