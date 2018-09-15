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

abstract class A {
  protected int $x;

}

class B extends A {
  protected int $y;
  public function __construct() {
    $this->x = 0;
    $this->y = 0;
    if (true) {
      $this->foo();
    }
  }

  public function foo(): void {}
}
