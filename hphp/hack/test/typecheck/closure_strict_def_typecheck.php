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


class Foo {
  public function bar(): void {
    /* Make sure we raise a type error for the nonexistent method access in the
     * lambda below */
    $f = function() { Foo::f1(); };
  }
}
