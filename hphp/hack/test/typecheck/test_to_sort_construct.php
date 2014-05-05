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

// Checking that the initialization of constructors is properly detected

class A {
  private int $x;

  public function __construct(int $x) {
    if(true) {
      $this->x = $x;
    } else {
      $y = $this->init();
    }

  }

  private function init(): void {
    $this->x = 0;
    $this->init();
  }
}

class B extends A {

  public function __construct() {
    parent::__construct(1);
    $x = new A(0);
  }

}
