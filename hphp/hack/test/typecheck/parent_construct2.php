<?hh // strict
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

abstract class A {
  public int $x;

  public function __construct() {
    $this->x = 5;
  }

  public function make_my_day(): bool {
    return ($this->x === 0);
  }

}

class B extends A {
  public int $z;

  public function __construct() {
    if(true) {
    }
    $this->z = 0;
    $this->make_my_day();
    parent::__construct();
  }
}

