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

abstract class A1 {
  protected int $i;
}

abstract class A2 extends A1 {
  protected int $j;
}

class B extends A2 {
  public function __construct() {
    $this->i = 1;
    $this->j = 2;
  }
}

class C extends B {
  public function test(): void {
    $this->i = 2;
  }
}
