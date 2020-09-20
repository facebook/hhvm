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
  public int $x;

  public function __construct() {
    $this->x = 5;
  }

  public function make_my_day(): bool {
    return ($this->x === 0);
  }

}

class B extends A {

  public function __construct() {
    if (true) {
      parent::__construct();
    } else {
      $this->init();
    }
  }

  private function init(): void {
    if (false) {
      /* HH_FIXME[3011] */
      parent::__construct();
    } else {
      /* HH_FIXME[3011] */
      parent::__construct();
    }
  }
}
