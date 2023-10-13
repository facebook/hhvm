<?hh
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 */

class A<T as A<T>> {
  public function bar(T $x): T {
    return $x;
  }

}

class Z {

  public function foo(): void {}
}

function test(): void {
  $x = new A();
  $x->bar(new Z())->foo();
}
