<?hh
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 */

function f(): void {
  $x = async $v ==> $v;
  /* HH_IGNORE_ERROR[4273] */
  if ($x(true)) {}
}
