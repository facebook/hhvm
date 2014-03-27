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

class X {
  public int $foo = 0;
  public function foo(): void {
    echo 'hello';
  }
}

function test(): void {
  $x = new X();
  call_user_func($x->foo);
}
