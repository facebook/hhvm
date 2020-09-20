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

  public function foo(): void {}

  public static function bar(): void {
    $x = new self();
    $x->xxx();
  }
}
