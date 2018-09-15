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
  public function foo<T as A>(T $x): T {
    return $x;
  }
}

class B extends A {
  public function foo2<T as B>(T $x): void {

  }
}


