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

function do_something(): void {}

class X {
  private static ?int $x = null;

  public function getX(): int {
    if (X::$x === null) {
      X::$x = 0;
    }
    do_something();
    return X::$x;
  }
}
