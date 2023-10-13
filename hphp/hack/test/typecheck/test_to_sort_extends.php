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

class A {
  public function plus(A $x, A $y): A {
    return $x;
  }
}

class B extends A {
  public function plus(A $x, A $y): B {
    $res = parent::plus($x, $x);
    $res->plus($x, $y);
    return $x;
  }

}
