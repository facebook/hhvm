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

abstract class A {
  protected int $x;

  public function __construct() {
    $this->x = 5;
  }

  public function make_my_day(): bool {
    return ($this->x === 0);
  }

}

abstract class B extends A {
  protected int $y;

  public function __construct() {
    $this->y = 9;
  }

  public function do_this_too(): bool {
    return ($this->y === 9);
  }
}

final class C extends B {
  public function __construct() {
    parent::__construct();
    $this->do_this_too();
    print $this->x;
  }
}
