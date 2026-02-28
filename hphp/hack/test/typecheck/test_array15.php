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

class X {
  public varray<int> $v = dict[];

  public function set(int $x): void {
    $this->v[0] = $x;
  }
}

function test(varray<int> &$x): void {
  $x[0] = 33;
}
