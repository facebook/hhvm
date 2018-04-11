<?hh // strict
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 */

namespace NS;

function foo(mixed $in): void {
  /* HH_IGNORE_ERROR[4016] isset() is unsafe, but still defined */
  \var_dump(isset($in));
  /* HH_IGNORE_ERROR[4135] unsset() is also banned */
  unset($in);
  exit(0);
}
