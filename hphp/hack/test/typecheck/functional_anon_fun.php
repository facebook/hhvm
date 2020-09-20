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

class A {

  public function doStuff(): void {
    return;
  }
}

function f(int $x): int {
  $f = function($y) use ($x) {
    return $y->doStuff();
  };
  $result = ($f)(new A());
  return $result;
}
