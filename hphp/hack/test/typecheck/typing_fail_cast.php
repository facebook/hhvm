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

class B {
  public function g(): int { return 0;
  }
}

class A<T> extends Exception implements B {
  public function f(): void {
  }
}

function test1(mixed $x): A<bool> {
  if($x instanceOf A) {
    return (A<bool>)$x;
  }
  throw new A();
}
