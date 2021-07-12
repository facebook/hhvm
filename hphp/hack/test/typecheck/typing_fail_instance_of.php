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
  public function f(): int {
    return 0;
  }
}

class B {
  public function f(): int {
    return 0;
  }
  public function g(): int {
    return 0;
  }
}

function test(): void {
  $x = new A();
  if ($x is B) {

  }
}

function test2(mixed $x): void {
  if ($x is A) {
    $x->f();
  } else if ($x is B) {
    $x->g();
  } else if ($x) {
    $x = $x + 1;
  }
}
