<?hh // partial
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
}

class B extends A {
}

function foo(): void {
  $x = new A();

  if(true) {
    $x = new B();
  }
}

function foo2(mixed $x): int {
  if(!($x is int)) {
    $x = 0;
  }
  return $x;
}

function foo3(mixed $x): void {
  if(true) {
    $x = 0;
  }
}

function foo4(): void {
  if(true) {
    $x = new A();
  }
  else {
    $x = new B();
  }
}
