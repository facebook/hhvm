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
  private mixed $x;

  public function __construct() {
    $this->x = '';
  }

  public function getX(): Z {
    if($this->x instanceof Z) {
      return $this->x;
    }
    return new Z();
  }
}

class Z {}
