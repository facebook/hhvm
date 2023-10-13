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

/**
 * Hack treats method dispatch in a different way from PHP.
 */

class A {
  public function f1(): void {}
  public static function f2(): void {}
}

class B extends A {
  public function f1(): void {}
  public static function f2(): void {}
  public function test1(A $x): void {
    /* HH_FIXME[4090] */
    $x::f1();
  }
  public static function test2(): void {}
}

class C {
  public function f3(): void {}
  public static function f4(): void {}
}
