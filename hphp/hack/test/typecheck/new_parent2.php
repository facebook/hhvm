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

class B {}

class A extends B {

  public function foo(): void {}

  public static function bar(): void {
    $x = new parent();
    $x->foo();
    echo 'test';
  }
}
