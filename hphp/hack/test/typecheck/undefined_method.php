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

interface I1 {}

class C implements I1 {
  public static function f1(): I1 {
    return new C();
  }

  public function f2(): void {
    $x = C::f1();
    $x->blah();
  }
}
