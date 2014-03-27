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
  public function f(): int { return 0; }
}

class B {
  public function f(): int { return 0; }
  public function g(): int { return 0; }
}

function test(): void {
  $x = new A();
  if($x instanceof B) {

  }
}

function test2(dyn $x): void {
  if($x instanceof A) {
    $x->f();
  } else if($x instanceof B) {
    $x->g();
  } else if($x = 0) {
    $y = $x + 1;
  } else if($x) {
    $x = $x + 1;
  }
}
