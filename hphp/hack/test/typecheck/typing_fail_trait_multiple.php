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

trait A {
  protected int $i = 0;
}

trait B {
  use A;
}

class C {
  use B;

  public function __construct() {
    $this->i = 1;
  }
}
