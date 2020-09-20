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
  private int $x;

  public function __construct(int $x) {
    $this->x = $x;
  }
}

class B extends A {}

class C extends B {
  private int $x;

  public function __construct() {
    $this->x = 0;
    switch (mt_rand() % 3) {
      case 0:
        parent::__construct(0);
        break;
      case 1:
        parent::__construct($this->x);
        break;
      case 2:
        $args = varray[1];
        parent::__construct(...$args);
        break;
    }
  }

}
