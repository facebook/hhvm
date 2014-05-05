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

abstract class A {
  protected int $i;
  protected int $j;

  public function __construct() {
    $this->j = 0;
  }
}

class B extends A {
  public function __construct() {
    parent::__construct();
    $this->i = 1;
  }
}

class C extends B {
  public function test(): void {
    $this->i = 2;
  }
}
