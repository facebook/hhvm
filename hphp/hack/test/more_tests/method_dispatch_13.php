<?hh // strict
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

/**
 * Hack treats method dispatch in a different way from PHP.
 */

class A {
  public function f1(): void {
  }
  public static function f2(): void {
  }
}

class B extends A {
  public function f1(): void {
  }
  public static function f2(): void {
  }
  public function test1(): void {
  }
  public static function test2(): void {
    self::f1();
  }
}

class C {
  public function f3(): void {
  }
  public static function f4(): void {
  }
}
