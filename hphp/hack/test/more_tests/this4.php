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

class X<T1, T2> {

  private T1 $x;

  public function __construct(T1 $x) {
    $this->x = $x;
  }

  public function foo(): this {
    return $this;
  }

  public function bar(): T1 {
    return $this->x;
  }
}

function test(): int {
  $x = new X(0);
  return $x->foo()->bar();
}


