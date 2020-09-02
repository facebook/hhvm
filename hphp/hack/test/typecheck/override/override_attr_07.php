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

abstract class A {
  abstract protected function foo();
}

abstract class B extends A {

  <<__Override>>
  public function foo(): void {}

  abstract protected function bar();
}

class C extends B {

  <<__Override>>
  public function bar(): void {}
}
