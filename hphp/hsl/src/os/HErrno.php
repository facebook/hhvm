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

// hackfmt-ignore
/** OS-level host error number constants from `netdb.h`.
 *
 * These values are typically stored in a global `h_errno` variable by C APIs.
 *
 * `NO_ADDRESS` is not defined here:
 * - on Linux, it is an alias for `NO_DATA`
 * - on MacOS, it is undefined.
 */
enum HErrno: int {
  HOST_NOT_FOUND = 1;
  TRY_AGAIN      = 2;
  NO_RECOVERY    = 3;
  NO_DATA        = 4;
}
