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
  protected int $x;

  public function __construct() {
    $this->x = 0;
  }

}

class B extends A {

  public function __construct() {
    parent::__construct();
    if(true) {
      $this->foo();
    }
    else {
      $this->foo();
    }
  }

  private function foo(): void {
    if(true) {
      return;
    }
  }
}
