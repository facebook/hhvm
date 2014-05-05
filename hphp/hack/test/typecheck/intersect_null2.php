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

class X {
  public function someint(): int {
    return 2;
  }
}

class Foo {
  private ?X $x;

  public function test(): int {
    $y = new X();
    if ($y->someint()) {
    }

    $this->x = $y;
    return $y->someint();
  }
}
