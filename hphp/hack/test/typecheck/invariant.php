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

function foo(?int $x): int {
  invariant($x !== null, "yup!");
  return $x;
}

function foo2(?int $x): int {
  if($x === null) {
    invariant_violation("yup!");
  }
  return $x;
}
