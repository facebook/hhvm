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

function foo(): mixed {
  return 0;
}

function test(bool $statement): bool {
  if(!$statement) {
    $y = foo();
    invariant($y is Exception, "");
    throw $y;
  }
  return $statement;
}
