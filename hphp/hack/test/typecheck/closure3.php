<?hh // partial
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
  public int $called = 0;

  public function foo($x) {
    $y = function($x) {
      $this->called++;
      return $x + 10;
    };
    return $y($x);
  }
}
