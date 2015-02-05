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
        $args = array(1);
        parent::__construct(...$args);
        break;
    }
  }

}

function mt_rand(): int {
  // UNSAFE
}
