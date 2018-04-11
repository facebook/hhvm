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
  public int $x = 0;
}

class B {
  public A $y;
  public int $z = 44;
  public int $u = 22;

  public function __construct() {
    $this->y = new A();
  }

  public function __clone(): void {
    $this->u = 51;
  }
}

function main(): void {
  $b = new B();
  $b2 = clone $b;
  $b->z = 32;
  if($b2->z === 44 && $b2->u === 51 && $b2->y->x === 0) {
    echo 'OK';
  }
  else {
    echo 'Failure: test_clone.1';
  }
  
}
