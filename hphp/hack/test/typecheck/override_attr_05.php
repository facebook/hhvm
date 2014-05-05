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

class A {
  public function foo(): void {}

  public function bar(): void {}

  public static function statFoo(): void {}
}

trait TUnrelated {
  public function baz(): void {}
}

trait ATr {
  <<Override>> // enforced onto C1 with A as parent
  public function foo(): void {}

  <<Override>> // enforced onto C1 with A as parent
  public static function statFoo(): void {}

  <<Override>> // enforced onto C1 with A as parent
  final public function bar(): void {}

  use TUnrelated;
  <<Override>> // enforced immediately with TUnrelated parent
  public function baz(): void {}
}

class C1 extends A {
  use ATr;
}

class C2 extends C1 {

  <<Override>> // C1 is the parent
  public function foo(): void {}

}
