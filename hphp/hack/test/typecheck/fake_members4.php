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

function do_something(): void {
}

class X {
  private ?int $x;

  public function getX(): int {
    if($this->x === null) {
      $this->x = 0;
    }
    do_something();
    hh_show($this->x);
    return $this->x;
  }
}
