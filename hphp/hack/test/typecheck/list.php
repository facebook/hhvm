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

class A {
  public function foo() {
    $x = 1;
    $y = 2;
    $this->list($x, $y) = 10;
  }
}
