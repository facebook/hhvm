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

interface I {}
class A implements I {
  public function get(): void {}
}
class B implements I {
  public function get(): void {}
}
class Z {}

function bar(): void {
  $x = true? new A(): new B();
  $x->get();
}
