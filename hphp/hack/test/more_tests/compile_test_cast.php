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

  public function __toString(): string {
    return 'Afda';
  }
}

class B extends A {
}

function main(): void {
  $x = '11';
  $y = (int)$x;
  $z = new A();
  $x2 = (string)$z;
  $x3 = new B();
  $x4 = (A)$x3;
  if($y === 11 && $x2 === 'Afda') {
    echo 'OK';
  }
  else {
    echo 'Failure: test_cast.1';
  }
}
