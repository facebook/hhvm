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

interface I {}
class A implements I {
  public function get(): void {}
}
class B implements I {}
class Z {}

function bar(): B {
  $x = true ? null : new B();
  if ($x === null) {
    return new B();
  }
  return $x;
}
