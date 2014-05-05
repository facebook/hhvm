<?hh
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
  if(!is_int($x)) {
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
