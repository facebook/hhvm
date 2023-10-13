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

class A {
  public function render(): string { return ""; }
}

function foo<T as A>(Vector<T> $x): Vector<T> {
  $x[0] = new A();
  return $x;
}

