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

/** Get the name of the terminal associated with a file descriptor, if any.
 *
 * This function will throw if an error occurs; you may want to specifically
 * handle `ENOTTY`.
 */
function ttyname(FileDescriptor $fd): string {
  return _OS\wrap_impl(
    () ==> _OS\ttyname($fd),
  );
}
