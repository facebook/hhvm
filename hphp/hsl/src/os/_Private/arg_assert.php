<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the root directory of this source tree.
 *
 */

namespace HH\Lib\_Private\_OS;

use namespace HH\Lib\{Str, OS};

/** Raises EINVAL if condition is false */
function arg_assert(bool $condition, Str\SprintfFormatString $message, mixed ...$args): void {
  if ($condition) {
    return;
  }
  /* HH_IGNORE_ERROR[4027] passing format string */
  throw_errno(OS\Errno::EINVAL, $message, ...$args);
}
