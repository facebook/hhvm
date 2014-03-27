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

abstract class A {
  abstract protected function foo();
}

abstract class B extends A {

  <<Override>>
  public function foo(): void {}

  abstract protected function bar();
}

class C extends B {

  <<Override>>
  public function bar(): void {}
}
