<?hh // strict
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
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
  call_user_func($f, new A());
  return call_user_func($f, new A());
}
